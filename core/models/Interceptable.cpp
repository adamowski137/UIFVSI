#include "Interceptable.hpp"
#include "Matrix.hpp"
#include "MatrixUtils.hpp"
#include "Vector.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <vector>

float Interceptable::DistanceSquared(const std::shared_ptr<Interceptable> &i1,
                                     const std::shared_ptr<Interceptable> &i2,
                                     const ::math137::Vector4f &params) {
  math137::Vector3f pos1 = i1->getValue(params.x(), params.y());
  math137::Vector3f pos2 = i2->getValue(params.z(), params.w());

  return (pos1.x() - pos2.x()) * (pos1.x() - pos2.x()) +
         (pos1.y() - pos2.y()) * (pos1.y() - pos2.y()) +
         (pos1.z() - pos2.z()) * (pos1.z() - pos2.z());
}
math137::Vector4f
Interceptable::Gradient(const std::shared_ptr<Interceptable> &i1,
                        const std::shared_ptr<Interceptable> &i2,
                        const math137::Vector4f &params, float h) {
  math137::Vector3f diff = i1->getValue(params.x(), params.y()) -
                           i2->getValue(params.z(), params.w());
  math137::Vector3f df1du = i1->uDerivative(params.x(), params.y());
  math137::Vector3f df2du = i2->uDerivative(params.z(), params.w());
  math137::Vector3f df1dv = i1->vDerivative(params.x(), params.y());
  math137::Vector3f df2dv = i2->vDerivative(params.z(), params.w());
  return {diff * df1du * 2.f, diff * df1dv * 2.f, diff * df2du * -2.f,
          diff * df2dv * -2.f};
}
math137::Vector4f
Interceptable::GradientDescent(const std::shared_ptr<Interceptable> &i1,
                               const std::shared_ptr<Interceptable> &i2,
                               math137::Vector4f x0) {
  math137::Vector4f gradient = Gradient(i1, i2, x0);
  math137::Vector4f d = {-gradient.x(), -gradient.y(), -gradient.z(),
                         -gradient.w()};

  for (uint16_t i = 0; i < maxIterations; i++) {
    float alpha = LineSearch(i1, i2, d, x0);
    x0 = x0 + d * 1e-5;
    x0.wrap(1);
    math137::Vector4f newGradient = Gradient(i1, i2, x0);
    if (DistanceSquared(i1, i2, x0) < tolerance) {
      return x0;
    }

    float beta = (newGradient * newGradient) / (gradient * gradient);
    d = newGradient * -1 + d * beta;
    gradient = newGradient;
  }

  return x0;
}

float Interceptable::LineSearch(const std::shared_ptr<Interceptable> &i1,
                                const std::shared_ptr<Interceptable> &i2,
                                const math137::Vector4f &d,
                                const math137::Vector4f &x0) {
  float best = 0.0f;
  float minF = std::numeric_limits<float>::max();
  for (float a = 0.0f; a < 1.0f; a += 0.01f) {
    math137::Vector4f val = x0 + d * a;
    float f = DistanceSquared(i1, i2, val);
    if (f < minF) {
      minF = f;
      best = a;
    }
  }

  return best;
}

math137::Matrix4f
Interceptable::Jacobian(const std::shared_ptr<Interceptable> &i1,
                        const std::shared_ptr<Interceptable> &i2,
                        const math137::Vector4f &params,
                        const math137::Vector3f &d) {
  math137::Matrix4f res;
  math137::Vector3f df1du = i1->uDerivative(params.x(), params.y());
  math137::Vector3f df1dv = i1->vDerivative(params.x(), params.y());
  math137::Vector3f df2du = i2->uDerivative(params.z(), params.w());
  math137::Vector3f df2dv = i2->vDerivative(params.z(), params.w());

  res.setValue(0, 0, df1du.x());
  res.setValue(1, 0, df1du.y());
  res.setValue(2, 0, df1du.z());
  res.setValue(3, 0, d * df1du);

  res.setValue(0, 1, df1dv.x());
  res.setValue(1, 1, df1dv.y());
  res.setValue(2, 1, df1dv.z());
  res.setValue(3, 1, d * df1dv);

  res.setValue(0, 2, df2du.x() * -1.f);
  res.setValue(1, 2, df2du.y() * -1.f);
  res.setValue(2, 2, df2du.z() * -1.f);
  res.setValue(3, 2, 0);

  res.setValue(0, 3, df2dv.x() * -1.f);
  res.setValue(1, 3, df2dv.y() * -1.f);
  res.setValue(2, 3, df2dv.z() * -1.f);
  res.setValue(3, 3, 0);

  return res;
}

