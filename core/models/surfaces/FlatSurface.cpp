#include "FlatSurface.hpp"

math137::Vector3f FlatSurface::uDerivative(float u, float v) const {
    return math137::Vector3f(m_width, 0.0f, 0.0f);
}
math137::Vector3f FlatSurface::vDerivative(float u, float v) const {
    return math137::Vector3f(0.0f, 0.0f, m_depth);
}

math137::Vector3f FlatSurface::getValue(float u, float v) const {
    float x = m_position.x() + u * m_width;
    float y = m_position.y();
    float z = m_position.z() + v * m_depth;
    return math137::Vector3f(x, y, z);
}
