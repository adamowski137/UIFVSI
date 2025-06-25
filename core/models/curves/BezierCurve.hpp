#pragma once

#include "../Object.hpp"
#include "Curve.hpp"
#include "Vector.hpp"
#include <vector>

class BezierCurve : public Curve {
public:
  BezierCurve(const std::vector<std::weak_ptr<Object>> &points);
  void render(std::shared_ptr<Renderer> &renderer,
              const math137::Vector4f &color) override;
  void renderFramebuffer(std::shared_ptr<Renderer> &renderer,
                         unsigned int id) override;
  bool renderObjectMenu() override;
  void notify() override;

private:
  std::string getTypeName() const override { return "bezierC0"; }
  void recalculateModel() override;
  void setVertices() override;
  void setEdges() const override;
};
