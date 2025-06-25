#pragma once
#include "Intersectable.hpp"
#include "Vector.hpp"
#include <array>
#include <cstdint>
#include <memory>
#include <optional>

class IntersectionUtils {
public:
  static constexpr float tolerance = 1e-5f;
  static constexpr uint32_t maxIterations = 10000;

  static math137::Vector4f
  GradientDescent(const std::shared_ptr<Intersectable> &i1,
                  const std::shared_ptr<Intersectable> &i2,
                  math137::Vector4f x0 = {0, 0, 0, 0});
  static math137::Vector4f
  StartFromCursor(const math137::Vector3f &pos,
                  const std::shared_ptr<Intersectable> &i1,
                  const std::shared_ptr<Intersectable> &i2);
  static math137::Vector4f FreeStart(const std::shared_ptr<Intersectable> &i1,
                                     const std::shared_ptr<Intersectable> &i2);
  static std::pair<bool, std::vector<math137::Vector4f>>
  GenerateIntersectionPoints(const std::shared_ptr<Intersectable> &i1,
                             const std::shared_ptr<Intersectable> &i2,
                             math137::Vector4f start, float step, bool dir);

private:
  static float CursorDist(const std::shared_ptr<Intersectable> &i1,
                          const std::shared_ptr<Intersectable> &i2,
                          const math137::Vector4f &params,
                          const math137::Vector3f &cursor);
  static math137::Vector4f CursorGrad(const std::shared_ptr<Intersectable> &i1,
                                      const std::shared_ptr<Intersectable> &i2,
                                      const math137::Vector4f &params,
                                      const math137::Vector3f &cursor);
  static float MaxAlpha(const math137::Vector4f &x,
                        const math137::Vector4f &dir,
                        const std::array<bool, 4> &wrap);
  static float AlphaRange(const std::shared_ptr<Intersectable> &i1,
                          const std::shared_ptr<Intersectable> &i2,
                          const math137::Vector4f &x,
                          const math137::Vector4f &dir,
                          const std::array<bool, 4> &wrap);
  static float LineSearch(const std::shared_ptr<Intersectable> &i1,
                          const std::shared_ptr<Intersectable> &i2,
                          const math137::Vector4f &grad,
                          const math137::Vector4f &x0);
  static float DistanceSquared(const std::shared_ptr<Intersectable> &i1,
                               const std::shared_ptr<Intersectable> &i2,
                               const math137::Vector4f &params);
  static math137::Vector4f Gradient(const std::shared_ptr<Intersectable> &i1,
                                    const std::shared_ptr<Intersectable> &i2,
                                    const math137::Vector4f &params,
                                    float h = 0.0001f);
  static math137::Matrix4f Jacobian(const std::shared_ptr<Intersectable> &i1,
                                    const std::shared_ptr<Intersectable> &i2,
                                    const math137::Vector4f &params,
                                    const math137::Vector3f &d);
  static math137::Vector4f Value(const std::shared_ptr<Intersectable> &i1,
                                 const std::shared_ptr<Intersectable> &i2,
                                 const math137::Vector4f &old,
                                 const math137::Vector3f &prevPoint,
                                 const math137::Vector3f &tangent, float step);
  static std::optional<math137::Vector4f>
  Newton(const std::shared_ptr<Intersectable> &i1,
         const std::shared_ptr<Intersectable> &i2,
         const math137::Vector4f &initial, const math137::Vector3f &prevPoint,
         const math137::Vector3f &tangent, float step);
  static std::optional<math137::Vector4f>
  NextIntersectionPoint(const std::shared_ptr<Intersectable> &i1,
                        const std::shared_ptr<Intersectable> &i2,
                        const math137::Vector4f &prev, float step, bool dir);
  static bool CheckWrap(const math137::Vector4f &val,
                        const std::array<bool, 4> &wrap);
};
