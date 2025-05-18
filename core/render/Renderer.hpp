#pragma once

#include "Shader.hpp"
#include "Vector.hpp"
#include <cstdint>

class Object;

class Renderer {
public:
  Renderer();
  void setProjection(const math137::Matrix4f &projection);
  void setView(const math137::Matrix4f &view);
  void setModel(const math137::Matrix4f &model);
  void setShader(const ShaderType type);
  void setColor(const math137::Vector4f &color);
  void setDegree(uint8_t degree);
  void setUVSubdivisions(uint16_t u, uint16_t v);

private:
  Shader *m_selectedShader;
  Shader m_objectShader;
  Shader m_pointShader;
  Shader m_curveShader;
  Shader m_surfaceShader;
  ShaderType m_type;
};
