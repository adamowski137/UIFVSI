#include "Camera.hpp"
#include "MatrixUtils.hpp"
#include "Vector.hpp"

Camera::Camera(math137::Vector3f positon, math137::Vector3f front,
               math137::Vector3f up)
    : m_position(positon), m_front(front), m_up(up),
      m_view(math137::MatrixUtils::Identity()) {}
void Camera::recalculateView() {
  m_view = math137::MatrixUtils::LookAt(m_position, m_position + m_front, m_up);
}

void Camera::rotateCamera(float dx, float dy) {
  m_front.x(m_front.x() + dx);
  m_front.y(m_front.y() + dy);
  recalculateView();
}

void Camera::moveCamera(float dx, float dy) { recalculateView(); }
