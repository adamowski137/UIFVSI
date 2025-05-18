#include "BezierC0.hpp"
#include "Surface.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include <vector>

uint16_t BezierC0::s_count = 0;

BezierC0::BezierC0(const std::vector<std::shared_ptr<Object>> &points,
                   uint16_t uPatches, uint16_t vPatches, bool cylinder)
    : Surface(points, uPatches, vPatches, cylinder) {
  name = "BezierCurve " + std::to_string(s_count++);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  setVertices();
  setEdges();
}

void BezierC0::setVertices() {
  std::vector<float> points;
  for (uint16_t u = 0; u < m_points.size(); u++) {
    for (uint16_t v = 0; v < m_points[0].size(); v++) {
      math137::Vector3f p = m_points[u][v].lock()->getTranslation();
      points.push_back(p.x());
      points.push_back(p.y());
      points.push_back(p.z());
    }
  }
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(),
               GL_STATIC_DRAW);
}

void BezierC0::notify() { setVertices(); }

void BezierC0::render(std::shared_ptr<Renderer> &renderer,
                      const math137::Vector4f &color) {
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(color);
  if (m_type == ShaderType::SURFACE) {
    renderer->setUVSubdivisions(m_divisionsU, m_divisionsV);
    glPatchParameteri(GL_PATCH_VERTICES, 16);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_PATCHES, m_edges.size(), GL_UNSIGNED_SHORT, (void *)0);
  }
  if (m_type == ShaderType::OBJECT) {
    glDrawElements(GL_LINES, m_edges.size(), GL_UNSIGNED_SHORT, (void *)0);
  }
}
