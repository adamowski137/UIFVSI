#include "Gregory.hpp"
#include "Vector.hpp"
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

Gregory::Gregory(
    const std::array<std::array<std::weak_ptr<Object>, 4>, 3> &edges,
    const std::array<std::array<std::weak_ptr<Object>, 4>, 3> &prev)
    : m_edges(edges), m_prev(prev), Object(ShaderType::OBJECT) {
  setVertices();
  name = ("Gregory" + std::to_string(m_id));
}
math137::Vector3f Gregory::lerp(const math137::Vector3f &a,
                                const math137::Vector3f &b, float t) {
  return a * (1 - t) + b * t;
}

std::pair<std::array<math137::Vector3f, 4>, std::array<math137::Vector3f, 4>>
Gregory::divideCurve(float t,
                     const std::array<std::weak_ptr<Object>, 4> &points) {
  math137::Vector3f p0 = points[0].lock()->getTranslation();
  math137::Vector3f p1 = points[1].lock()->getTranslation();
  math137::Vector3f p2 = points[2].lock()->getTranslation();
  math137::Vector3f p3 = points[3].lock()->getTranslation();

  math137::Vector3f A = lerp(p0, p1, t);
  math137::Vector3f B = lerp(p1, p2, t);
  math137::Vector3f C = lerp(p2, p3, t);

  math137::Vector3f D = lerp(A, B, t);
  math137::Vector3f E = lerp(B, C, t);
  math137::Vector3f F = lerp(D, E, t);

  return {{p0, A, D, F}, {F, E, C, p3}};
}
std::array<math137::Vector3f, 3> Gregory::derivativeU(
    const std::array<std::array<math137::Vector3f, 4>, 4> &points) {
  math137::Vector3f d0 = (points[0][1] - points[0][0]) * 3.f;
  math137::Vector3f d1 = (points[1][3] - points[0][3]) * 3.f;
  // calculate value at 0.5f;
  math137::Vector3f f = points[0][0] * 0.125f + points[0][1] * 0.375f +
                        points[0][2] * 0.375f + points[0][3] * 0.125f;
  math137::Vector3f b = points[1][0] * 0.125f + points[1][1] * 0.375f +
                        points[1][2] * 0.375f + points[1][3] * 0.125f;

  return {d0, (b - f) * 3.f, d1};
}
std::array<math137::Vector3f, 3>
Gregory::twist(const std::array<std::array<math137::Vector3f, 4>, 4> &points) {
  std::vector<math137::Vector3f> coeffs;
  for (int j = 0; j < 3; j++)
    coeffs.push_back(
        (points[1][j + 1] - points[0][j + 1] - points[1][j] + points[0][j]) *
        9.f);
  math137::Vector3f w0 = coeffs[0];
  math137::Vector3f w1 = coeffs[0] * 0.125f + coeffs[1] * 0.375f +
                         coeffs[2] * 0.375f + coeffs[3] * 0.125f;
  math137::Vector3f w2 = coeffs[3];

  return {w0, w1, w2};
}

void Gregory::setVertices() {
  math137::Vector3f p3[3], p2[3], p1[3], q[3];
  for (uint16_t i = 0; i < 3; i++) {
    const auto [e1, e2] = divideCurve(0.5f, m_edges[i]);
    const auto [prev1, prev2] = divideCurve(0.5f, m_prev[i]);
    p3[i] = e2[0];
    p2[i] = e2[0] - prev2[0] + e2[0];
    q[i] = (p2[i] * 3.f - p3[i]) * 0.5f;
  }
  math137::Vector3f p0 = (q[0] + q[1] + q[2]) / 3.f;
  for (uint16_t i = 0; i < 3; i++) {
    p1[i] = (q[i] * 2 + p0) / 3;
  }

  std::array<float, 3 * 3 * 4> vert = {
      p0.x(),    p0.y(),    p0.z(),    p1[0].x(), p1[0].y(), p1[0].z(),
      p2[0].x(), p2[0].y(), p2[0].z(), p3[0].x(), p3[0].y(), p3[0].z(),
      p0.x(),    p0.y(),    p0.z(),    p1[1].x(), p1[1].y(), p1[1].z(),
      p2[1].x(), p2[1].y(), p2[1].z(), p3[1].x(), p3[1].y(), p3[1].z(),
      p0.x(),    p0.y(),    p0.z(),    p1[2].x(), p1[2].y(), p1[2].z(),
      p2[2].x(), p2[2].y(), p2[2].z(), p3[2].x(), p3[2].y(), p3[2].z(),
  };
  std::array<uint16_t, 18> ind = {
      0, 1, 1, 2, 2, 3, 4, 5, 5, 6, 6, 7, 8, 9, 9, 10, 10, 11,
  };

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ARRAY_BUFFER, vert.size() * sizeof(float), vert.data(),
               GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof(uint16_t),
               ind.data(), GL_STATIC_DRAW);
}

void Gregory::render(std::shared_ptr<Renderer> &renderer,
                     const math137::Vector4f &color) {
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(color);
  glDrawElements(GL_LINES, 18, GL_UNSIGNED_SHORT, (void *)0);
}
