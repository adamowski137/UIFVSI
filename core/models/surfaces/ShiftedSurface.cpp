#include "ShiftedSurface.hpp"
#include <cmath>

ShiftedSurface::ShiftedSurface(std::shared_ptr<BezierC2> base, float radius)
    : m_base(base), m_radius(radius) {}

bool ShiftedSurface::wrappableU() const
{
  if (m_base)
    return m_base->wrappableU();
  return false;
}

bool ShiftedSurface::wrappableV() const
{
  if (m_base)
    return m_base->wrappableV();
  return false;
}

math137::Vector3f ShiftedSurface::uDerivative(float u, float v) const
{
  if (!m_base)
    return math137::Vector3f(0, 0, 0);
  // base first derivatives
  math137::Vector3f du = m_base->uDerivative(u, v);
  math137::Vector3f dv = m_base->vDerivative(u, v);

  if(du * du < 1e-8f || dv * dv < 1e-8f)
    return {0.f, 0.f, 0.f}; // avoid division by zero for degenerate cases

  // second derivatives
  math137::Vector3f duu = m_base->uuDerivative(u, v);
  math137::Vector3f duv = m_base->uvDerivative(u, v);
  math137::Vector3f dvv = m_base->vvDerivative(u, v);

  // unnormalized normal
  math137::Vector3f an = math137::Vector3f::Cross(du, dv);
  float norm = std::sqrt(an * an);

  // derivatives of unnormalized normal
  // d/du (du x dv) = duu x dv + du x duv
  math137::Vector3f an_u = math137::Vector3f::Cross(duu, dv) + math137::Vector3f::Cross(du, duv);
  // derivative of normalized normal: (an_u / norm) - an * (an . an_u) / norm^3
  float norm3 = norm * norm * norm;
  math137::Vector3f dn_du = (an_u / norm) - (an * ((an * an_u) / norm3));

  // shifted derivative = base derivative + radius * dn/du
  return du + dn_du * m_radius;
}

math137::Vector3f ShiftedSurface::vDerivative(float u, float v) const
{
  if (!m_base)
    return math137::Vector3f(0, 0, 0);
  // base first derivatives
  math137::Vector3f du = m_base->uDerivative(u, v);
  math137::Vector3f dv = m_base->vDerivative(u, v);

  // second derivatives
  math137::Vector3f duu = m_base->uuDerivative(u, v);
  math137::Vector3f duv = m_base->uvDerivative(u, v);
  math137::Vector3f dvv = m_base->vvDerivative(u, v);

  // unnormalized normal
  math137::Vector3f an = math137::Vector3f::Cross(du, dv);
  float norm = std::sqrt(an * an);
  // d/dv (du x dv) = duv x dv + du x dvv
  math137::Vector3f an_v = math137::Vector3f::Cross(duv, dv) + math137::Vector3f::Cross(du, dvv);
  float norm3 = norm * norm * norm;
  math137::Vector3f dn_dv = (an_v / norm) - (an * ((an * an_v) / norm3));

  // shifted derivative = base derivative + radius * dn/dv
  return dv + dn_dv * m_radius;
}

math137::Vector3f ShiftedSurface::getValue(float u, float v) const
{
  if (!m_base)
    return math137::Vector3f(0, 0, 0);

  math137::Vector3f p = m_base->getValue(u, v);
  math137::Vector3f du = m_base->uDerivative(u, v);
  math137::Vector3f dv = m_base->vDerivative(u, v);

  math137::Vector3f an = math137::Vector3f::Cross(du, dv);
  float norm = std::sqrt(an * an);
  math137::Vector3f n = an / norm;
  return p + n * m_radius;
}

void ShiftedSurface::intersectTrimmingTexture(uint16_t x, uint16_t y)
{
  if (m_base)
  {
    m_base->intersectTrimmingTexture(x, y);
  }
  Intersectable::intersectTrimmingTexture(x, y);
}

void ShiftedSurface::unionTrimmingTexture(uint16_t x, uint16_t y)
{
  if (m_base)
  {
    m_base->unionTrimmingTexture(x, y);
  }
  Intersectable::unionTrimmingTexture(x, y);
}

void ShiftedSurface::resetTrimming()
{
  if (m_base)
  {
    m_base->resetTrimming();
  }
  Intersectable::resetTrimming();
}

bool ShiftedSurface::isTrimmedUV(float u, float v) const
{
  return Intersectable::isTrimmedUV(u, v);
}
