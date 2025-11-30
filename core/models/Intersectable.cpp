#include "Intersectable.hpp"
#include <GL/glew.h>
#include <cstdint>
#include <stack>
#include <set>
#include <iostream>

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
  std::vector<uint8_t> temp = m_textureData;
  constexpr int nx8[8] = {1, 1, 0, -1, -1, -1, 0, 1};
  constexpr int ny8[8] = {0, 1, 1, 1, 0, -1, -1, -1};

  std::vector<math137::Vector3f> path;

  int dir = 0;
  auto [x, y] = findFirstContourPoint(sx, sy);
  if (x == -1 || y == -1)
    return path; // brak punktów konturu
  path.emplace_back(getValue(static_cast<float>(y) / (m_height - 1), static_cast<float>(x) / (m_width - 1)));
  do
  {
    int startDir = (dir + 6) % 8; // 6 = -90° w Moore grid (8 kierunków)
    int foundDir = -1;

    for (int i = 0; i < 8; i++)
    {
      int nd = (startDir + i) % 8;
      int nx = x + nx8[nd];
      int ny = y + ny8[nd];
      if((x < 0 || x >= m_width) && !wrappableV())
        continue; // poza granicami
      if((y < 0 || y >= m_height) && !wrappableU())
        continue; // poza granicami
      nx = (nx + m_width) % m_width;
      ny = (ny + m_height) % m_height;
      if (temp[ny * m_width + nx] != 0)
      {
        foundDir = nd;
        x = nx;
        y = ny;
        path.emplace_back(getValue(static_cast<float>(y) / (m_height - 1), static_cast<float>(x) / (m_width - 1)));
        temp[ny * m_width + nx] = 0;
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

std::vector<std::vector<math137::Vector3f>> Intersectable::extractPaths(int step) const
{
  std::vector<std::vector<math137::Vector3f>> res;
  std::vector<uint8_t> temp = m_trimmingData;
  for (int y = 0; y < m_height; y += step)
  {
    for (int x = 0; x < m_width; x += step)
    {
      if (temp[y * m_width + x] != 0)
        continue;

      auto path = extractPath(x, y, temp, step);
      if (path.size() > 2)
      {
        res.push_back(path);
      }
    }
  }
  return res;
}

std::vector<math137::Vector3f> Intersectable::extractPath(int x, int y, std::vector<uint8_t> &temp, int step, bool checkYFirst) const
{
  struct State
  {
    int x;
    int y;
    int dir;
    int wall;
  };

  std::vector<math137::Vector3f> path;
  std::stack<State> s;
  s.push({x, y, step, 1});
  int yMul = 1;
  while (!s.empty())
  {
    auto state = s.top();
    s.pop();

    float u = static_cast<float>(state.y) / (m_height - 1);
    float v = static_cast<float>(state.x) / (m_width - 1);
    math137::Vector3f pos = getValue(u, v);

    math137::Vector3f du = uDerivative(u, v);
    math137::Vector3f dv = vDerivative(u, v);

    math137::Vector3f normal = math137::Vector3f::Cross(du, dv);
    normal.normalize();

    if (temp[state.y * m_width + state.x] != 0 && state.wall == 0)
      continue;
   
    if (!(normal.any(
                                 [](float coord)
                                 { return std::isnan(coord) || std::isinf(coord); })))
   {
    path.emplace_back(pos);
   }

    temp[state.y * m_width + state.x] = 255; // oznacz jako odwiedzony
    int nx = state.x + state.dir;
    int ny = state.y + step * yMul;
    if (wrappableU())
      ny = (ny + m_height) % m_height;
    if (wrappableV())
      nx = (nx + m_width) % m_width;

    bool canMoveX = (nx >= 0 && nx < m_width && m_trimmingData[state.y * m_width + nx] == 0);
    bool canMoveY = (ny >= 0 && ny < m_height && temp[ny * m_width + state.x] == 0);

    // If requested, when on wall==1 try vertical move first
    if (checkYFirst && state.wall == 1 && canMoveY)
    {
      s.push({state.x, ny, -state.dir, 0});
      continue;
    }

    if (canMoveX)
    {
      s.push({nx, state.y, state.dir, state.wall});
      continue;
    }

    // If not checked earlier, try vertical move when appropriate
    if (!checkYFirst && state.wall == 1 && canMoveY)
    {
      s.push({state.x, ny, -state.dir, 0});
      continue;
    }

    if (state.wall == 2 && yMul > 0)
    {
      break;
    }
    else if (state.wall == 1)
    {
      s.push({state.x, state.y, -state.dir, 2});
      continue;
    }
    else if (state.wall == 0)
    {
      s.push({state.x, state.y, -state.dir, 1});
    }
  }
  return path;
}

std::vector<math137::Vector3f> Intersectable::gridMillingPath(float stepU, float stepV) const
{
    std::vector<math137::Vector3f> path;
    bool direction = true;
    for (float v = 0.0f; v <= 1.0f; v += stepV)
    {
        if (direction)
        {
            for (float u = 0.0f; u <= 1.0f; u += stepU)
            {
                if(!isTrimmedUV(u, v))
                    path.push_back(getValue(u, v));
            }
        }
        else
        {
            for (float u = 1.0f; u >= 0.0f; u -= stepU)
            {
                if(!isTrimmedUV(u, v))
                    path.push_back(getValue(u, v));
            }
        }
        direction = !direction;
    }
    for (float u = 0.0f; u <= 1.0f; u += stepU)
    {
        if (direction)
        {
            for (float v = 0.0f; v <= 1.0f; v += stepV)
            {
                if(!isTrimmedUV(u, v))
                    path.push_back(getValue(u, v));
            }
        }
        else
        {
            for (float v = 1.0f; v >= 0.0f; v -= stepV)
            {
                if(!isTrimmedUV(u, v))
                    path.push_back(getValue(u, v));
            }
        }
        direction = !direction;
    }

    return path;
}