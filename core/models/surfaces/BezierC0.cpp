#include "BezierC0.hpp"
#include "Surface.hpp"
#include "Vector.hpp"
#include <array>
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include <utility>
#include <vector>

uint16_t BezierC0::s_count = 0;

BezierC0::BezierC0(const std::vector<std::shared_ptr<Object>> &points,
                   uint16_t uPatches, uint16_t vPatches)
    : Surface(points, uPatches, vPatches, ShaderType::SURFACE) {
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

void BezierC0::setEdges() {
  uint16_t uPoints = (4 + (m_uPatches - 1) * 3);
  uint16_t vPoints = (4 + (m_vPatches - 1) * 3);
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

  } else {
    for (uint16_t u = 0; u < m_uPatches; u++) {
      for (uint16_t v = 0; v < m_vPatches; v++) {
        for (uint16_t du = 0; du < 4; du++) {
          for (uint16_t dv = 0; dv < 4; dv++) {
            m_edges.push_back(((u * 3 + du) % uPoints) * vPoints + v * 3 + dv);
          }
        }
      }
    }
    for (uint16_t u = 0; u < m_uPatches; u++) {
      for (uint16_t v = 0; v < m_vPatches; v++) {
        for (uint16_t dv = 0; dv < 4; dv++) {
          for (uint16_t du = 0; du < 4; du++) {
            m_edges.push_back(((u * 3 + du) % uPoints) * vPoints + v * 3 + dv);
          }
        }
      }
    }
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_edges.size() * sizeof(uint16_t),
               m_edges.data(), GL_STATIC_DRAW);
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

std::map<std::shared_ptr<Object>, std::set<std::shared_ptr<Object>>>
BezierC0::getConnectionsGraph() const {
  std::map<std::shared_ptr<Object>, std::set<std::shared_ptr<Object>>> graph;
  for (uint16_t u = 0; u < m_uPatches + 1; u++) {
    uint16_t vPoints = m_points[0].size() - 1;
    std::weak_ptr<Object> currTop = m_points[u * 3][0];
    std::weak_ptr<Object> currBottom = m_points[u * 3][vPoints];
    if (u != 0) {
      graph[currTop.lock()].insert(m_points[(u - 1) * 3][0].lock());
      graph[currBottom.lock()].insert(m_points[(u - 1) * 3][vPoints].lock());
    }
    if (u != m_uPatches) {
      graph[currTop.lock()].insert(m_points[(u + 1) * 3][0].lock());
      graph[currBottom.lock()].insert(m_points[(u + 1) * 3][vPoints].lock());
    }
  }
  for (uint16_t v = 0; v < m_vPatches + 1; v++) {
    uint16_t uPoints = m_points.size() - 1;
    std::weak_ptr<Object> currTop = m_points[0][v * 3];
    std::weak_ptr<Object> currBotttom = m_points[uPoints][v * 3];
    if (v != 0) {
      graph[currTop.lock()].insert(m_points[0][(v - 1) * 3].lock());
      graph[currBotttom.lock()].insert(m_points[uPoints][(v - 1) * 3].lock());
    }
    if (v != m_vPatches) {
      graph[currTop.lock()].insert(m_points[0][(v + 1) * 3].lock());
      graph[currBotttom.lock()].insert(m_points[uPoints][(v + 1) * 3].lock());
    }
  }

  return graph;
}

std::pair<std::array<std::weak_ptr<Object>, 4>,
          std::array<std::weak_ptr<Object>, 4>>
BezierC0::getEdgeFromPoints(const std::shared_ptr<Object> &p1,
                            const std::shared_ptr<Object> &p2) const {
  static const std::array<std::pair<uint16_t, uint16_t>, 8> conf = {
      std::make_pair(0, 1),
      {0, 2},
      {1, 3},
      {2, 3},
      {1, 0},
      {2, 0},
      {3, 1},
      {3, 2}};
  uint16_t ru = 0, rv = 0;
  uint16_t up1, up2, vp1, vp2;
  for (uint16_t i = 0; i < m_uPatches * m_vPatches; i++) {
    uint16_t u = i / m_vPatches;
    uint16_t v = i % m_vPatches;
    bool found1 = false, found2 = false;
    for (const auto &[pos1, pos2] : conf) {
      uint16_t u1 = pos1 / 2;
      uint16_t u2 = pos2 / 2;
      uint16_t v1 = pos1 % 2;
      uint16_t v2 = pos2 % 2;
      if (m_points[(u + u1) * 3][(v + v1) * 3].lock() == p1 &&
          m_points[(u + u2) * 3][(v + v2) * 3].lock() == p2) {
        up1 = u1;
        vp1 = v1;
        ru = u;
        rv = v;
        up2 = u2;
        vp2 = v2;
      }
    }
  }
  int du = (up2 - up1);
  int dv = (vp2 - vp1);
  int ddu = 0, ddv = 0;
  if (up1 == 0 && up2 == 0)
    ddu = 1;
  if (up1 == 1 && up2 == 1)
    ddu = -1;
  if (vp1 == 0 && vp2 == 0)
    ddv = 1;
  if (vp1 == 1 && vp2 == 1)
    ddv = -1;

  std::array<std::weak_ptr<Object>, 4> points = {
      m_points[(ru + up1) * 3 + 0 * du][(rv + vp1) * 3 + 0 * dv],
      m_points[(ru + up1) * 3 + 1 * du][(rv + vp1) * 3 + 1 * dv],
      m_points[(ru + up1) * 3 + 2 * du][(rv + vp1) * 3 + 2 * dv],
      m_points[(ru + up1) * 3 + 3 * du][(rv + vp1) * 3 + 3 * dv]};

  std::array<std::weak_ptr<Object>, 4> diff = {
      m_points[(ru + up1) * 3 + 0 * du + ddu][(rv + vp1) * 3 + 0 * dv + ddv],
      m_points[(ru + up1) * 3 + 1 * du + ddu][(rv + vp1) * 3 + 1 * dv + ddv],
      m_points[(ru + up1) * 3 + 2 * du + ddu][(rv + vp1) * 3 + 2 * dv + ddv],
      m_points[(ru + up1) * 3 + 3 * du + ddu][(rv + vp1) * 3 + 3 * dv + ddv]};
  return {points, diff};
}
