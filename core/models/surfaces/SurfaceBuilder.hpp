#pragma once

#include "Surface.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

class SurfaceBuilder {
public:
  static std::vector<std::vector<std::shared_ptr<Object>>>
  GetPointGrid(uint16_t uPatches, uint16_t vPatches,
               const math137::Vector3f &pos, float w, float h, bool c0,
               bool cylinder);

private:
  static std::vector<std::vector<std::shared_ptr<Object>>>
  NewPointGridC0(uint16_t uPatches, uint16_t vPatches,
                 const math137::Vector3f &pos, float w, float h);
  static std::vector<std::vector<std::shared_ptr<Object>>>
  NewPointCylinderC0(uint16_t uPatches, uint16_t vPatches,
                     const math137::Vector3f &pos, float r, float h);
  static std::vector<std::vector<std::shared_ptr<Object>>>
  NewPointGridC2(uint16_t uPatches, uint16_t vPatches,
                 const math137::Vector3f &pos, float w, float h);
  static std::vector<std::vector<std::shared_ptr<Object>>>
  NewPointCylinderC2(uint16_t uPatches, uint16_t vPatches,
                     const math137::Vector3f &pos, float r, float h);
};