math137::Vector4f Interceptable::Value(const std::shared_ptr<Interceptable> &i1,
                                       const std::shared_ptr<Interceptable> &i2,
                                       const math137::Vector4f &old,
                                       const math137::Vector3f &prevPoint,
                                       const math137::Vector3f &tangent,
                                       float step) {
  math137::Vector3f p1 = i1->getValue(old.x(), old.y());
  math137::Vector3f p2 = i2->getValue(old.z(), old.w());
  math137::Vector3f diff = p1 - p2;
  float v = ((p1 - prevPoint) * tangent) - step;
  return {diff.x(), diff.y(), diff.z(), v};
}

std::optional<math137::Vector4f>
Interceptable::Newton(const std::shared_ptr<Interceptable> &i1,
                      const std::shared_ptr<Interceptable> &i2,
                      const math137::Vector4f &initial,
                      const math137::Vector3f &prevPoint,
                      const math137::Vector3f &tangent, float step) {
  math137::Vector4f newSol = initial;
  int i = 0;
  float eps = 1e-5;
  float length;
  do {
    if (i++ > 10) {
      std::cout << "maxIterations" << std::endl;
      return std::nullopt;
    }
    math137::Vector4f old = newSol;
    math137::Matrix4f jacobian = Jacobian(i1, i2, old, tangent);
    math137::Vector4f val = Value(i1, i2, old, prevPoint, tangent, step);
    std::optional<math137::Matrix4f> invJ =
        math137::MatrixUtils::Inverse(jacobian);
    if (!invJ.has_value()) {
      std::cout << "Inverse" << std::endl;
      return std::nullopt;
    }

    // newSol = old - val * invJ.value();
    newSol = old - invJ.value() * val;
    math137::Vector4f nValue = Value(i1, i2, newSol, prevPoint, tangent, step);
    length = nValue * nValue;
  } while (length > eps);

  return newSol;
}

std::optional<math137::Vector4f>
Interceptable::NextIntersectionPoint(const std::shared_ptr<Interceptable> &i1,
                                     const std::shared_ptr<Interceptable> &i2,
                                     const math137::Vector4f &prev,
                                     float step) {
  float remaining = step;
  const float minStep = step / 1024.f;
  std::optional<math137::Vector4f> newSol;
  do {
    math137::Vector3f df1du = i1->uDerivative(prev.x(), prev.y());
    math137::Vector3f df1dv = i1->vDerivative(prev.x(), prev.y());
    math137::Vector3f df2du = i2->uDerivative(prev.z(), prev.w());
    math137::Vector3f df2dv = i2->vDerivative(prev.z(), prev.w());
    math137::Vector3f n1 = math137::Vector3f::Cross(df1du, df1dv);
    math137::Vector3f n2 = math137::Vector3f::Cross(df2du, df2dv);
    math137::Vector3f tangent = math137::Vector3f::Cross(n1, n2);
    math137::Vector3f prevPoint = i1->getValue(prev.x(), prev.y());
    newSol = Newton(i1, i2, prev, prevPoint, tangent, step);
    if (!newSol.has_value()) {
      std::cout << "no value" << std::endl;
      step /= 2.f;
      if (step < minStep)
        return std::nullopt;
    } else {
      remaining -= step;
      step = remaining;
    }

  } while (remaining > 0);

  return newSol;
}
std::vector<math137::Vector3f> Interceptable::GenerateIntersectionPoints(
    const std::shared_ptr<Interceptable> &i1,
    const std::shared_ptr<Interceptable> &i2, math137::Vector4f start) {
  std::vector<math137::Vector3f> res = {};
  float step = 1e-3;
  auto startOpt = NextIntersectionPoint(i1, i2, start, step);
  if (!startOpt.has_value())
    return res;
  start = startOpt.value();
  int i = 0;
  math137::Vector3f startPos = i1->getValue(start.x(), start.y());
  res.push_back(i1->getValue(start.x(), start.y()));
  math137::Vector3f difference;
  float min = 1.f;
  do {
    if (i++ > 1000)
      break;
    startOpt = NextIntersectionPoint(i1, i2, start, step);
    if (!startOpt.has_value())
      break;
    start = startOpt.value();
    start.wrap(1.f);
    res.push_back(i1->getValue(start.x(), start.y()));
    difference = startPos - i1->getValue(start.x(), start.y());
    if (min > sqrtf(difference * difference))
      min = sqrtf(difference * difference);
    std::cout << i << ": " << sqrtf(difference * difference) << " " << min
              << std::endl;
  } while (sqrtf(difference * difference) > step / 2);
  return res;
}
