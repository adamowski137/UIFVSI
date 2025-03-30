#pragma once

#include "Matrix.hpp"
#include "Vector.hpp"

class Camera {
public:
  Camera(math137::Vector3f position, math137::Vector3f front,
         math137::Vector3f up);

  inline math137::Matrix4f getView() const { return m_view; };

  void rotateCamera(float dx, float dy);
  void moveCamera(float dx, float dy);

private:
  void recalculateView();

  math137::Vector3f m_position;
  math137::Vector3f m_front;
  math137::Vector3f m_up;
  math137::Matrix4f m_view;
};
