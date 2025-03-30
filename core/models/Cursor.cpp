#include "Cursor.hpp"
#include <GL/glew.h>
#include <MatrixUtils.hpp>

Cursor::Cursor() : m_model(math137::MatrixUtils::Identity()) {

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ebo);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  float vert[] = {0.0f, 0.0f,  0.0f, 0.03f, 0.0f, 0.0f,
                  0.0f, 0.03f, 0.0f, 0.0f,  0.0f, 0.03f};
  uint16_t ind[] = {0, 1, 0, 2, 0, 3};

  glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ind), ind, GL_STATIC_DRAW);
}
void Cursor::render() const {
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glDrawElements(GL_LINES, 6, GL_UNSIGNED_SHORT, 0);
}
void Cursor::setPosition(const math137::Vector3f &pos) {
  m_model.setValue(0, 3, pos.x());
  m_model.setValue(1, 3, pos.y());
  m_model.setValue(2, 3, pos.z());
}
void Cursor::move(const math137::Vector3f &pos) {
  m_model.setValue(0, 3, m_model.getValue(0, 3) + pos.x());
  m_model.setValue(1, 3, m_model.getValue(1, 3) + pos.y());
  m_model.setValue(2, 3, m_model.getValue(2, 3) + pos.z());
}
math137::Vector3f Cursor::getPosition() const {
  return math137::Vector3f(m_model.getValue(0, 3), m_model.getValue(1, 3),
                           m_model.getValue(2, 3));
}
