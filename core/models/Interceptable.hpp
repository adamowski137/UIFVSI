#include "Vector.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
class Interceptable {
public:
  math137::Vector3f virtual uDerivative(float u, float v) const = 0;
  math137::Vector3f virtual vDerivative(float u, float v) const = 0;
  math137::Vector3f virtual getValue(float u, float v) const = 0;

  static constexpr float tolerance = 1e-3f;
  static constexpr uint32_t maxIterations = 10000;

  static math137::Vector4f
  GradientDescent(const std::shared_ptr<Interceptable> &i1,
                  const std::shared_ptr<Interceptable> &i2,
                  math137::Vector4f x0 = {0, 0, 0, 0});

  static std::vector<math137::Vector3f>
  GenerateIntersectionPoints(const std::shared_ptr<Interceptable> &i1,
                             const std::shared_ptr<Interceptable> &i2,
                             math137::Vector4f start);

private:
  static float LineSearch(const std::shared_ptr<Interceptable> &i1,
                          const std::shared_ptr<Interceptable> &i2,
                          const math137::Vector4f &grad,
                          const math137::Vector4f &x0);
  static float DistanceSquared(const std::shared_ptr<Interceptable> &i1,
                               const std::shared_ptr<Interceptable> &i2,
                               const math137::Vector4f &params);
  static math137::Vector4f Gradient(const std::shared_ptr<Interceptable> &i1,
                                    const std::shared_ptr<Interceptable> &i2,
                                    const math137::Vector4f &params,
                                    float h = 0.0001f);
  static math137::Matrix4f Jacobian(const std::shared_ptr<Interceptable> &i1,
                                    const std::shared_ptr<Interceptable> &i2,
                                    const math137::Vector4f &params,
                                    const math137::Vector3f &d);
  static math137::Vector4f Value(const std::shared_ptr<Interceptable> &i1,
                                 const std::shared_ptr<Interceptable> &i2,
                                 const math137::Vector4f &old,
                                 const math137::Vector3f &prevPoint,
                                 const math137::Vector3f &tangent, float step);
  static std::optional<math137::Vector4f>
  Newton(const std::shared_ptr<Interceptable> &i1,
         const std::shared_ptr<Interceptable> &i2,
         const math137::Vector4f &initial, const math137::Vector3f &prevPoint,
         const math137::Vector3f &tangent, float step);
  static std::optional<math137::Vector4f>
  NextIntersectionPoint(const std::shared_ptr<Interceptable> &i1,
                        const std::shared_ptr<Interceptable> &i2,
                        const math137::Vector4f &prev, float step);
};
