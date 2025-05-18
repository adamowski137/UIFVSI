#pragma once

#include "Object.hpp"
#include "Quaternion.hpp"
#include "Vector.hpp"
#include <memory>
#include <vector>
class ObjectBuilder {
public:
  ObjectBuilder &withNewTorus(float R, float r);
  ObjectBuilder &withNewPoint();
  ObjectBuilder &
  withNewBezierCurve(const std::vector<std::weak_ptr<Object>> &points);
  ObjectBuilder &
  withNewBSpline(const std::vector<std::weak_ptr<Object>> &points);
  ObjectBuilder &
  withNewIBSpline(const std::vector<std::weak_ptr<Object>> &points);
  ObjectBuilder &withPosition(const math137::Vector3f &pos);
  ObjectBuilder &withRotation(const math137::Quaternion &rot);
  ObjectBuilder &withScale(const math137::Vector3f &scale);
  std::shared_ptr<Object> build();

private:
  std::shared_ptr<Object> m_object;
};
