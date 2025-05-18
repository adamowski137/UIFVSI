#include "BezierC2.hpp"
#include "Surface.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>
#include <vector>

uint16_t BezierC2::s_count = 0;

BezierC2::BezierC2(const std::vector<std::shared_ptr<Object>> &points,
                   uint16_t uPatches, uint16_t vPatches, bool cylinder)
    : Surface(points, uPatches, vPatches, cylinder) {
  name = "BezierCurve " + std::to_string(s_count++);
  m_bezierPoints.resize(4 + (uPatches - 1) * 3 - (cylinder ? 1 : 0),
                        std::vector<math137::Vector3f>(4 + (vPatches - 1) * 3));
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  setVertices();
}

void BezierC2::setVertices() {
  std::vector<float> points;
  if (m_type == ShaderType::OBJECT) {
    for (uint16_t i = 0; i < m_points.size(); i++) {
      for (uint16_t j = 0; j < m_points[i].size(); j++) {
        math137::Vector3f pos = m_points[i][j].lock()->getTranslation();
        points.push_back(pos.x());
        points.push_back(pos.y());
        points.push_back(pos.z());
      }
    }
  } else {
    uint16_t uPoints = (4 + (m_uPatches - 1) * 3) - (m_cylinder ? 1 : 0);
    const float conversionMatrix[4][4] = {{1.0f / 6, 4.0f / 6, 1.0f / 6, 0.0f},
                                          {0, 2.0f / 3, 1.0f / 3, 0.0f},
                                          {0, 1.0f / 3, 2.0f / 3, 0.0f},
                                          {0, 1.0f / 6, 4.0f / 6, 1.0f / 6}};

    for (uint16_t u = 0; u < (4 + (m_uPatches - 1) * 3) - (m_cylinder ? 1 : 3);
         u++) {
      for (uint16_t v = 0; v < m_points[0].size() - 3; v++) {
        math137::Vector3f patch[4][4];

        for (uint i = 0; i < 4; i++) {
          for (uint j = 0; j < 4; j++) {
            patch[i][j] =
                m_points[(u + i) % uPoints][v + j].lock()->getTranslation();
          }
        }

        math137::Vector3f temp[4][4];
        for (int i = 0; i < 4; i++) {
          for (int j = 0; j < 4; j++) {
            temp[i][j] = {};
            for (int k = 0; k < 4; k++) {
              temp[i][j] = temp[i][j] + patch[k][j] * conversionMatrix[i][k];
            }
          }
        }

        math137::Vector3f bezierPatch[4][4];
        for (int i = 0; i < 4; i++) {
          for (int j = 0; j < 4; j++) {
            bezierPatch[i][j] = {};
            for (int k = 0; k < 4; k++) {
              bezierPatch[i][j] =
                  bezierPatch[i][j] + temp[i][k] * conversionMatrix[j][k];
            }
          }
        }
        for (int i = 0; i < 4; i++) {
          for (int j = 0; j < 4; j++) {
            points.push_back(bezierPatch[i][j].x());
            points.push_back(bezierPatch[i][j].y());
            points.push_back(bezierPatch[i][j].z());
          }
        }
      }
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(),
               GL_STATIC_DRAW);
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
