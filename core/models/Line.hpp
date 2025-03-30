#pragma once
#include "Object.hpp"
#include "Point.hpp"
#include <cstdint>
#include <memory>

class Line : public Object {
public:
  Line();
  virtual void render() const override;
  virtual void renderObjectMenu() override;
  void addPoint(std::shared_ptr<Point> p);
  void removePoint(const std::shared_ptr<Point> &p);
  bool containsPoint(const std::shared_ptr<Point> &p) const;
  void setVertexData() const;

private:
  static uint16_t s_count;

private:
  std::vector<std::weak_ptr<Point>> m_points;
};
