#pragma once

#include "../Object.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>
#include <vector>

class Torus : public Object {
public:
  Torus(float r, float R);
  ~Torus();
  inline float getSmallR() { return m_r; }
  inline float getBigR() { return m_R; }

  virtual bool renderObjectMenu() override;
  virtual void render(std::shared_ptr<Renderer> &renderer,
                      const math137::Vector4f &color) override;
  void setRadius(float r, float R);
  std::vector<float> getMesh();
  std::vector<uint32_t> getEdges();

  int32_t m_alphaSamples;
  int32_t m_betaSamples;

  static uint16_t s_count;

private:
  void setVertexData();
  float m_r;
  float m_R;
};
