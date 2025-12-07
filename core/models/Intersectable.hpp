#pragma once
#include "Vector.hpp"
#include <cstdint>
#include <vector>
class Intersectable
{
public:
  Intersectable();
  virtual ~Intersectable();
  bool virtual wrappableU() const = 0;
  bool virtual wrappableV() const = 0;
  math137::Vector3f virtual uDerivative(float u, float v) const = 0;
  math137::Vector3f virtual vDerivative(float u, float v) const = 0;
  math137::Vector3f virtual getValue(float u, float v) const = 0;
  virtual void intersectTrimmingTexture(uint16_t x,
                                        uint16_t y);
  virtual void unionTrimmingTexture(uint16_t x,
                                    uint16_t y);
  virtual void resetTrimming();
  virtual bool isTrimmedUV(float u, float v) const;
  std::vector<math137::Vector3f> extractContour(int sx, int sy) const;
  std::vector<std::vector<math137::Vector3f>> extractPaths(int step, bool dir) const;
  std::vector<math137::Vector3f> gridMillingPath(float stepU, float stepV) const;

  std::vector<uint8_t> m_textureData;
  std::vector<uint8_t> m_trimmingData;
  static constexpr uint16_t m_width = 512;
  static constexpr uint16_t m_height = 512;

protected:
  std::pair<int, int> findFirstContourPoint(int startX, int startY) const;
  std::vector<math137::Vector3f> extractPath(int x, int y, std::vector<uint8_t>& temp, int step, bool checkYFirst) const;

  uint32_t m_trimmingTexture;
};
