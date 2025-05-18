#pragma once

#include "Surface.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

class SurfaceBuilder {
public:
  template <typename T>
  static std::pair<std::shared_ptr<Surface>,
                   std::vector<std::shared_ptr<Object>>>
  NewPatch(uint16_t uPatches, uint16_t vPatches, const math137::Vector3f &pos,
           bool cylinder, float p1, float p2) {
    std::vector<std::shared_ptr<Object>> points =
        !cylinder ? NewPointGrid(uPatches, vPatches, pos, p1, p2)
                  : NewPointCylinder(uPatches, vPatches, pos, p1, p2);
    std::shared_ptr<T> bezier =
        std::make_shared<T>(points, uPatches, vPatches, cylinder);

    return {bezier, points};
  }

private:
  static std::vector<std::shared_ptr<Object>>
  NewPointGrid(uint16_t uPatches, uint16_t vPatches,
               const math137::Vector3f &pos, float w, float h);
  static std::vector<std::shared_ptr<Object>>
  NewPointCylinder(uint16_t uPatches, uint16_t vPatches,
                   const math137::Vector3f &pos, float r, float h);
};
