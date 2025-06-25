#pragma once

#include "../Intersectable.hpp"
#include "../Object.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>
#include <vector>

class Torus : public Object, public Intersectable {
public:
  Torus(float R, float r);
  ~Torus();
  inline float getSmallR() { return m_r; }
  inline float getBigR() { return m_R; }

  virtual bool renderObjectMenu() override;
  virtual void render(std::shared_ptr<Renderer> &renderer,
                      const math137::Vector4f &color) override;
  virtual void renderFramebuffer(std::shared_ptr<Renderer> &renderer,
                                 unsigned int id) override;
  virtual math137::Vector3f uDerivative(float u, float v) const override;
  virtual math137::Vector3f vDerivative(float u, float v) const override;
  virtual math137::Vector3f getValue(float u, float v) const override;
  virtual bool wrappableU() const override { return true; }
  virtual bool wrappableV() const override { return true; }
  void setRadius(float r, float R);
  std::vector<float> getMesh();
  std::vector<uint32_t> getEdges();

  int32_t m_alphaSamples;
  int32_t m_betaSamples;

private:
  friend class Serializer;
  void setVertexData();
  float m_r;
  float m_R;
};
