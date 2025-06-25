#include "BezierC2.hpp"
#include "Surface.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>
#include <vector>

BezierC2::BezierC2(
    const std::vector<std::vector<std::shared_ptr<Object>>> &points,
    uint16_t uPatches, uint16_t vPatches)
    : Surface(points, uPatches, vPatches, ShaderType::SURFACEC2) {
  name = "BezierSurfaceC2 " + std::to_string(s_itemCount++);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  setVertices();
  setEdges();
}

void BezierC2::setVertices() {
  float diffU = 1.f / (m_points.size() - 1);
  float diffV = 1.f / (m_points[0].size() - 1);
  std::vector<float> points;
  for (uint16_t i = 0; i < m_points.size(); i++) {
    for (uint16_t j = 0; j < m_points[i].size(); j++) {
      float nu = diffU * i;
      float nv = diffV * j;
      math137::Vector3f pos = m_points[i][j].lock()->getTranslation();
      points.push_back(pos.x());
      points.push_back(pos.y());
      points.push_back(pos.z());
      points.push_back(nu);
      points.push_back(nv);
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(),
               GL_STATIC_DRAW);
}

void BezierC2::setEdges() {
  uint16_t uPoints = m_points.size();
  uint16_t vPoints = m_points[0].size();
  m_edges.clear();
  if (m_type == ShaderType::OBJECT) {
    for (uint16_t u = 0; u < uPoints; u++) {
      for (uint16_t v = 0; v < vPoints; v++) {
        if (v != 0) {
          m_edges.push_back(u * vPoints + v - 1);
          m_edges.push_back(u * vPoints + v);
        }
        if (u != 0) {
          m_edges.push_back((u - 1) * vPoints + v);
          m_edges.push_back(u * vPoints + v);
        }
      }
    }
  }
  if (m_type == ShaderType::SURFACEC2) {
    for (uint16_t u = 0; u < m_points.size() - 3; u++) {
      for (uint16_t v = 0; v < m_points[0].size() - 3; v++) {
        for (uint16_t du = 0; du < 4; du++) {
          for (uint16_t dv = 0; dv < 4; dv++) {
            m_edges.push_back(((u + du) % uPoints) * vPoints + v + dv);
          }
        }
      }
    }
    for (uint16_t u = 0; u < m_points.size() - 3; u++) {
      for (uint16_t v = 0; v < m_points[0].size() - 3; v++) {
        for (uint16_t dv = 0; dv < 4; dv++) {
          for (uint16_t du = 0; du < 4; du++) {
            m_edges.push_back(((u + du) % uPoints) * vPoints + v + dv);
          }
        }
      }
    }
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_edges.size() * sizeof(uint16_t),
               m_edges.data(), GL_STATIC_DRAW);
}

void BezierC2::notify() { setVertices(); }

void BezierC2::renderFramebuffer(std::shared_ptr<Renderer> &renderer,
                                 unsigned int c) {
  glBindVertexArray(m_vao);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(c);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_trimmingTexture);
  if (m_type == ShaderType::SURFACEC2) {
    renderer->reverseUV(false);
    renderer->setUVSubdivisions(m_divisionsU, m_divisionsV);
    glPatchParameteri(GL_PATCH_VERTICES, 16);
    glDrawElements(GL_PATCHES, m_edges.size() / 2, GL_UNSIGNED_SHORT,
                   (void *)0);
    renderer->setUVSubdivisions(m_divisionsV, m_divisionsU);
    renderer->reverseUV(true);
    glDrawElements(GL_PATCHES, m_edges.size() / 2, GL_UNSIGNED_SHORT,
                   (void *)(sizeof(uint16_t) * m_edges.size() / 2));
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  if (m_type == ShaderType::OBJECT) {
    glDrawElements(GL_LINES, m_edges.size(), GL_UNSIGNED_SHORT, (void *)0);
  }
}
void BezierC2::render(std::shared_ptr<Renderer> &renderer,
                      const math137::Vector4f &color) {
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(color);
  if (m_type == ShaderType::SURFACEC2) {
    renderer->setUVSubdivisions(m_divisionsU, m_divisionsV);
    glPatchParameteri(GL_PATCH_VERTICES, 16);
    glDrawElements(GL_PATCHES, m_edges.size() / 2, GL_UNSIGNED_SHORT,
                   (void *)0);
    renderer->setUVSubdivisions(m_divisionsV, m_divisionsU);
    glDrawElements(GL_PATCHES, m_edges.size() / 2, GL_UNSIGNED_SHORT,
                   (void *)(sizeof(uint16_t) * m_edges.size() / 2));
  }
  if (m_type == ShaderType::OBJECT) {
    glDrawElements(GL_LINES, m_edges.size(), GL_UNSIGNED_SHORT, (void *)0);
  }
}
float BezierC2::BSpline(int i, float t) const {
  switch (i) {
  case 0:
    return (1.f / 6.f) * (1 - t) * (1 - t) * (1 - t);
  case 1:
    return (1.0f / 6.0f) * (3 * t * t * t - 6 * t * t + 4);
  case 2:
    return (1.0f / 6.0f) * (-3 * t * t * t + 3 * t * t + 3 * t + 1);
  case 3:
    return (1.0f / 6.0f) * t * t * t;
  default:
    return 0;
  }
}
float BezierC2::dBSpline(int i, float t) const {
  switch (i) {
  case 0:
    return (-0.5f * (1 - t) * (1 - t));
  case 1:
    return (0.5f * (3 * t * t - 4 * t));
  case 2:
    return (0.5f * (-3 * t * t + 2 * t));
  case 3:
    return (0.5f * t * t);
  default:
    return 0;
  }
}
math137::Vector3f BezierC2::getValue(float u, float v) const {
  uint16_t ubPatches = m_points.size() - 3;
  uint16_t vbPatches = m_points[0].size() - 3;

  uint16_t uIndex = std::min(int(u * ubPatches), ubPatches - 1);
  uint16_t vIndex = std::min(int(v * vbPatches), vbPatches - 1);
  float nu = (u * ubPatches) - uIndex;
  float nv = (v * vbPatches) - vIndex;
  math137::Vector3f local;
  for (uint16_t du = 0; du < 4; du++) {
    for (uint16_t dv = 0; dv < 4; dv++) {
      local =
          local + m_points[uIndex + du][vIndex + dv].lock()->getTranslation() *
                      BSpline(du, nu) * BSpline(dv, nv);
    }
  }
  return local;
}
math137::Vector3f BezierC2::uDerivative(float u, float v) const {
  uint16_t ubPatches = m_points.size() - 3;
  uint16_t vbPatches = m_points[0].size() - 3;

  uint16_t uIndex = std::min(int(u * ubPatches), ubPatches - 1);
  uint16_t vIndex = std::min(int(v * vbPatches), vbPatches - 1);
  float nu = (u * ubPatches) - uIndex;
  float nv = (v * vbPatches) - vIndex;
  math137::Vector3f local;
  for (uint16_t du = 0; du < 4; du++) {
    for (uint16_t dv = 0; dv < 4; dv++) {
      local =
          local + m_points[uIndex + du][vIndex + dv].lock()->getTranslation() *
                      dBSpline(du, nu) * BSpline(dv, nv);
    }
  }
  return local;
}
math137::Vector3f BezierC2::vDerivative(float u, float v) const {
  uint16_t ubPatches = m_points.size() - 3;
  uint16_t vbPatches = m_points[0].size() - 3;

  uint16_t uIndex = std::min(int(u * ubPatches), ubPatches - 1);
  uint16_t vIndex = std::min(int(v * vbPatches), vbPatches - 1);
  float nu = (u * ubPatches) - uIndex;
  float nv = (v * vbPatches) - vIndex;
  math137::Vector3f local;
  for (uint16_t du = 0; du < 4; du++) {
    for (uint16_t dv = 0; dv < 4; dv++) {
      local =
          local + m_points[uIndex + du][vIndex + dv].lock()->getTranslation() *
                      BSpline(du, nu) * dBSpline(dv, nv);
    }
  }
  return local;
}
