#include "ObjectBuilder.hpp"
#include "Object.hpp"
#include "Quaternion.hpp"
#include "Vector.hpp"
#include "curves/BSpline.hpp"
#include "curves/BezierCurve.hpp"
#include "curves/IBSpline.hpp"
#include "primitives/Point.hpp"
#include "primitives/Torus.hpp"
#include "surfaces/BezierC0.hpp"
#include <cmath>
#include <memory>
#include <vector>

ObjectBuilder &ObjectBuilder::withNewTorus(float R, float r) {
  m_object = std::make_shared<Torus>(R, r);
  return *this;
}

ObjectBuilder &ObjectBuilder::withNewPoint() {
  m_object = std::make_shared<Point>();
  return *this;
}

ObjectBuilder &ObjectBuilder::withNewBezierCurve(
    const std::vector<std::weak_ptr<Object>> &points) {
  m_object = std::make_shared<BezierCurve>(points);
  return *this;
}

ObjectBuilder &ObjectBuilder::withNewBSpline(
    const std::vector<std::weak_ptr<Object>> &points) {
  m_object = std::make_shared<BSpline>(points);
  return *this;
}

ObjectBuilder &ObjectBuilder::withNewIBSpline(
    const std::vector<std::weak_ptr<Object>> &points) {
  m_object = std::make_shared<IBSpline>(points);
  return *this;
}

ObjectBuilder &ObjectBuilder::withPosition(const math137::Vector3f &pos) {
  m_object->setTranslation(pos);
  return *this;
}

ObjectBuilder &ObjectBuilder::withRotation(const math137::Quaternion &rot) {
  m_object->setRotation(rot);
  return *this;
}

ObjectBuilder &ObjectBuilder::withScale(const math137::Vector3f &scale) {
  m_object->setScale(scale);
  return *this;
}

ObjectBuilder &ObjectBuilder::withId(const uint16_t val) {
  m_object->m_id = val;
  if (Object::s_itemCount < val)
    Object::s_itemCount = val;
  return *this;
}

std::shared_ptr<Object> ObjectBuilder::build() {
  std::shared_ptr<Object> t = m_object;
  m_object.reset();
  return t;
}
