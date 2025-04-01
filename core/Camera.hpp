#pragma once

#include "Matrix.hpp"
#include "Vector.hpp"

class Camera {
public:
  Camera(float distance, math137::Vector3f target);

  inline math137::Matrix4f getView() const { return m_view; };
  inline math137::Vector3f getPosition() const { return m_position; }
  math137::Matrix4f getInverseView() const;
  void rotateCamera(float dx, float dy);
  void moveCamera(float dx);
  inline void changeDistance(float dx) {
    m_distance += dx;
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

  const float sensitivity = 1.f;
};
