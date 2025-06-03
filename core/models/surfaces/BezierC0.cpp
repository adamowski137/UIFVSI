#include "BezierC0.hpp"
#include "Surface.hpp"
#include "Vector.hpp"
#include <array>
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include <vector>

uint16_t BezierC0::s_count = 0;

BezierC0::BezierC0(const std::vector<std::shared_ptr<Object>> &points,
                   uint16_t uPatches, uint16_t vPatches, bool cylinder)
    : Surface(points, uPatches, vPatches, cylinder, ShaderType::SURFACE) {
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
  uint16_t uPoints = (4 + (m_uPatches - 1) * 3) - (m_cylinder ? 1 : 0);
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

    if (m_cylinder) {
      for (uint16_t v = 0; v < vPoints; v++) {
        m_edges.push_back((uPoints - 1) * vPoints + v);
        m_edges.push_back(v);
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
  for (uint16_t u = 0; u < m_uPatches; u++) {
    std::weak_ptr<Object> lt = m_points[u * 3][0];
    std::weak_ptr<Object> rt = m_points[u * 3][3];
    std::weak_ptr<Object> lb = m_points[u * 3 + 3][0];
    std::weak_ptr<Object> rb = m_points[u * 3 + 3][3];

    graph[lt.lock()].insert(rt.lock());
    graph[lt.lock()].insert(lb.lock());
    graph[rt.lock()].insert(lt.lock());
    graph[rt.lock()].insert(rb.lock());
    graph[lb.lock()].insert(rb.lock());
    graph[lb.lock()].insert(lt.lock());
    graph[rb.lock()].insert(rt.lock());
    graph[rb.lock()].insert(lb.lock());

    lt = m_points[u * 3][m_points[0].size() - 4];
    rt = m_points[u * 3][m_points[0].size() - 1];
    lb = m_points[u * 3 + 3][m_points[0].size() - 4];
    rb = m_points[u * 3 + 3][m_points[0].size() - 1];

    graph[lt.lock()].insert(rt.lock());
    graph[lt.lock()].insert(lb.lock());
    graph[rt.lock()].insert(lt.lock());
    graph[rt.lock()].insert(rb.lock());
    graph[lb.lock()].insert(rb.lock());
    graph[lb.lock()].insert(lt.lock());
    graph[rb.lock()].insert(rt.lock());
    graph[rb.lock()].insert(lb.lock());
  }

  for (uint16_t v = 0; v < m_vPatches; v++) {
    std::weak_ptr<Object> lt = m_points[0][v * 3];
    std::weak_ptr<Object> rt = m_points[3][v * 3];
    std::weak_ptr<Object> lb = m_points[0][v * 3 + 3];
    std::weak_ptr<Object> rb = m_points[3][v * 3 + 3];

    graph[lt.lock()].insert(rt.lock());
    graph[lt.lock()].insert(lb.lock());
    graph[rt.lock()].insert(lt.lock());
    graph[rt.lock()].insert(rb.lock());
    graph[lb.lock()].insert(rb.lock());
    graph[lb.lock()].insert(lt.lock());
    graph[rb.lock()].insert(rt.lock());
    graph[rb.lock()].insert(lb.lock());

    lt = m_points[m_points[0].size() - 4][v * 3];
    rt = m_points[m_points[0].size() - 1][v * 3];
    lb = m_points[m_points[0].size() - 4][v * 3 + 3];
    rb = m_points[m_points[0].size() - 1][v * 3 + 3];

    graph[lt.lock()].insert(rt.lock());
    graph[lt.lock()].insert(lb.lock());
    graph[rt.lock()].insert(lt.lock());
    graph[rt.lock()].insert(rb.lock());
    graph[lb.lock()].insert(rb.lock());
    graph[lb.lock()].insert(lt.lock());
    graph[rb.lock()].insert(rt.lock());
    graph[rb.lock()].insert(lb.lock());
  }

  return graph;
}

std::pair<std::array<std::weak_ptr<Object>, 4>,
          std::array<std::weak_ptr<Object>, 4>>
BezierC0::getEdgeFromPoints(const std::shared_ptr<Object> &p1,
                            const std::shared_ptr<Object> &p2) const {
  for (uint16_t u = 0; u < m_uPatches; u++) {
    for (uint16_t v = 0; v < m_vPatches; v++) {
      uint16_t up1, up2, vp1, vp2;
      for (uint16_t cu = 0; cu < 2; cu++) {
        for (uint16_t cv = 0; cv < 2; cv++) {
          if (m_points[(u + cu) * 3][(v + cv) * 3].lock() == p1) {
            up1 = cu;
            vp1 = cv;
          }
          if (m_points[(u + cu) * 3][(v + cv) * 3].lock() == p2) {
            up2 = cu;
            vp2 = cv;
          }
        }
      }
    }
    uint16_t du = 0;
    if (p1 != m_points[u * 3][0].lock())
      continue;
    if (u > 0 && p2 == m_points[(u - 1) * 3][0].lock()) {
      du = -1;
    }
    if (u < m_uPatches - 1 && p2 == m_points[(u + 1) * 3][0].lock()) {
      du = 1;
    }

    std::array<std::weak_ptr<Object>, 4> points = {
        m_points[u * 3 + 0 * du][0],
        m_points[u * 3 + 1 * du][0],
        m_points[u * 3 + 2 * du][0],
        m_points[u * 3 + 3 * du][0],
    };
    std::array<std::weak_ptr<Object>, 4> prev = {
        m_points[u * 3 + 0 * du][1],
        m_points[u * 3 + 1 * du][1],
        m_points[u * 3 + 2 * du][1],
        m_points[u * 3 + 3 * du][1],
    };

    return {points, prev};
  }
  for (uint16_t u = 0; u < m_uPatches; u++) {
    uint16_t du = 0;
    if (p1 != m_points[u * 3][m_points[0].size() - 1].lock())
      continue;
    if (u > 0 && p2 == m_points[(u - 1) * 3][m_points[0].size() - 1].lock()) {
      du = -1;
    }
    if (u < m_uPatches - 1 &&
        p2 == m_points[(u + 1) * 3][m_points[0].size() - 1].lock()) {
      du = 1;
    }

    std::array<std::weak_ptr<Object>, 4> points = {
        m_points[u * 3 + 0 * du][m_points[0].size() - 1],
        m_points[u * 3 + 1 * du][m_points[0].size() - 1],
        m_points[u * 3 + 2 * du][m_points[0].size() - 1],
        m_points[u * 3 + 3 * du][m_points[0].size() - 1],
    };
    std::array<std::weak_ptr<Object>, 4> prev = {
        m_points[u * 3 + 0 * du][m_points[0].size() - 2],
        m_points[u * 3 + 1 * du][m_points[0].size() - 2],
        m_points[u * 3 + 2 * du][m_points[0].size() - 2],
        m_points[u * 3 + 3 * du][m_points[0].size() - 2],
    };

    return {points, prev};
  }

  for (uint16_t v = 0; v < m_vPatches; v++) {
    uint16_t dv = 0;
    if (p1 != m_points[0][v * 3].lock())
      continue;
    if (v > 0 && p2 == m_points[0][(v - 1) * 3].lock()) {
      dv = -1;
    }
    if (v < m_vPatches - 1 && p2 == m_points[0][(v + 1) * 3].lock()) {
      dv = 1;
    }

    std::array<std::weak_ptr<Object>, 4> points = {
        m_points[0][v * 3 + 0 * dv], m_points[0][v * 3 + 1 * dv],
        m_points[0][v * 3 + 2 * dv], m_points[0][v * 3 + 3 * dv]};
    std::array<std::weak_ptr<Object>, 4> prev = {
        m_points[1][v * 3 + 0 * dv], m_points[1][v * 3 + 1 * dv],
        m_points[1][v * 3 + 2 * dv], m_points[1][v * 3 + 3 * dv]};

    return {points, prev};
  }

  for (uint16_t v = 0; v < m_vPatches; v++) {
    uint16_t dv = 0;
    if (p1 != m_points[m_points.size() - 1][v * 3].lock())
      continue;
    if (v > 0 && p2 == m_points[m_points.size() - 1][(v - 1) * 3].lock()) {
      dv = -1;
    }
    if (v < m_vPatches - 1 &&
        p2 == m_points[m_points.size() - 1][(v + 1) * 3].lock()) {
      dv = 1;
    }

    std::array<std::weak_ptr<Object>, 4> points = {
        m_points[m_points.size() - 1][v * 3 + 0 * dv],
        m_points[m_points.size() - 1][v * 3 + 1 * dv],
        m_points[m_points.size() - 1][v * 3 + 2 * dv],
        m_points[m_points.size() - 1][v * 3 + 3 * dv]};
    std::array<std::weak_ptr<Object>, 4> prev = {
        m_points[m_points.size() - 2][v * 3 + 0 * dv],
        m_points[m_points.size() - 2][v * 3 + 1 * dv],
        m_points[m_points.size() - 2][v * 3 + 2 * dv],
        m_points[m_points.size() - 2][v * 3 + 3 * dv]};

    return {points, prev};
  }
  return {};
}
