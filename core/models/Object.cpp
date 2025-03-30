#include "Object.hpp"
#include "MatrixUtils.hpp"
#include "Quaternion.hpp"
#include "Vector.hpp"
#include "imgui.h"
#include <cstdint>
#include <string>

uint16_t Object::s_itemCount = 0;

Object::Object()
    : m_rotation(math137::Quaternion::Identity()),
      m_scale(math137::Vector3f(1.f, 1.f, 1.f)),
      m_model(math137::MatrixUtils::Identity()), m_update(false),
      m_id(s_itemCount++) {}

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

void Object::setNameMenu() {
  char buffer[256];
  strcpy(buffer, name.c_str());
  if (ImGui::InputText(("###ObjectNameMenu" + std::to_string(m_id)).c_str(),
                       buffer, sizeof(buffer),
                       ImGuiInputTextFlags_EnterReturnsTrue)) {
    name = std::string(buffer);
  }
}
