#include "IntersectionUtils.hpp"
#include "Intersectable.hpp"
#include "Matrix.hpp"
#include "MatrixUtils.hpp"
#include "Vector.hpp"
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <ostream>
#include <vector>

math137::Vector4f
IntersectionUtils::FreeStart(const std::shared_ptr<Intersectable> &i1,
                             const std::shared_ptr<Intersectable> &i2) {
  int divisons = 10;
  bool selfCross = i1 == i2;
  math137::Vector4f best;
  float minDist = std::numeric_limits<float>::max();
  for (uint16_t u = 1; u < divisons; u++) {
    for (uint16_t v = 1; v < divisons; v++) {
      for (uint16_t u1 = 1; u1 < divisons; u1++) {
        for (uint16_t v1 = 1; v1 < divisons; v1++) {
          math137::Vector4f x{u / (float)(divisons), v / (float)(divisons),
                              u1 / (float)divisons, v1 / (float)divisons};
          float val = DistanceSquared(i1, i2, x);
          if (selfCross) {
            math137::Vector4f grad = Gradient(i1, i2, x);
            float reverse =
                (i1->wrappableU() && fabsf(x.x() - x.z()) > 0.5f) ||
                        (i1->wrappableV() && fabsf(x.y() - x.w()) > 0.5f)
                    ? -1
                    : 1;
            math137::Vector2f v1 = math137::Vector2f(-grad.x(), -grad.y());
            math137::Vector2f v2 = math137::Vector2f(-grad.z(), -grad.w());
            math137::Vector2f v3 =
                math137::Vector2f(x.z() - x.x(), x.w() - x.y());
            math137::Vector2f v4 =
                math137::Vector2f(x.x() - x.y(), x.z() - x.w());

            if ((x.x() == x.z() && x.y() == x.w()) || v1 * v3 * reverse > 0 ||
                v2 * v4 * reverse > 0)
              continue;
          }
          if (val < minDist) {
            best = x;
            minDist = val;
          }
        }
      }
    }
  }

  return best;
}

math137::Vector4f
IntersectionUtils::StartFromCursor(const math137::Vector3f &pos,
                                   const std::shared_ptr<Intersectable> &i1,
                                   const std::shared_ptr<Intersectable> &i2) {
  int divisons = 10;
  bool selfCross = i1 == i2;
  math137::Vector4f best;
  float minDist = CursorDist(i1, i2, best, pos);
  for (uint16_t u = 1; u < divisons; u++) {
    for (uint16_t v = 1; v < divisons; v++) {
      math137::Vector4f x{u / (float)(divisons), v / (float)(divisons), 0, 0};
      float val = CursorDist(i1, i2, x, pos);
      if (val < minDist) {
        best = x;
        minDist = val;
      }
    }
  }

  if (selfCross && best.x() == best.w() && best.y() == best.z()) {
    best.z(1.f);
    minDist = CursorDist(i1, i2, best, pos);
  }

  for (uint16_t u = 1; u < divisons; u++) {
    for (uint16_t v = 1; v < divisons; v++) {
      math137::Vector4f x{best.x(), best.y(), u / (float)(divisons),
                          v / (float)(divisons)};
      if (selfCross) {
        math137::Vector4f grad = Gradient(i1, i2, x);
        float reverse =
            (i1->wrappableU() && fabsf(x.x() - x.z()) > 0.5f) ||
                    (i1->wrappableV() && fabsf(x.y() - x.w()) > 0.5f)
                ? -1.f
                : 1.f;
        math137::Vector2f v1 = math137::Vector2f(-grad.x(), -grad.y());
        math137::Vector2f v2 = math137::Vector2f(-grad.z(), -grad.w());
        math137::Vector2f v3 = math137::Vector2f(x.z() - x.x(), x.w() - x.y());
        math137::Vector2f v4 = math137::Vector2f(x.x() - x.y(), x.z() - x.w());

        if ((x.x() == x.z() && x.y() == x.w()) || v1 * v3 * reverse > 0 ||
            v2 * v4 * reverse > 0)
          continue;
      }
      float val = CursorDist(i1, i2, x, pos);
      if (val < minDist) {
        best = x;
        minDist = val;
      }
    }
  }

  return best;
}
float IntersectionUtils::CursorDist(const std::shared_ptr<Intersectable> &i1,
                                    const std::shared_ptr<Intersectable> &i2,
                                    const math137::Vector4f &params,
                                    const math137::Vector3f &cursor) {
  math137::Vector3f v1 = i1->getValue(params.x(), params.y()) - cursor;
  math137::Vector3f v2 = i2->getValue(params.z(), params.w()) - cursor;
  return v1 * v1 + v2 * v2;
}
math137::Vector4f
IntersectionUtils::CursorGrad(const std::shared_ptr<Intersectable> &i1,
                              const std::shared_ptr<Intersectable> &i2,
                              const math137::Vector4f &params,
                              const math137::Vector3f &cursor) {
  math137::Vector3f v1 = i1->getValue(params.x(), params.y()) - cursor;
  math137::Vector3f v2 = i2->getValue(params.z(), params.w()) - cursor;
  math137::Vector3f f1du = i1->uDerivative(params.x(), params.y());
  math137::Vector3f f1dv = i1->vDerivative(params.x(), params.y());
  math137::Vector3f f2du = i2->uDerivative(params.z(), params.w());
  math137::Vector3f f2dv = i2->vDerivative(params.z(), params.w());
  return {v1 * f1du * 2.f, v1 * f1dv * 2.f, v2 * f2du * 2.f, v2 * f2dv * 2.f};
}

