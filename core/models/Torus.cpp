#include "Torus.hpp"
#include <GL/glew.h>
#include <cmath>
#include <cstdint>
#include <imgui.h>
#include <string>
#include <vector>

uint16_t Torus::s_count = 0;

Torus::Torus(float R, float r)
    : m_R{R}, m_r{r}, m_alphaSamples(10), m_betaSamples(30) {
  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ebo);
  name = "Torus " + std::to_string(s_count++);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
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
  points.reserve(3 * m_alphaSamples * m_betaSamples);
  for (uint16_t j = 0; j < m_betaSamples; j++) {
    float beta = j * ((2 * M_PI) / m_betaSamples);
    for (uint16_t i = 0; i < m_alphaSamples; i++) {
      float alpha = i * ((2 * M_PI) / m_alphaSamples);
      float x = (m_R + m_r * cosf(alpha)) * cosf(beta);
      float y = m_r * sinf(alpha);
      float z = (m_R + m_r * cosf(alpha)) * sinf(beta);
      points.push_back(x);
      points.push_back(y);
      points.push_back(z);
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

void Torus::render() const {
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glDrawElements(GL_LINES, m_alphaSamples * m_betaSamples * 4, GL_UNSIGNED_INT,
                 0);
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
void Torus::renderObjectMenu() {
  ImGui::Text("Object: %s", name.c_str());
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
                         0.1, 0.5)) {
    setVertexData();
  }
  ImGui::Text("Torus R:");
  if (ImGui::SliderFloat(("##torus R" + std::to_string(m_id)).c_str(), &m_R,
                         0.1, 0.5)) {
    setVertexData();
  }
  ImGui::Separator();
}
