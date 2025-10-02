#pragma once

#include "Matrix.hpp"
#include "Vector.hpp"
#include <cmath>

class Camera {
public:
  Camera(float distance, math137::Vector3f target);

  inline math137::Matrix4f getView() const { return m_view; };
  inline math137::Vector3f getPosition() const { return m_position; }
  math137::Matrix4f getInverseView() const;
  void rotateCamera(float dx, float dy);

  inline void setTarget(const math137::Vector3f &v) {
    m_target = v;
    math137::Vector3f delta = v - m_position;
    float xzLength = sqrtf(delta.x() * delta.x() + delta.z() * delta.z());
    m_distance = sqrtf(delta * delta);
    m_yaw = atan2f(delta.x(), delta.y());
    m_pitch = atan2f(delta.y(), xzLength);
  }

  inline void moveTarget(const math137::Vector3f &v) {
    m_target = m_target + v;
    recalculateView();
  }

  inline void changeDistance(float dx) {
    m_distance += dx;
    constexpr float eps = 0.01f;
    if (m_distance < eps)
      m_distance = eps;
    recalculateView();
  }

private:
  void recalculateView();

  float m_distance;
  float m_yaw;
  float m_pitch;
  math137::Vector3f m_position;
  math137::Vector3f m_target;
  math137::Matrix4f m_view;

  const float sensitivity = 0.01f;
};
