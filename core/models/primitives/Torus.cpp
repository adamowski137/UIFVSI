#include "Torus.hpp"
#include "../Object.hpp"
#include "MatrixUtils.hpp"
#include "Vector.hpp"
#include <GL/gl.h>
#include <GL/glew.h>
#include <cmath>
#include <cstdint>
#include <imgui.h>
#include <string>
#include <vector>

Torus::Torus(float R, float r)
    : Object(ShaderType::OBJECT), m_R{R}, m_r{r}, m_alphaSamples(10),
      m_betaSamples(30) {
  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ebo);
  name = "Torus " + std::to_string(m_id);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

  setVertexData();
}

Torus::~Torus() {}

void Torus::setRadius(float r, float R) {
  m_r = r;
  m_R = R;
}

std::vector<float> Torus::getMesh() {
  std::vector<float> points;
  points.reserve(5 * m_alphaSamples * m_betaSamples);
  for (uint16_t j = 0; j < m_betaSamples; j++) {
    float v = j / (float)(m_betaSamples - 1);
    for (uint16_t i = 0; i < m_alphaSamples; i++) {
      float u = i / (float)(m_alphaSamples - 1);
      float mu = 2.f * M_PI * u;
      float mv = 2.f * M_PI * v;
      float x = (m_R + m_r * cosf(mu)) * cosf(mv);
      float y = m_r * sinf(mu);
      float z = (m_R + m_r * cosf(mu)) * sinf(mv);
      points.push_back(x);
      points.push_back(y);
      points.push_back(z);
      points.push_back(v);
      points.push_back(u);
    }
  }

  return points;
}

std::vector<uint32_t> Torus::getEdges() {
  std::vector<uint32_t> result;
  result.reserve(m_alphaSamples * m_betaSamples * 4);
  for (uint16_t i = 0; i < m_betaSamples; i++) {
    for (uint16_t j = 0; j < m_alphaSamples; j++) {
      uint32_t index = i * m_alphaSamples + j;
      uint32_t closeNeighbour = i * m_alphaSamples + ((j + 1) % m_alphaSamples);
      uint32_t farNeighbour = ((i + 1) % m_betaSamples) * m_alphaSamples + j;

      result.push_back(index);
      result.push_back(closeNeighbour);
      result.push_back(index);
      result.push_back(farNeighbour);
    }
  }

  return result;
}

void Torus::renderFramebuffer(std::shared_ptr<Renderer> &renderer,
                              unsigned int c) {
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(c);
  glDrawElements(GL_LINES, m_alphaSamples * m_betaSamples * 4, GL_UNSIGNED_INT,
                 0);
}

void Torus::render(std::shared_ptr<Renderer> &renderer,
                   const math137::Vector4f &color) {
  glBindVertexArray(m_vao);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_trimmingTexture);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(color);
  glDrawElements(GL_LINES, m_alphaSamples * m_betaSamples * 4, GL_UNSIGNED_INT,
                 0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

math137::Vector3f Torus::getValue(float u, float v) const {
  float mu = 2.f * M_PI * u;
  float mv = 2.f * M_PI * v;
  float x = (m_R + m_r * cosf(mu)) * cosf(mv);
  float y = m_r * sinf(mu);
  float z = (m_R + m_r * cosf(mu)) * sinf(mv);
  math137::Vector4f res = m_model * math137::Vector4f(x, y, z, 1.f);

  return math137::Vector3f(res.x(), res.y(), res.z());
}

math137::Vector3f Torus::uDerivative(float u, float v) const {
  float m = 2.f * M_PI;
  float mu = m * u;
  float mv = m * v;
  math137::Vector4f local{-m_r * sinf(mu) * cosf(mv) * m, m_r * cosf(mu) * m,
                          -m_r * sinf(mu) * sinf(mv) * m, 0};
  math137::Vector4f res =
      math137::MatrixUtils::FromQuaternion(m_rotation) *
      math137::MatrixUtils::Scale(m_scale.x(), m_scale.y(), m_scale.z()) *
      local;
  return {res.x(), res.y(), res.z()};
}

math137::Vector3f Torus::vDerivative(float u, float v) const {
  float m = 2.f * M_PI;
  float mu = m * u;
  float mv = m * v;
  math137::Vector4f local{-(m_R + m_r * cosf(mu)) * sinf(mv) * m, 0,
                          (m_R + m_r * cosf(mu)) * cosf(mv) * m, 0};
  math137::Vector4f res =
      math137::MatrixUtils::FromQuaternion(m_rotation) *
      math137::MatrixUtils::Scale(m_scale.x(), m_scale.y(), m_scale.z()) *
      local;
  return {res.x(), res.y(), res.z()};
}

void Torus::setVertexData() {
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  std::vector<float> vertices = getMesh();
  std::vector<uint32_t> indices = getEdges();
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
               indices.data(), GL_STATIC_DRAW);
}
bool Torus::renderObjectMenu() {
  if (!m_openMenu)
    return false;
  ImGui::Begin("Settings", &m_openMenu);
  setNameMenu();
  ImGui::Text("Alpha samples");
  if (ImGui::SliderInt(("##alpha" + std::to_string(m_id)).c_str(),
                       &m_alphaSamples, 3, 100)) {
    setVertexData();
  }
  ImGui::Text("Beta samples");
  if (ImGui::SliderInt(("##beta" + std::to_string(m_id)).c_str(),
                       &m_betaSamples, 3, 100)) {
    setVertexData();
  }
  ImGui::Separator();
  ImGui::Text("Torus r:");
  if (ImGui::SliderFloat(("##torus r" + std::to_string(m_id)).c_str(), &m_r,
                         0.1, 5)) {
    setVertexData();
  }
  ImGui::Text("Torus R:");
  if (ImGui::SliderFloat(("##torus R" + std::to_string(m_id)).c_str(), &m_R,
                         0.1, 5)) {
    setVertexData();
  }
  ImGui::End();
  return false;
}
