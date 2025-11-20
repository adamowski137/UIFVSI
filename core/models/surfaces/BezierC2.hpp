#pragma once

#include "../Intersectable.hpp"
#include "Surface.hpp"
#include "Vector.hpp"
#include <iostream>
#include <vector>

class BezierC2 : public Surface, public Intersectable {
public:
  BezierC2(const std::vector<std::vector<std::shared_ptr<Object>>> &points,
           uint16_t uPatches, uint16_t vPatches);
  ~BezierC2() {}


  void notify() override;
  void render(std::shared_ptr<Renderer> &renderer,
              const math137::Vector4f &color) override;
  void renderFramebuffer(std::shared_ptr<Renderer> &renderer,
                         unsigned int id) override;
  virtual math137::Vector3f uDerivative(float u, float v) const override;
  virtual math137::Vector3f vDerivative(float u, float v) const override;
  math137::Vector3f uuDerivative(float u, float v) const;
  math137::Vector3f vvDerivative(float u, float v) const;
  math137::Vector3f uvDerivative(float u, float v) const;
  virtual math137::Vector3f getValue(float u, float v) const override;
  virtual bool wrappableV() const override {
    return m_points[0][0].lock() == m_points[0][m_points[0].size() - 3].lock();
  }
  virtual bool wrappableU() const override {
    return m_points[0][0].lock() == m_points[m_points.size() - 3][0].lock();
  }

private:
  std::string getTypeName() const override { return "bezierSurfaceC2"; }
  void setVertices() override;
  void setEdges() override;
  void recalculateModel() override { m_update = false; }
  float BSpline(int i, float t) const;
  float dBSpline(int i, float t) const;
  float ddBSpline(int i, float t) const;
};
