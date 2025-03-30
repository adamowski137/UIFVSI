#pragma once

#include "Matrix.hpp"
#include "Quaternion.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <string>
#include <vector>
class Point {
public:
  Point();
  void render() const;
  void setPosition(const math137::Vector3f &pos);
  void rotate(const math137::Quaternion &rot, const math137::Vector3f &pivot);
  void move(const math137::Vector3f &pos);
  math137::Matrix4f getModel() const;
  math137::Vector3f getPosition() const;

  std::string name;

private:
  std::vector<float> getSphereVertices();
  std::vector<uint16_t> getSphereIndices();

  math137::Matrix4f m_model;
  uint32_t m_vao;
  uint32_t m_vbo;
  uint32_t m_ebo;

  const uint16_t m_latSamples = 100;
  const uint16_t m_longSamples = 100;
  const float m_radius = 0.005f;

  static uint16_t s_count;
};
