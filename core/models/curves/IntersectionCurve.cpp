#include "IntersectionCurve.hpp"
#include "Vector.hpp"
#include <vector>

uint16_t IntersectionCurve::s_count = 0;

IntersectionCurve::IntersectionCurve(
    const std::vector<math137::Vector3f> &points)
    : m_points(points), Object(ShaderType::OBJECT) {
  name = "Intersection " + std::to_string(s_count++);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  setVetrices();
}
void IntersectionCurve::setVetrices() {
  std::vector<float> verts;
  verts.reserve(m_points.size() * 3);
  for (const auto &p : m_points) {
    verts.push_back(p.x());
    verts.push_back(p.y());
    verts.push_back(p.z());
  }
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(),
               GL_STATIC_DRAW);
}

void IntersectionCurve::render(std::shared_ptr<Renderer> &renderer,
                               const math137::Vector4f &color) {
  glBindVertexArray(m_vao);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(color);
  glDrawArrays(GL_LINE_STRIP, 0, m_points.size());
}
bool IntersectionCurve::renderObjectMenu() { return false; }
