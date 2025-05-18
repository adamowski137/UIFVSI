#include "SurfaceBuilder.hpp"
#include "../ObjectBuilder.hpp"
#include "Vector.hpp"
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

std::vector<std::shared_ptr<Object>>
SurfaceBuilder::NewPointGrid(uint16_t uPatches, uint16_t vPatches,
                             const math137::Vector3f &pos, float w, float h) {
  uint16_t uPoints = (4 + (uPatches - 1) * 3);
  uint16_t vPoints = (4 + (vPatches - 1) * 3);
  uint16_t totalPoints = uPoints * vPoints;

  std::vector<std::shared_ptr<Object>> points;
  points.reserve(totalPoints);
  float dx = w / uPoints;
  float dy = h / vPoints;

  ObjectBuilder ob;

  for (uint16_t u = 0; u < uPoints; u++) {
    for (uint16_t v = 0; v < vPoints; v++) {
      points.push_back(
          ob.withNewPoint()
              .withPosition(pos + math137::Vector3f(dx * u, 0, dy * v))
              .build());
    }
  }

  return points;
}

std::vector<std::shared_ptr<Object>>
SurfaceBuilder::NewPointCylinder(uint16_t uPatches, uint16_t vPatches,
                                 const math137::Vector3f &pos, float r,
                                 float h) {

  uint16_t uPoints = (4 + (uPatches - 1) * 3) - 1;
  uint16_t vPoints = (4 + (vPatches - 1) * 3);
  uint16_t totalPoints = uPoints * vPoints;

  std::vector<std::shared_ptr<Object>> points;
  points.reserve(totalPoints);

  float da = 2 * M_PI / uPoints;
  float dz = h / vPoints;

  ObjectBuilder ob;

  for (uint16_t u = 0; u < uPoints; u++) {
    for (uint16_t v = 0; v < vPoints; v++) {
      points.push_back(
          ob.withNewPoint()
              .withPosition(pos + math137::Vector3f(r * cosf(da * u),
                                                    r * sinf(da * u), dz * v))
              .build());
    }
  }

  return points;
}
