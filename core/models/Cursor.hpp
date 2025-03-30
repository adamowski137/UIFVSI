#pragma once

#include "Matrix.hpp"
#include "Vector.hpp"

class Cursor {
public:
  Cursor();
  void render() const;
  void setPosition(const math137::Vector3f &pos);
  void move(const math137::Vector3f &pos);
  math137::Vector3f getPosition() const;

  inline math137::Matrix4f getModel() const { return m_model; }

private:
  math137::Matrix4f m_model;
  uint32_t m_vao;
  uint32_t m_vbo;
  uint32_t m_ebo;
};