float IntersectionUtils::DistanceSquared(
    const std::shared_ptr<Intersectable> &i1,
    const std::shared_ptr<Intersectable> &i2,
    const ::math137::Vector4f &params) {
  math137::Vector3f pos1 = i1->getValue(params.x(), params.y());
  math137::Vector3f pos2 = i2->getValue(params.z(), params.w());

  return (pos1.x() - pos2.x()) * (pos1.x() - pos2.x()) +
         (pos1.y() - pos2.y()) * (pos1.y() - pos2.y()) +
         (pos1.z() - pos2.z()) * (pos1.z() - pos2.z());
}
math137::Vector4f
IntersectionUtils::Gradient(const std::shared_ptr<Intersectable> &i1,
                            const std::shared_ptr<Intersectable> &i2,
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
IntersectionUtils::GradientDescent(const std::shared_ptr<Intersectable> &i1,
                                   const std::shared_ptr<Intersectable> &i2,
                                   math137::Vector4f x0) {
  math137::Vector4f gradient = Gradient(i1, i2, x0);
  math137::Vector4f d = {-gradient.x(), -gradient.y(), -gradient.z(),
                         -gradient.w()};
  std::array<bool, 4> wrappable = {i1->wrappableU(), i1->wrappableV(),
                                   i2->wrappableU(), i2->wrappableV()};

  for (uint16_t i = 0; i < maxIterations; i++) {
    float dist = DistanceSquared(i1, i2, x0);
    if (dist < tolerance) {
      return x0;
    }
    float alpha = LineSearch(i1, i2, d, x0);
    x0 = x0 + d * alpha;
    x0.wrap(1);

    math137::Vector4f newGradient = Gradient(i1, i2, x0);

    float beta = (newGradient * newGradient) / (gradient * gradient);
    d = newGradient * -1.f + d * beta;
    gradient = newGradient;
  }

  return x0;
}

float IntersectionUtils::AlphaRange(const std::shared_ptr<Intersectable> &i1,
                                    const std::shared_ptr<Intersectable> &i2,
                                    const math137::Vector4f &x,
                                    const math137::Vector4f &dir,
                                    const std::array<bool, 4> &wrap) {
  float maxA = MaxAlpha(x, dir, wrap);
  if (maxA == 0)
    return 0;
  float del = maxA / 100.f;
  float a2 = del;
  float f1 = DistanceSquared(i1, i2, x);
  float f2 = DistanceSquared(i1, i2, x + dir * a2);
  if (f2 > f1)
    return a2;
  while (true) {
    float a3 = a2 + del;
    if (a3 > maxA)
      return maxA;
    float f3 = DistanceSquared(i1, i2, x + dir * a3);
    if (f3 > f2)
      return a3;
    del *= 2.f;
  }
}

float IntersectionUtils::MaxAlpha(const math137::Vector4f &x,
                                  const math137::Vector4f &dir,
                                  const std::array<bool, 4> &wrap) {
  math137::Vector4f maxA1 = {
      (1.f - x.x()) / dir.x(),
      (1.f - x.y()) / dir.y(),
      (1.f - x.z()) / dir.z(),
      (1.f - x.w()) / dir.w(),
  };
  math137::Vector4f maxA2 = {
      -x.x() / dir.x(),
      -x.y() / dir.y(),
      -x.z() / dir.z(),
      -x.w() / dir.w(),
  };
  std::array<float, 8> max = {
      maxA1.x(), maxA1.y(), maxA1.z(), maxA1.w(),
      maxA2.x(), maxA2.y(), maxA2.z(), maxA2.w(),
  };

  float maxAlpha = 1.f;
  for (uint16_t i = 0; i < 8; i++) {
    if (max[i] >= 0 && maxAlpha > max[i] && !wrap[i % 4])
      maxAlpha = max[i];
  }
  return maxAlpha;
}

float IntersectionUtils::LineSearch(const std::shared_ptr<Intersectable> &i1,
                                    const std::shared_ptr<Intersectable> &i2,
                                    const math137::Vector4f &d,
                                    const math137::Vector4f &x0) {
  float best = 0.0f;
  std::array<bool, 4> wrappable = {i1->wrappableU(), i1->wrappableV(),
                                   i2->wrappableU(), i2->wrappableV()};
  float minF = std::numeric_limits<float>::max();
  float r = AlphaRange(i1, i2, x0, d, wrappable);
  float l = 0.f;
  float length = 1;
  while (length > 1e-5) {
    math137::Vector4f change = d * (r - l);
    length = change * change;
    float m = (l + r) / 2.f;
    float da = Gradient(i1, i2, x0 + d * m) * d;
    if (fabsf(da) < 1e-5)
      return m;
    if (da > 0)
      r = m;
    else
      l = m;
  }
  return (r + l) / 2;
}

math137::Matrix4f
IntersectionUtils::Jacobian(const std::shared_ptr<Intersectable> &i1,
                            const std::shared_ptr<Intersectable> &i2,
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

math137::Vector4f
IntersectionUtils::Value(const std::shared_ptr<Intersectable> &i1,
                         const std::shared_ptr<Intersectable> &i2,
                         const math137::Vector4f &old,
                         const math137::Vector3f &prevPoint,
                         const math137::Vector3f &tangent, float step) {
  math137::Vector3f p1 = i1->getValue(old.x(), old.y());
  math137::Vector3f p2 = i2->getValue(old.z(), old.w());
  math137::Vector3f diff = p1 - p2;
  float v = ((p1 - prevPoint) * tangent) - step;
  return {diff.x(), diff.y(), diff.z(), v};
}

std::optional<math137::Vector4f>
IntersectionUtils::Newton(const std::shared_ptr<Intersectable> &i1,
                          const std::shared_ptr<Intersectable> &i2,
                          const math137::Vector4f &initial,
                          const math137::Vector3f &prevPoint,
                          const math137::Vector3f &tangent, float step) {
  math137::Vector4f newSol = initial;
  std::array<bool, 4> wrappable = {i1->wrappableU(), i1->wrappableV(),
                                   i2->wrappableU(), i2->wrappableV()};
  int i = 0;
  float eps = 1e-5;
  float length;
  math137::Vector4f nValue = {0, 0, 0, -step};

  do {
    if (i++ > 100) {
      break;
    }
    math137::Vector4f old = newSol;
    math137::Matrix4f jacobian = Jacobian(i1, i2, old, tangent);
    auto dx = math137::MatrixUtils::SolveLinearSystem(jacobian, nValue);
    if (!dx.has_value()) {
      std::cout << "Inverse" << std::endl;
      return std::nullopt;
    }
    newSol = old - dx.value();

    if (!CheckWrap(newSol, wrappable)) {
      std::cout << "Out of bounds" << std::endl;
      return std::nullopt;
    }
    newSol.wrap(1.f);
    nValue = Value(i1, i2, newSol, prevPoint, tangent, step);
    length = nValue * nValue;
  } while (nValue.any([eps](float v) { return fabsf(v) > eps; }));
  //} while (length > eps);

  return newSol;
}

std::optional<math137::Vector4f> IntersectionUtils::NextIntersectionPoint(
    const std::shared_ptr<Intersectable> &i1,
    const std::shared_ptr<Intersectable> &i2, const math137::Vector4f &prev,
    float step, bool dir) {
  std::optional<math137::Vector4f> newSol;
  math137::Vector3f df1du = i1->uDerivative(prev.x(), prev.y());
  math137::Vector3f df1dv = i1->vDerivative(prev.x(), prev.y());
  math137::Vector3f df2du = i2->uDerivative(prev.z(), prev.w());
  math137::Vector3f df2dv = i2->vDerivative(prev.z(), prev.w());
  math137::Vector3f n1 = math137::Vector3f::Cross(df1du, df1dv);
  math137::Vector3f n2 = math137::Vector3f::Cross(df2du, df2dv);
  n1.normalize();
  n2.normalize();

  if ((n1 - n2) * (n1 - n2) < 1e-6) {
    std::cout << "Newton normal vectors: " << (n1 - n2) * (n1 - n2)
              << std::endl;
    return std::nullopt;
  }

  math137::Vector3f tangent =
      math137::Vector3f::Cross(n1, n2) * (dir ? 1.f : -1.f);
  tangent.normalize();
  math137::Vector3f prevPoint = i1->getValue(prev.x(), prev.y());
  newSol = Newton(i1, i2, prev, prevPoint, tangent, step);

  return newSol;
}
bool IntersectionUtils::CheckWrap(const math137::Vector4f &val,
                                  const std::array<bool, 4> &wrap) {
  for (uint16_t i = 0; i < 4; i++) {
    if ((val[i] < 0 || val[i] > 1) && !wrap[i])
      return false;
  }
  return true;
}
std::pair<bool, std::vector<math137::Vector4f>>
IntersectionUtils::GenerateIntersectionPoints(
    const std::shared_ptr<Intersectable> &i1,
    const std::shared_ptr<Intersectable> &i2, math137::Vector4f start,
    float step, bool dir) {
  std::vector<math137::Vector4f> res = {};
  std::array<bool, 4> wrappable = {i1->wrappableU(), i1->wrappableV(),
                                   i2->wrappableU(), i2->wrappableV()};
  auto startOpt = NextIntersectionPoint(i1, i2, start, step, dir);
  if (!startOpt.has_value()) {
    std::cout << "no value outer" << std::endl;
    return {false, res};
  }
  start = startOpt.value();
  int i = 0;
  math137::Vector3f startPos = i1->getValue(start.x(), start.y());
  res.push_back(start);
  math137::Vector3f difference;

  do {
    if (i++ > 10000)
      break;
    startOpt = NextIntersectionPoint(i1, i2, start, step, dir);
    if (!startOpt.has_value()) {
      std::cout << "no value outer" << std::endl;
      return {false, res};
    }
    start = startOpt.value();
    if (!CheckWrap(start, wrappable)) {
      std::cout << "Out of bounds outer" << std::endl;
      return {false, res};
    }

    start.wrap(1.f);
    res.push_back(start);
    difference = startPos - i1->getValue(start.x(), start.y());
    if (i > 5 && difference * difference < step * step)
      return {true, res};
  } while (true);
  //} while (sqrtf(difference * difference) > step / 2);
  return {false, res};
}
