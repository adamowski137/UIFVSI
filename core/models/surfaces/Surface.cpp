#include "Surface.hpp"
#include "Vector.hpp"
#include "imgui.h"
#include "BezierC2.hpp"
#include "BezierC0.hpp"
#include "../Intersectable.hpp"
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include <vector>

Surface::Surface(
    const std::vector<std::vector<std::shared_ptr<Object>>> &points,
    uint16_t uPatches, uint16_t vPatches, ShaderType type)
    : Object(type), m_uPatches{uPatches}, m_vPatches(vPatches) {
  uint16_t uPoints = points.size();
  uint16_t vPoints = points[0].size();

  m_divisionsV = 4;
  m_divisionsU = 4;

  m_points.resize(uPoints, std::vector<std::weak_ptr<Object>>(vPoints));
  for (uint16_t u = 0; u < uPoints; u++) {
    for (uint16_t v = 0; v < vPoints; v++) {
      m_points[u][v] = points[u][v];
    }
  }
}

math137::Vector3f Surface::getMassCenter() {
  math137::Vector3f res;
  for (const auto &v1 : m_points)
    for (const auto &p : v1)
      res = res + p.lock()->getMassCenter();

  return res / (m_points.size() * m_points[0].size());
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

void Surface::replacePoint(const std::weak_ptr<Object> &current,
                           const std::shared_ptr<Object> &newPoint) {
  for (uint16_t i = 0; i < m_points.size(); i++) {
    for (uint16_t j = 0; j < m_points[i].size(); j++) {
      if (m_points[i][j].lock() == current.lock())
        m_points[i][j] = newPoint;
    }
  }
}

