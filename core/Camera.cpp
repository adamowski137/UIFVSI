#include "Camera.hpp"
#include "Matrix.hpp"
#include "MatrixUtils.hpp"
#include "Vector.hpp"
#include <cmath>

Camera::Camera(float distance, math137::Vector3f target)
    : m_distance(distance), m_target(target), m_pitch(0), m_yaw(0),
      m_view(math137::MatrixUtils::Identity()) {
  recalculateView();
}

math137::Matrix4f Camera::getInverseView() const {
  math137::Matrix4f matrix = m_view;
  matrix.setValue(1, 0, m_view.getValue(0, 1));
  matrix.setValue(2, 0, m_view.getValue(0, 2));
  matrix.setValue(2, 1, m_view.getValue(1, 2));
  matrix.setValue(0, 1, m_view.getValue(1, 0));
  matrix.setValue(0, 2, m_view.getValue(2, 0));
  matrix.setValue(1, 2, m_view.getValue(2, 1));
  float x = matrix.getValue(0, 0) * m_view.getValue(0, 3) +
            matrix.getValue(0, 1) * m_view.getValue(1, 3) +
            matrix.getValue(0, 2) * m_view.getValue(2, 3);
  float y = matrix.getValue(1, 0) * m_view.getValue(0, 3) +
            matrix.getValue(1, 1) * m_view.getValue(1, 3) +
            matrix.getValue(1, 2) * m_view.getValue(2, 3);
  float z = matrix.getValue(2, 0) * m_view.getValue(0, 3) +
            matrix.getValue(2, 1) * m_view.getValue(1, 3) +
            matrix.getValue(2, 2) * m_view.getValue(2, 3);

  matrix.setValue(0, 3, -x);
  matrix.setValue(1, 3, -y);
  matrix.setValue(2, 3, -z);

  return matrix;
}

void Camera::recalculateView() {
  float x = m_distance * cosf(m_pitch) * cosf(m_yaw);
  float y = m_distance * sinf(m_pitch);
  float z = m_distance * cosf(m_pitch) * sin(m_yaw);
  m_position = math137::Vector3f(x, y, z) + m_target;
  m_view = math137::MatrixUtils::LookAt(m_position, m_target, {0.f, 1.f, 0.f});
}

void Camera::rotateCamera(float dx, float dy) {
  m_yaw += (sensitivity * dx);
  m_pitch += (sensitivity * dy);
  if (m_pitch >= M_PI_2)
    m_pitch = M_PI_2 - 0.01f;
  if (m_pitch <= -M_PI_2)
    m_pitch = -M_PI_2 + 0.01f;
  recalculateView();
}
