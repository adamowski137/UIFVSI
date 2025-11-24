#include "Intersectable.hpp"
#include <GL/glew.h>
#include <cstdint>
#include <stack>
#include <set>

Intersectable::Intersectable()
{
  m_trimmingData.resize(m_width * m_height);
  m_textureData.resize(m_width * m_height);
  glGenTextures(1, &m_trimmingTexture);
  glBindTexture(GL_TEXTURE_2D, m_trimmingTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_width, m_height, 0, GL_RED,
               GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);
}

Intersectable::~Intersectable() { glDeleteTextures(1, &m_trimmingTexture); }

void Intersectable::intersectTrimmingTexture(
    uint16_t x, uint16_t y)
{
  // Create a temporary buffer for the incoming mask sized to the internal
  // trimming buffer. Copy the incoming data (or zeros if smaller), perform
  // a flood-fill expansion on that temporary mask starting from (x,y), then
  // intersect the filled temporary mask with the existing trimming mask.
  std::vector<uint8_t> temp(m_trimmingData.size(), 0);
  size_t minSz = m_trimmingData.size();
  for (size_t i = 0; i < minSz; ++i)
    temp[i] = (m_textureData[i] != 0) ? 255 : 0;

  // Flood-fill the temporary mask from the seed (x,y) using the same
  // wrap/edge behavior as unionTrimmingTexture.
  std::stack<std::pair<int, int>> s;
  s.push({static_cast<int>(x), static_cast<int>(y)});
  while (!s.empty())
  {
    auto [xx, yy] = s.top();
    s.pop();

    if ((yy < 0 || yy >= m_height) && !wrappableU())
    {
      continue;
    }
    if ((xx < 0 || xx >= m_width) && !wrappableV())
    {
      continue;
    }

    xx = (xx + m_width) % m_width;
    yy = (yy + m_height) % m_height;

    size_t idx = yy * m_width + xx;
    if (temp[idx] != 0)
      continue; // already filled
    temp[idx] = 255;

    if (xx > 0 && temp[yy * m_width + xx - 1] == 0)
    {
      s.push({xx - 1, yy});
    }
    if (xx < m_width - 1 && temp[yy * m_width + xx + 1] == 0)
    {
      s.push({xx + 1, yy});
    }
    if (yy > 0 && temp[(yy - 1) * m_width + xx] == 0)
    {
      s.push({xx, yy - 1});
    }
    if (yy < m_height - 1 && temp[(yy + 1) * m_width + xx] == 0)
    {
      s.push({xx, yy + 1});
    }
  }

  // Intersect: keep only pixels present in both existing mask and filled temp
  for (size_t i = 0; i < m_trimmingData.size(); ++i)
  {
    m_trimmingData[i] = (m_trimmingData[i] != 0 && temp[i] != 0) ? 255 : 0;
  }

  // Upload updated trimming mask to GPU
  glBindTexture(GL_TEXTURE_2D, m_trimmingTexture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RED,
                  GL_UNSIGNED_BYTE, m_trimmingData.data());
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Intersectable::unionTrimmingTexture(uint16_t x, uint16_t y)
{
  // OR incoming data into existing mask (union). Any non-zero in `v` marks
  // the pixel as trimmed.
  size_t minSz = m_trimmingData.size();
  for (size_t i = 0; i < minSz; ++i)
  {
    if (m_textureData[i] != 0)
      m_trimmingData[i] = 255;
  }

  // optionally perform the same flood-fill behaviour as setTrimmingTexture
  // so the seed pixel expands the trimmed region if requested by the UI.
  std::stack<std::pair<int, int>> s;
  s.push({x, y});
  while (!s.empty())
  {
    auto [xx, yy] = s.top();
    s.pop();

    if ((yy < 0 || yy >= m_height) && !wrappableU())
    {
      continue;
    }
    if ((xx < 0 || xx >= m_width) && !wrappableV())
    {
      continue;
    }

    xx = (xx + m_width) % m_width;
    yy = (yy + m_height) % m_height;

    m_trimmingData[yy * m_width + xx] = 255;
    if (xx > 0 && m_trimmingData[yy * m_width + xx - 1] == 0)
    {
      s.push({xx - 1, yy});
      m_trimmingData[yy * m_width + xx - 1] = 255;
    }
    if (xx < m_width - 1 && m_trimmingData[yy * m_width + xx + 1] == 0)
    {
      s.push({xx + 1, yy});
      m_trimmingData[yy * m_width + xx + 1] = 255;
    }
    if (yy > 0 && m_trimmingData[(yy - 1) * m_width + xx] == 0)
    {
      s.push({xx, yy - 1});
      m_trimmingData[(yy - 1) * m_width + xx] = 255;
    }
    if (yy < m_height - 1 && m_trimmingData[(yy + 1) * m_width + xx] == 0)
    {
      s.push({xx, yy + 1});
      m_trimmingData[(yy + 1) * m_width + xx] = 255;
    }
  }

  glBindTexture(GL_TEXTURE_2D, m_trimmingTexture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RED,
                  GL_UNSIGNED_BYTE, m_trimmingData.data());
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Intersectable::resetTrimming()
{
  std::fill(m_trimmingData.begin(), m_trimmingData.end(), 0);
  glBindTexture(GL_TEXTURE_2D, m_trimmingTexture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RED,
                  GL_UNSIGNED_BYTE, m_trimmingData.data());
  glBindTexture(GL_TEXTURE_2D, 0);
}

bool Intersectable::isTrimmedUV(float u, float v) const
{
  int x = static_cast<int>(v * (m_width - 1));
  int y = static_cast<int>(u * (m_height - 1));

  if (!wrappableU())
  {
    if (y < 0)
      y = 0;
    if (y >= (int)m_height)
      y = (int)m_height - 1;
  }
  else
  {
    y = (y % (int)m_height + (int)m_height) % (int)m_height;
  }
  if (!wrappableV())
  {
    if (x < 0)
      x = 0;
    if (x >= (int)m_width)
      x = (int)m_width - 1;
  }
  else
  {
    x = (x % (int)m_width + (int)m_width) % (int)m_width;
  }

  return m_trimmingData[y * m_width + x] != 0;
}
std::vector<math137::Vector3f> Intersectable::extractContour(int sx, int sy) const
{
  constexpr int nx8[8] = {1, 1, 0, -1, -1, -1, 0, 1};
  constexpr int ny8[8] = {0, 1, 1, 1, 0, -1, -1, -1};

  std::vector<math137::Vector3f> path;
  std::set<std::pair<int, int>> visited;

  int dir = 0;
  auto [x, y] = findFirstContourPoint(sx, sy);
  if (x == -1 || y == -1)
    return path; // brak punktów konturu
  path.emplace_back(getValue(static_cast<float>(y) / (m_height - 1), static_cast<float>(x) / (m_width - 1)));
  visited.insert({x, y});
  do
  {
    int startDir = (dir + 6) % 8; // 6 = -90° w Moore grid (8 kierunków)
    int foundDir = -1;

    for (int i = 0; i < 8; i++)
    {
      int nd = (startDir + i) % 8;
      int nx = x + nx8[nd];
      int ny = y + ny8[nd];
      if (nx < 0 || nx >= m_width || ny < 0 || ny >= m_height)
        continue; // poza granicami
      if (m_textureData[ny * m_width + nx] != 0 && visited.find({nx, ny}) == visited.end())
      {
        foundDir = nd;
        x = nx;
        y = ny;
        visited.insert({x, y});
        path.emplace_back(getValue(static_cast<float>(y) / (m_height - 1), static_cast<float>(x) / (m_width - 1)));
        dir = nd;
        break;
      }
    }

    if (foundDir == -1)
    {
      // Nie znaleziono nic — obwód jest samoprzecinający lub błędny
      break;
    }

  } while (!(x == sx && y == sy && path.size() > 1));

  return path;
}

std::pair<int, int> Intersectable::findFirstContourPoint(int startX, int startY) const
{
  int sx = -1, sy = -1;

  for (int y = startY; y < m_height; y++)
  {
    for (int x = startX; x < m_width; x++)
    {
      if (m_textureData[y * m_width + x] != 0)
      {
        sx = x;
        sy = y;
        break;
      }
    }
    if (sx != -1)
      break;
  }

  return std::pair<int, int>(sx, sy);
}
