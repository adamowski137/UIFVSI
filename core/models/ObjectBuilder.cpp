#include "ObjectBuilder.hpp"
#include "BezierCurve.hpp"
#include "Point.hpp"
#include "Torus.hpp"
#include "Vector.hpp"
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

ObjectBuilder &
ObjectBuilder::withNewBezierCurve(std::vector<std::shared_ptr<Point>> points) {
  m_object = std::make_shared<BezierCurve>(points);
  return *this;
}

ObjectBuilder &ObjectBuilder::withPosition(const math137::Vector3f &pos) {
  m_object->setTranslation(pos);
  return *this;
}

ObjectBuilder &ObjectBuilder::withScale(const math137::Vector3f &scale) {
  m_object->setScale(scale);
  return *this;
}

std::shared_ptr<Object> ObjectBuilder::build() {
  std::shared_ptr<Object> t = m_object;
  m_object.reset();
  return t;
}
