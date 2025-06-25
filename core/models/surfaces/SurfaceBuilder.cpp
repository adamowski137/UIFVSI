#include "SurfaceBuilder.hpp"
#include "../ObjectBuilder.hpp"
#include "Vector.hpp"
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

std::vector<std::vector<std::shared_ptr<Object>>>
SurfaceBuilder::GetPointGrid(uint16_t uPatches, uint16_t vPatches,
                             const math137::Vector3f &pos, float w, float h,
                             bool c0, bool cylinder) {
  return c0 ? (cylinder ? NewPointCylinderC0(uPatches, vPatches, pos, w, h)
                        : NewPointGridC0(uPatches, vPatches, pos, w, h))
            : (cylinder ? NewPointCylinderC2(uPatches, vPatches, pos, w, h)
                        : NewPointGridC2(uPatches, vPatches, pos, w, h));
}

std::vector<std::vector<std::shared_ptr<Object>>>
SurfaceBuilder::NewPointGridC0(uint16_t uPatches, uint16_t vPatches,
                               const math137::Vector3f &pos, float w, float h) {
  uint16_t uPoints = (4 + (uPatches - 1) * 3);
  uint16_t vPoints = (4 + (vPatches - 1) * 3);
  uint16_t totalPoints = uPoints * vPoints;

  std::vector<std::vector<std::shared_ptr<Object>>> points(
      uPoints, std::vector<std::shared_ptr<Object>>(vPoints));
  float dx = w / uPoints;
  float dy = h / vPoints;

  ObjectBuilder ob;

  for (uint16_t u = 0; u < uPoints; u++) {
    for (uint16_t v = 0; v < vPoints; v++) {
      points[u][v] =
          (ob.withNewPoint()
               .withPosition(pos + math137::Vector3f(dx * u, 0, dy * v))
               .build());
    }
  }

  return points;
}

std::vector<std::vector<std::shared_ptr<Object>>>
SurfaceBuilder::NewPointCylinderC0(uint16_t uPatches, uint16_t vPatches,
                                   const math137::Vector3f &pos, float r,
                                   float h) {

  uint16_t uPoints = (4 + (uPatches - 1) * 3);
  uint16_t vPoints = (4 + (vPatches - 1) * 3);
  uint16_t totalPoints = uPoints * vPoints;

  std::vector<std::vector<std::shared_ptr<Object>>> points(
      uPoints, std::vector<std::shared_ptr<Object>>(vPoints));

  float da = 2 * M_PI / (uPoints - 1);
  float dz = h / vPoints;

  ObjectBuilder ob;

  for (uint16_t u = 0; u < uPoints - 1; u++) {
    for (uint16_t v = 0; v < vPoints; v++) {
      points[u][v] =
          (ob.withNewPoint()
               .withPosition(pos + math137::Vector3f(r * cosf(da * u),
                                                     r * sinf(da * u), dz * v))
               .build());
    }
  }
  for (uint16_t v = 0; v < vPoints; v++)
    points[uPoints - 1][v] = points[0][v];

  return points;
}

std::vector<std::vector<std::shared_ptr<Object>>>
SurfaceBuilder::NewPointGridC2(uint16_t uPatches, uint16_t vPatches,
                               const math137::Vector3f &pos, float w, float h) {
  uint16_t uPoints = 4 + (uPatches - 1);
  uint16_t vPoints = 4 + (vPatches - 1);
  uint16_t totalPoints = uPoints * vPoints;

  std::vector<std::vector<std::shared_ptr<Object>>> points(
      uPoints, std::vector<std::shared_ptr<Object>>(vPoints));

  float dx = w / uPoints;
  float dy = h / vPoints;

  ObjectBuilder ob;

  for (uint16_t u = 0; u < uPoints; u++) {
    for (uint16_t v = 0; v < vPoints; v++) {
      points[u][v] =
          (ob.withNewPoint()
               .withPosition(pos + math137::Vector3f(dx * u, 0, dy * v))
               .build());
    }
  }

  return points;
}

std::vector<std::vector<std::shared_ptr<Object>>>
SurfaceBuilder::NewPointCylinderC2(uint16_t uPatches, uint16_t vPatches,
                                   const math137::Vector3f &pos, float r,
                                   float h) {

  uint16_t uPoints = 4 + uPatches - 1;
  uint16_t vPoints = 4 + vPatches - 1;
  uint16_t totalPoints = uPoints * vPoints;

  std::vector<std::vector<std::shared_ptr<Object>>> points(
      uPoints, std::vector<std::shared_ptr<Object>>(vPoints));

  float da = 2 * M_PI / (uPoints - 3);
  float dz = h / vPoints;

  ObjectBuilder ob;

  for (uint16_t u = 0; u < uPoints - 3; u++) {
    for (uint16_t v = 0; v < vPoints; v++) {
      points[u][v] =
          (ob.withNewPoint()
               .withPosition(pos + math137::Vector3f(r * cosf(da * u),
                                                     r * sinf(da * u), dz * v))
               .build());
    }
  }
  for (uint16_t u = 0; u < 3; u++)
    for (uint16_t v = 0; v < vPoints; v++)
      points[uPoints - 3 + u][v] = (points[u][v]);

  return points;
}
