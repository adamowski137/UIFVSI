#pragma once

#include "Matrix.hpp"
#include "Quaternion.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <string>

class Object {
public:
  Object();
  virtual ~Object() {}

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
  void rotate(const math137::Quaternion &rot, const math137::Vector3f &pivot) {
    math137::Vector3f translatedPos = m_translation - pivot;
    math137::Quaternion rotatedPos =
        rot * math137::Quaternion::FromVector(translatedPos) * rot.conjugate();
    m_rotation = rot * m_rotation;
    m_translation =
        math137::Vector3f(rotatedPos.b, rotatedPos.c, rotatedPos.d) + pivot;
    m_update = true;
  }
  inline void scale(float s, const math137::Vector3f &pivot) {
    if (fabsf((m_translation - pivot) * (m_translation - pivot)) < 1e-6) {
      m_scale = m_scale * s;
      m_update = true;
      return;
    }
    math137::Vector3f axis = m_translation - pivot;
    axis.normalize();
    math137::Vector3f parallel = axis * (m_scale * axis);
    math137::Vector3f perp = m_scale - parallel;
    m_scale = (perp + parallel * s);
    m_update = true;
  }
  math137::Matrix4f getModel();
  inline math137::Quaternion getRotation() { return m_rotation; }
  inline math137::Vector3f getTranslation() { return m_translation; }
  inline math137::Vector3f getScale() { return m_scale; }

  void recalculateModel();
  void setNameMenu();
  virtual void renderObjectMenu() = 0;
  virtual void render() const = 0;

public:
  std::string name;

protected:
  uint16_t m_id;

  uint32_t m_vao;
  uint32_t m_vbo;
  uint32_t m_ebo;

private:
  math137::Vector3f m_translation;
  math137::Quaternion m_rotation;
  math137::Vector3f m_scale;
  math137::Matrix4f m_model;
  bool m_update;

  static uint16_t s_itemCount;
};
