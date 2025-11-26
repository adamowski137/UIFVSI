#pragma once

#include "../Intersectable.hpp"

class FlatSurface : public Intersectable
{
public:
    FlatSurface(float width, float depth, const math137::Vector3f &position)
        : m_width(width), m_depth(depth), m_position(position) {};
    virtual ~FlatSurface() = default;

    bool virtual wrappableU() const override { return false; };
    bool virtual wrappableV() const override { return false; };
    math137::Vector3f virtual uDerivative(float u, float v) const override;
    math137::Vector3f virtual vDerivative(float u, float v) const override;
    math137::Vector3f virtual getValue(float u, float v) const override;
    
private:
    float m_width;
    float m_depth;
    math137::Vector3f m_position;
};