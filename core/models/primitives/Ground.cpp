#include "Ground.hpp"
#include "../Object.hpp"
#include "Vector.hpp"
#include <GL/glew.h>
#include <memory>
#include <vector>

Ground::Ground() : Object(ShaderType::OBJECT) {
  std::vector<float> vertices;

  for (float i = -m_gridSize; i <= m_gridSize; i += m_gapSize) {
    vertices.push_back(i);
    vertices.push_back(0.0f);
    vertices.push_back(-m_gridSize);
    vertices.push_back(i);
    vertices.push_back(0.0f);
    vertices.push_back(m_gridSize);

    vertices.push_back(-m_gridSize);
    vertices.push_back(0.0f);
    vertices.push_back(i);
    vertices.push_back(m_gridSize);
    vertices.push_back(0.0f);
    vertices.push_back(i);
  }

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

  glBindVertexArray(0);
}

void Ground::render(std::shared_ptr<Renderer> &renderer,
                    const math137::Vector4f &color) {
  glBindVertexArray(m_vao);
  renderer->setShader(m_type);
  renderer->setModel(m_model);
  renderer->setColor(color);
  glDrawArrays(GL_LINES, 0, (m_gridSize * 4 / m_gapSize + 1) * 4);
  glBindVertexArray(0);
}
