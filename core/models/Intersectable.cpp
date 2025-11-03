#include "Intersectable.hpp"
#include <GL/glew.h>
#include <cstdint>
#include <stack>

Intersectable::Intersectable() {
  m_trimmingData.resize(m_width * m_height);
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

void Intersectable::intersectTrimmingTexture(const std::vector<uint8_t> &v,
                                       uint16_t x, uint16_t y) {
  // Create a temporary buffer for the incoming mask sized to the internal
  // trimming buffer. Copy the incoming data (or zeros if smaller), perform
  // a flood-fill expansion on that temporary mask starting from (x,y), then
  // intersect the filled temporary mask with the existing trimming mask.
  std::vector<uint8_t> temp(m_trimmingData.size(), 0);
  size_t minSz = std::min(m_trimmingData.size(), v.size());
  for (size_t i = 0; i < minSz; ++i)
    temp[i] = (v[i] != 0) ? 255 : 0;

  // Flood-fill the temporary mask from the seed (x,y) using the same
  // wrap/edge behavior as unionTrimmingTexture.
  std::stack<std::pair<int, int>> s;
  s.push({static_cast<int>(x), static_cast<int>(y)});
  while (!s.empty()) {
    auto [xx, yy] = s.top();
    s.pop();

    if ((yy < 0 || yy >= m_height) && !wrappableU()) {
      continue;
    }
    if ((xx < 0 || xx >= m_width) && !wrappableV()) {
      continue;
    }

    xx = (xx + m_width) % m_width;
    yy = (yy + m_height) % m_height;

    size_t idx = yy * m_width + xx;
    if (temp[idx] != 0)
      continue; // already filled
    temp[idx] = 255;

    if (xx > 0 && temp[yy * m_width + xx - 1] == 0) {
      s.push({xx - 1, yy});
    }
    if (xx < m_width - 1 && temp[yy * m_width + xx + 1] == 0) {
      s.push({xx + 1, yy});
    }
    if (yy > 0 && temp[(yy - 1) * m_width + xx] == 0) {
      s.push({xx, yy - 1});
    }
    if (yy < m_height - 1 && temp[(yy + 1) * m_width + xx] == 0) {
      s.push({xx, yy + 1});
    }
  }

  // Intersect: keep only pixels present in both existing mask and filled temp
  for (size_t i = 0; i < m_trimmingData.size(); ++i) {
    m_trimmingData[i] = (m_trimmingData[i] != 0 && temp[i] != 0) ? 255 : 0;
  }

  // Upload updated trimming mask to GPU
  glBindTexture(GL_TEXTURE_2D, m_trimmingTexture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RED,
                  GL_UNSIGNED_BYTE, m_trimmingData.data());
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Intersectable::unionTrimmingTexture(const std::vector<uint8_t> &v,
                                         uint16_t x, uint16_t y) {
  // OR incoming data into existing mask (union). Any non-zero in `v` marks
  // the pixel as trimmed.
  size_t minSz = std::min(m_trimmingData.size(), v.size());
  for (size_t i = 0; i < minSz; ++i) {
    if (v[i] != 0)
      m_trimmingData[i] = 255;
  }

  // optionally perform the same flood-fill behaviour as setTrimmingTexture
  // so the seed pixel expands the trimmed region if requested by the UI.
  std::stack<std::pair<int, int>> s;
  s.push({x, y});
  while (!s.empty()) {
    auto [xx, yy] = s.top();
    s.pop();

    if ((yy < 0 || yy >= m_height) && !wrappableU()) {
      continue;
    }
    if ((xx < 0 || xx >= m_width) && !wrappableV()) {
      continue;
    }

    xx = (xx + m_width) % m_width;
    yy = (yy + m_height) % m_height;

    m_trimmingData[yy * m_width + xx] = 255;
    if (xx > 0 && m_trimmingData[yy * m_width + xx - 1] == 0) {
      s.push({xx - 1, yy});
      m_trimmingData[yy * m_width + xx - 1] = 255;
    }
    if (xx < m_width - 1 && m_trimmingData[yy * m_width + xx + 1] == 0) {
      s.push({xx + 1, yy});
      m_trimmingData[yy * m_width + xx + 1] = 255;
    }
    if (yy > 0 && m_trimmingData[(yy - 1) * m_width + xx] == 0) {
      s.push({xx, yy - 1});
      m_trimmingData[(yy - 1) * m_width + xx] = 255;
    }
    if (yy < m_height - 1 && m_trimmingData[(yy + 1) * m_width + xx] == 0) {
      s.push({xx, yy + 1});
      m_trimmingData[(yy + 1) * m_width + xx] = 255;
    }
  }

  glBindTexture(GL_TEXTURE_2D, m_trimmingTexture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RED,
                  GL_UNSIGNED_BYTE, m_trimmingData.data());
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Intersectable::resetTrimming() {
  std::fill(m_trimmingData.begin(), m_trimmingData.end(), 0);
  glBindTexture(GL_TEXTURE_2D, m_trimmingTexture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RED,
                  GL_UNSIGNED_BYTE, m_trimmingData.data());
  glBindTexture(GL_TEXTURE_2D, 0);
}

bool Intersectable::isTrimmedUV(float u, float v) const {
  int x = static_cast<int>(v * m_width);
  int y = static_cast<int>(u * m_height);

  if (!wrappableU()) {
    if (y < 0) y = 0;
    if (y >= (int)m_height) y = (int)m_height - 1;
  } else {
    y = (y % (int)m_height + (int)m_height) % (int)m_height;
  }
  if (!wrappableV()) {
    if (x < 0) x = 0;
    if (x >= (int)m_width) x = (int)m_width - 1;
  } else {
    x = (x % (int)m_width + (int)m_width) % (int)m_width;
  }

  return m_trimmingData[y * m_width + x] != 0;
}
