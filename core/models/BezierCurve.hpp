#pragma once
#include "Object.hpp"
#include "Point.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>
#include <vector>

class BezierCurve : public Object {
public:
  BezierCurve(const std::vector<std::shared_ptr<Point>> &points);
  void render(std::shared_ptr<Renderer> &renderer,
              const math137::Vector4f &color) override;
  void renderObjectMenu() override;
  void notify() override;
  void addPoint(std::shared_ptr<Point> p);
  void removePoint(const std::shared_ptr<Point> &p);
  bool containsPoint(const std::shared_ptr<Object> &p) const;
  void setVertices() const;
  void setEdges() const;

private:
  void recalculateModel() override;
  std::vector<std::weak_ptr<Point>> m_points;

  static uint16_t s_count;
};
