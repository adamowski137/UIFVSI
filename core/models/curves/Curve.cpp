#include "Curve.hpp"
#include <vector>

void Curve::addPoint(const std::weak_ptr<Object> &p) {
  if (containsPoint(p))
    return;
  m_points.push_back(p);
}
void Curve::removePoint(const std::weak_ptr<Object> &p) {
  for (int i = 0; i < m_points.size(); i++) {
    if (m_points[i].lock() == p.lock()) {
      m_points.erase(m_points.begin() + i);
      break;
    }
  }
}
bool Curve::containsPoint(const std::weak_ptr<Object> &p) const {
  for (const auto &it : m_points) {
    if (it.lock() == p.lock())
      return true;
  }
  return false;
}

void Curve::replacePoint(const std::weak_ptr<Object> &current,
                         const std::shared_ptr<Object> &newPoint) {
  for (uint16_t i = 0; i < m_points.size(); i++) {
    if (m_points[i].lock() == current.lock())
      m_points[i] = newPoint;
  }
}
