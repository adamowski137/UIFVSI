#pragma once

#include "../render/Renderer.hpp"
#include "../render/Shader.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>
#include <string>

class Object {
public:
  Object(ShaderType type);
  virtual ~Object();

  math137::Matrix4f getModel();
  inline math137::Quaternion getRotation() { return m_rotation; }
  inline math137::Vector3f getTranslation() { return m_translation; }
  inline math137::Vector3f getScale() { return m_scale; }
  inline void setTranslation(const math137::Vector3f &pos) {
    m_translation = pos;
    m_update = true;
  }
  inline void setRotation(const math137::Quaternion &rot) {
    m_rotation = rot;
    m_update = true;
  }
  inline void setScale(const math137::Vector3f &scale) {
    m_scale = scale;
    m_update = true;
  }
  inline void move(const math137::Vector3f &tr) {
    m_translation = m_translation + tr;
    m_update = true;
  }
  inline void rotate(const math137::Quaternion &rot) {
    m_rotation = m_rotation * rot;
    m_update = true;
  }
  inline void setMenu(bool open) { m_openMenu = open; }
  void rotate(const math137::Quaternion &rot, const math137::Vector3f &pivot);
  void scale(float s, const math137::Vector3f &pivot);
  void setNameMenu();

  virtual void renderObjectMenu() = 0;
  virtual void render(std::shared_ptr<Renderer> &renderer,
                      const math137::Vector4f &color) = 0;
  virtual void notify() {}

  std::string name;

  bool operator==(const Object &o) { return m_id == o.m_id; }

protected:
  virtual void recalculateModel();

  uint16_t m_id;
  uint32_t m_vao;
  uint32_t m_vbo;
  uint32_t m_ebo;

  math137::Vector3f m_translation;
  math137::Quaternion m_rotation;
  math137::Vector3f m_scale;
  math137::Matrix4f m_model;

  bool m_update;
  bool m_openMenu;
  ShaderType m_type;

private:
  static uint16_t s_itemCount;
};
