#include "BezierC2.hpp"
#include "Surface.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>
#include <vector>

uint16_t BezierC2::s_count = 0;

BezierC2::BezierC2(const std::vector<std::shared_ptr<Object>> &points,
                   uint16_t uPatches, uint16_t vPatches)
    : Surface(points, uPatches, vPatches, ShaderType::SURFACEC2) {
  name = "BezierCurve " + std::to_string(s_count++);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  setVertices();
  setEdges();
}

void BezierC2::setVertices() {
  std::vector<float> points;
  for (uint16_t i = 0; i < m_points.size(); i++) {
    for (uint16_t j = 0; j < m_points[i].size(); j++) {
      math137::Vector3f pos = m_points[i][j].lock()->getTranslation();
      points.push_back(pos.x());
      points.push_back(pos.y());
      points.push_back(pos.z());
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
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
