#pragma once

#include "../Object.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>
#include <vector>

class Point : public Object {
public:
  Point();
  ~Point() {}

  virtual bool renderObjectMenu() override;
  virtual void render(std::shared_ptr<Renderer> &renderer,
                      const math137::Vector4f &color) override;

  virtual void renderFramebuffer(std::shared_ptr<Renderer> &renderer,
                                 unsigned int id) override;

protected:
  void recalculateModel() override;

private:
  std::vector<float> getSphereVertices();
  std::vector<uint16_t> getSphereIndices();

  const uint16_t m_latSamples = 100;
  const uint16_t m_longSamples = 100;
  const float m_radius = 0.005f;
};
