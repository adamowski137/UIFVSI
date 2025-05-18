#pragma once

#include "../Object.hpp"
#include "Curve.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <vector>

class BezierCurve : public Curve {
public:
  BezierCurve(const std::vector<std::weak_ptr<Object>> &points);
  void render(std::shared_ptr<Renderer> &renderer,
              const math137::Vector4f &color) override;
  bool renderObjectMenu() override;
  void notify() override;

private:
  void recalculateModel() override;
  void setVertices() override;
  void setEdges() const override;

  static uint16_t s_count;
};
