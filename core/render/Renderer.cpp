#include "Renderer.hpp"
#include "Matrix.hpp"
#include "Shader.hpp"
#include <cstdint>

Renderer::Renderer()
    : m_objectShader("../shaders/base.vs", "../shaders/base.fs"),
      m_pointShader("../shaders/point.vs", "../shaders/point.fs"),
      m_curveShader("../shaders/bernstein.vs", "../shaders/bernstein.fs",
                    "../shaders/bernstein.tes", "../shaders/bernstein.eval") {
  m_selectedShader = &m_objectShader;
  m_selectedShader->use();
}

void Renderer::setProjection(const math137::Matrix4f &projection) {
  m_objectShader.use();
  m_objectShader.setMat4("projection", projection);
  m_pointShader.use();
  m_pointShader.setMat4("projection", projection);
  m_curveShader.use();
  m_curveShader.setMat4("projection", projection);
  m_selectedShader->use();
}

void Renderer::setView(const math137::Matrix4f &view) {
  m_objectShader.use();
  m_objectShader.setMat4("view", view);
  m_pointShader.use();
  m_pointShader.setMat4("view", view);
  m_curveShader.use();
  m_curveShader.setMat4("view", view);
  m_selectedShader->use();
}

void Renderer::setShader(const ShaderType type) {
  if (m_type == type)
    return;
  m_type = type;
  switch (type) {
  case ShaderType::POINT:
    m_selectedShader = &m_pointShader;
    break;
  case ShaderType::OBJECT:
    m_selectedShader = &m_objectShader;
    break;
  case ShaderType::CURVE:
    m_selectedShader = &m_curveShader;
    break;
  }
  m_selectedShader->use();
}

void Renderer::setDegree(uint8_t degree) {
  m_selectedShader->setInt("count", degree);
}

void Renderer::setColor(const math137::Vector4f &color) {
  m_selectedShader->setVec4("color", color);
}

void Renderer::setModel(const math137::Matrix4f &model) {
  m_selectedShader->setMat4("model", model);
}
