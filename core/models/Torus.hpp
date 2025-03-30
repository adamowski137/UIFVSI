#pragma once

#include "Object.hpp"
#include <cstdint>
#include <vector>
class Torus : public Object {
public:
  Torus(float r, float R);
  ~Torus();
  inline float getSmallR() { return m_r; }
  inline float getBigR() { return m_R; }

  virtual void renderObjectMenu() override;
  virtual void render() const override;
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
