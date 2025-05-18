#include "Surface.hpp"
#include "imgui.h"
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include <vector>

Surface::Surface(const std::vector<std::shared_ptr<Object>> &points,
                 uint16_t uPatches, uint16_t vPatches, bool cylinder)
    : Object(ShaderType::SURFACE), m_uPatches{uPatches}, m_vPatches(vPatches),
      m_cylinder(cylinder) {
  uint16_t uPoints = 4 + (uPatches - 1) * 3 - (cylinder ? 1 : 0);
  uint16_t vPoints = 4 + (vPatches - 1) * 3;

  m_divisionsV = 4;
  m_divisionsU = 4;

  m_points.resize(uPoints, std::vector<std::weak_ptr<Object>>(vPoints));
  for (uint16_t u = 0; u < uPoints; u++) {
    for (uint16_t v = 0; v < vPoints; v++) {
      m_points[u][v] = points[u * vPoints + v];
    }
  }

  setEdges();
}

void Surface::setEdges() {
  uint16_t uPoints = (4 + (m_uPatches - 1) * 3) - (m_cylinder ? 1 : 0);
  uint16_t vPoints = (4 + (m_vPatches - 1) * 3);
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
  }
  if (m_type == ShaderType::SURFACE) {
    m_edges.clear();
    for (uint16_t u = 0; u < m_uPatches; u++) {
      for (uint16_t v = 0; v < m_vPatches; v++) {
        for (uint16_t du = 0; du < 4; du++) {
          for (uint16_t dv = 0; dv < 4; dv++) {
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

bool Surface::renderObjectMenu() {
  if (!m_openMenu)
    return false;
  static const char *modes[] = {"Surface", "Grid"};
  static int mode = 0;
  ImGui::Begin(("Settings" + name).c_str(), &m_openMenu);
  setNameMenu();
  if (ImGui::Combo("Mode", &mode, modes, IM_ARRAYSIZE(modes))) {
    if (mode == 0)
      m_type = ShaderType::SURFACE;
    else
      m_type = ShaderType::OBJECT;
    setVertices();
    setEdges();
  }
  ImGui::SliderInt("U subdivisions", &m_divisionsU, 1, 10);
  ImGui::SliderInt("V subdivisions", &m_divisionsV, 1, 10);
  ImGui::End();
  return false;
}
