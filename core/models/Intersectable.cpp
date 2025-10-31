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

void Intersectable::setTrimmingTexture(const std::vector<uint8_t> &v,
                                       uint16_t x, uint16_t y) {
  std::stack<std::pair<int, int>> s;
  m_trimmingData = v;
  s.push({x, y});
  while (!s.empty()) {
    auto [x, y] = s.top();
    s.pop();

    if ((y < 0 || y >= m_height) && !wrappableU()) {
      continue;
    }
    if ((x < 0 || x >= m_width) && !wrappableV()) {
      continue;
    }

    x = (x + m_width) % m_width;
    y = (y + m_height) % m_height;

    m_trimmingData[y * m_width + x] = 255;
    if (x > 0 &&  m_trimmingData[y * m_width + x - 1] == 0) {
      s.push({x - 1, y});
      m_trimmingData[y * m_width + x - 1] = 255;
    }
    if (x < m_width - 1 && m_trimmingData[y * m_width + x + 1] == 0) {
      s.push({x + 1, y});
      m_trimmingData[y * m_width + x + 1] = 255;
    }
    if (y > 0 && m_trimmingData[(y - 1) * m_width + x] == 0) {
      s.push({x, y - 1});
      m_trimmingData[(y - 1) * m_width + x] = 255;
    }
    if (y < m_height - 1 && m_trimmingData[(y + 1) * m_width + x] == 0) {
      s.push({x, y + 1});
      m_trimmingData[(y + 1) * m_width + x] = 255;
    }
  }

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
