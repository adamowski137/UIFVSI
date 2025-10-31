#pragma once
#include "Vector.hpp"
#include <cstdint>
#include <vector>
class Intersectable {
public:
  Intersectable();
  virtual ~Intersectable();
  bool virtual wrappableU() const = 0;
  bool virtual wrappableV() const = 0;
  math137::Vector3f virtual uDerivative(float u, float v) const = 0;
  math137::Vector3f virtual vDerivative(float u, float v) const = 0;
  math137::Vector3f virtual getValue(float u, float v) const = 0;
  void setTrimmingTexture(const std::vector<uint8_t> &data, uint16_t x,
                          uint16_t y);
  bool isTrimmedUV(float u, float v) const;

protected:
  uint32_t m_trimmingTexture;
  const uint16_t m_width = 512;
  const uint16_t m_height = 512;
  std::vector<uint8_t> m_trimmingData;
};
