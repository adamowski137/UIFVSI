#include "Object.hpp"
#include "MatrixUtils.hpp"
#include "Quaternion.hpp"
#include "imgui.h"
#include <GL/glew.h>
#include <cstdint>
#include <string>

uint16_t Object::s_itemCount = 0;

Object::Object(ShaderType type)
    : m_rotation(math137::Quaternion::Identity()),
      m_scale(math137::Vector3f(1.f, 1.f, 1.f)),
      m_translation(math137::Vector3f(0.f, 0.f, 0.f)),
      m_model(math137::MatrixUtils::Identity()), m_update(false), m_type(type),
      m_id(s_itemCount++), m_openMenu(false) {
  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ebo);
}

Object::~Object() {
  glDeleteVertexArrays(1, &m_vao);
  glDeleteBuffers(1, &m_vbo);
  glDeleteBuffers(1, &m_ebo);
}

math137::Matrix4f Object::getModel() {
  if (m_update)
    recalculateModel();

  return m_model;
}

void Object::recalculateModel() {
  m_model = math137::MatrixUtils::Translate(
                m_translation.x(), m_translation.y(), m_translation.z()) *
            math137::MatrixUtils::FromQuaternion(m_rotation) *
            math137::MatrixUtils::Scale(m_scale.x(), m_scale.y(), m_scale.z());
  m_update = false;
}

bool Object::setNameMenu() {
  char buffer[256];
  bool change = false;
  strcpy(buffer, name.c_str());
  if (ImGui::InputText(("###ObjectNameMenu" + std::to_string(m_id)).c_str(),
                       buffer, sizeof(buffer),
                       ImGuiInputTextFlags_EnterReturnsTrue)) {
    name = std::string(buffer);
  }
  float data[3] = {m_translation.x(), m_translation.y(), m_translation.z()};
  if (ImGui::InputFloat3(("###Position" + name).c_str(), data)) {
    setTranslation({data[0], data[1], data[2]});
    change = true;
  }

  return change;
}
void Object::rotate(const math137::Quaternion &rot,
                    const math137::Vector3f &pivot) {
  math137::Vector3f translatedPos = m_translation - pivot;
  math137::Quaternion rotatedPos =
      rot * math137::Quaternion::FromVector(translatedPos) * rot.conjugate();
  m_rotation = rot * m_rotation;
  m_translation =
      math137::Vector3f(rotatedPos.b, rotatedPos.c, rotatedPos.d) + pivot;
  m_update = true;
}
void Object::scale(float s, const math137::Vector3f &pivot) {
  math137::Vector3f axis = m_translation - pivot;
  m_translation = pivot + (axis * s);
  m_scale = m_scale * s;
  m_update = true;
}
