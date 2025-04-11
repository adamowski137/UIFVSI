#include "Point.hpp"
#include "MatrixUtils.hpp"
#include "Object.hpp"
#include "Vector.hpp"
#include <GL/glew.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

uint16_t Point::s_count = 0;

Point::Point() : Object(ShaderType::POINT) {
  name = "Point " + std::to_string(s_count++);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  float w = 0.01f;
  std::vector<float> vert = getSphereVertices();
  std::vector<uint16_t> ind = getSphereIndices();
  glBufferData(GL_ARRAY_BUFFER, vert.size() * sizeof(float), vert.data(),
               GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof(uint16_t),
               ind.data(), GL_STATIC_DRAW);
  recalculateModel();
}

std::vector<float> Point::getSphereVertices() {
  std::vector<float> vert;
  vert.reserve(3 * m_latSamples * m_longSamples);
  for (uint16_t lat = 0; lat <= m_latSamples; ++lat) {
    float theta = lat * M_PI / m_latSamples;
    float sinTheta = std::sin(theta);
    float cosTheta = std::cos(theta);

    for (uint16_t lon = 0; lon <= m_longSamples; ++lon) {
      float phi = lon * 2.0f * M_PI / m_longSamples;
      float sinPhi = sinf(phi);
      float cosPhi = cosf(phi);

      float x = m_radius * cosPhi * sinTheta;
      float y = m_radius * cosTheta;
      float z = m_radius * sinPhi * sinTheta;

      vert.push_back(x);
      vert.push_back(y);
      vert.push_back(z);
    }
  }
  return vert;
}

std::vector<uint16_t> Point::getSphereIndices() {
  std::vector<uint16_t> indices;
  indices.reserve(m_latSamples * m_longSamples * 6);
  for (uint16_t lat = 0; lat < m_latSamples; ++lat) {
    for (uint16_t lon = 0; lon < m_longSamples; ++lon) {
      uint16_t first = (lat * (m_longSamples + 1)) + lon;
      uint16_t second = first + m_longSamples + 1;

      indices.push_back(first);
      indices.push_back(second);
      indices.push_back(first + 1);

      indices.push_back(second);
      indices.push_back(second + 1);
      indices.push_back(first + 1);
    }
  }
  return indices;
}

void Point::render(std::shared_ptr<Renderer> &renderer,
                   const math137::Vector4f &color) {
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(color);
  glDrawElements(GL_TRIANGLES, 6 * m_latSamples * m_longSamples,
                 GL_UNSIGNED_SHORT, 0);
}

void Point::renderObjectMenu() {
  if (!m_openMenu)
    return;
  setNameMenu();
}

void Point::recalculateModel() {
  m_model = math137::MatrixUtils::Translate(
      m_translation.x(), m_translation.y(), m_translation.z());
  m_update = false;
}
