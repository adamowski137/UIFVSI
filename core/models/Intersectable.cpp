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
    if (m_trimmingData[y * m_width + x - 1] == 0) {
      s.push({x - 1, y});
      m_trimmingData[y * m_width + x - 1] = 255;
    }
    if (m_trimmingData[y * m_width + x + 1] == 0) {
      s.push({x + 1, y});
      m_trimmingData[y * m_width + x + 1] = 255;
    }
    if (m_trimmingData[(y - 1) * m_width + x] == 0) {
      s.push({x, y - 1});
      m_trimmingData[(y - 1) * m_width + x] = 255;
    }
    if (m_trimmingData[(y + 1) * m_width + x] == 0) {
      s.push({x, y + 1});
      m_trimmingData[(y + 1) * m_width + x] = 255;
    }
  }

  glBindTexture(GL_TEXTURE_2D, m_trimmingTexture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RED,
                  GL_UNSIGNED_BYTE, m_trimmingData.data());
  glBindTexture(GL_TEXTURE_2D, 0);
}
