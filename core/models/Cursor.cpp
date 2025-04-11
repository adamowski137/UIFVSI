#include "Cursor.hpp"
#include "Object.hpp"
#include "Vector.hpp"
#include <GL/glew.h>
#include <MatrixUtils.hpp>
#include <iostream>
#include <memory>

Cursor::Cursor() : Object(ShaderType::OBJECT) {

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ebo);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  float vert[] = {0.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f,
                  0.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.1f};
  uint16_t ind[] = {0, 1, 0, 2, 0, 3};

  glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ind), ind, GL_STATIC_DRAW);
}
void Cursor::render(std::shared_ptr<Renderer> &renderer,
                    const math137::Vector4f &color) {
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(color);
  glDrawElements(GL_LINES, 6, GL_UNSIGNED_SHORT, 0);
}
void Cursor::recalculateModel() {
  m_model = math137::MatrixUtils::Translate(
      m_translation.x(), m_translation.y(), m_translation.z());
  m_update = false;
}
