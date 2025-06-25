#include "Renderer.hpp"
#include "Matrix.hpp"
#include "Shader.hpp"
#include <cstdint>

Renderer::Renderer()
    : m_objectShader("../shaders/base.vs", "../shaders/base.fs"),
      m_pointShader("../shaders/point.vs", "../shaders/base.fs"),
      m_curveShader("../shaders/bernstein.vs", "../shaders/base.fs",
                    "../shaders/bernstein.tes", "../shaders/bernstein.eval"),
      m_surfaceC2Shader("../shaders/surface.vs", "../shaders/base.fs",
                        "../shaders/surface.tes", "../shaders/surfaceC2.eval"),
      m_gregoryShader("../shaders/gregory.vs", "../shaders/base.fs",
                      "../shaders/gregory.tes", "../shaders/gregory.eval"),
      m_surfaceShader("../shaders/surface.vs", "../shaders/base.fs",
                      "../shaders/surface.tes", "../shaders/surface.eval") {
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
  m_surfaceShader.use();
  m_surfaceShader.setMat4("projection", projection);
  m_surfaceC2Shader.use();
  m_surfaceC2Shader.setMat4("projection", projection);
  m_gregoryShader.use();
  m_gregoryShader.setMat4("projection", projection);
  m_selectedShader->use();
}

void Renderer::setView(const math137::Matrix4f &view) {
  m_objectShader.use();
  m_objectShader.setMat4("view", view);
  m_pointShader.use();
  m_pointShader.setMat4("view", view);
  m_curveShader.use();
  m_curveShader.setMat4("view", view);
  m_surfaceShader.use();
  m_surfaceShader.setMat4("view", view);
  m_surfaceC2Shader.use();
  m_surfaceC2Shader.setMat4("view", view);
  m_gregoryShader.use();
  m_gregoryShader.setMat4("view", view);
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
  case ShaderType::SURFACEC2:
    m_selectedShader = &m_surfaceC2Shader;
    break;
  case ShaderType::SURFACE:
    m_selectedShader = &m_surfaceShader;
    break;
  case ShaderType::GREGORY:
    m_selectedShader = &m_gregoryShader;
  }

  m_selectedShader->use();
}

void Renderer::setDegree(uint8_t degree) {
  m_selectedShader->setInt("count", degree);
}

void Renderer::setColor(const math137::Vector4f &color) {
  m_selectedShader->setVec4("color", color);
}
void Renderer::setColor(unsigned int c) {
  float r = (c & 0xFF) / 255.f;
  float g = ((c >> 8) & 0xFF) / 255.f;
  float b = ((c >> 16) & 0xFF) / 255.f;
  float a = ((c >> 24) & 0xFF) / 255.f;

  m_selectedShader->setVec4("color", {r, g, b, a});
}

void Renderer::setModel(const math137::Matrix4f &model) {
  m_selectedShader->setMat4("model", model);
}
void Renderer::setUVSubdivisions(uint16_t u, uint16_t v) {
  m_selectedShader->setUInt("u_subdivisions", u + 1);
  m_selectedShader->setUInt("v_subdivisions", v + 1);
}
void Renderer::setUVpatches(uint16_t u, uint16_t v) {
  m_selectedShader->setUInt("uPatches", u);
  m_selectedShader->setUInt("vPatches", v);
}
void Renderer::reverseUV(bool r) { m_selectedShader->setBool("reverse", r); }
