#pragma once

#include "Curve.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>
#include <vector>

class BSpline : public Curve {
public:
  BSpline(const std::vector<std::weak_ptr<Object>> &points);
  void render(std::shared_ptr<Renderer> &renderer,
              const math137::Vector4f &color) override;
  bool renderObjectMenu() override;
  void notify() override;
  std::vector<std::weak_ptr<Object>> getVirtualObjects() const override;
  void notifyVirtual(uint16_t index, const math137::Vector3f &pos) override;
  void addPoint(const std::weak_ptr<Object> &p) override;
  void removePoint(const std::weak_ptr<Object> &p) override;

private:
  void recalculateModel() override;
  void setVertices() override;
  void setEdges() const override;
  std::string getTypeName() const override { return "bezierC2"; }

  std::vector<std::shared_ptr<Object>> m_bernsteinPoints;

  static uint16_t s_count;
};
