#pragma once

#include "../Intersectable.hpp"
#include "BezierC2.hpp"
#include <memory>

class ShiftedSurface : public Intersectable
{
public:
  ShiftedSurface(std::shared_ptr<BezierC2> base, float radius = 0.0f);
  ~ShiftedSurface() override = default;

  bool wrappableU() const override;
  bool wrappableV() const override;
  math137::Vector3f uDerivative(float u, float v) const override;
  math137::Vector3f vDerivative(float u, float v) const override;
  math137::Vector3f getValue(float u, float v) const override;

  // Delegate trimming operations to the base surface so that trimming is
  // applied/queried on the underlying BezierC2.
  void intersectTrimmingTexture(uint16_t x, uint16_t y) override;
  void unionTrimmingTexture(uint16_t x, uint16_t y) override;
  void resetTrimming() override;
  bool isTrimmedUV(float u, float v) const override;

  float getRadius() const { return m_radius; }

private:
  std::shared_ptr<BezierC2> m_base;
  float m_radius = 0.0f;
};
