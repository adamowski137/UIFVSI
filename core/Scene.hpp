#pragma once

#include "Matrix.hpp"
#include "SceneManager.hpp"
#include "Vector.hpp"
#include "models/Cursor.hpp"
#include "models/Ground.hpp"
#include "models/Object.hpp"
#include "models/Point.hpp"
#include "render/Renderer.hpp"
#include <memory>
#include <set>
#include <vector>

class Scene {
public:
  Scene();

  void addObject(const std::shared_ptr<Object> &o);
  void deleteSelected();
  void render(std::shared_ptr<Renderer> &renderer);
  void renderMenu();
  void recalculateMassCenter();
  std::vector<std::shared_ptr<Object>> getSelected();
  void selectObjects(const std::set<uint8_t> &indices, bool add);
  void notifySelected();

  inline void setInvProjection(const math137::Matrix4f &m) {
    m_invprojection = m;
  }
  inline void setCursorPosition(const math137::Vector3f &pos) {
    m_cursor.setTranslation(pos);
  }
  inline math137::Vector3f getCursorPosition() {
    return m_cursor.getTranslation();
  }
  inline math137::Matrix4f getInvProjection() { return m_invprojection; }
  inline math137::Vector3f getMassCenter() {
    return m_massCenter.getTranslation();
  }

private:
  void addToBezierCurve();

  SceneManager m_sceneManager;
  std::vector<std::shared_ptr<Object>> m_objects;
  std::set<std::shared_ptr<Object>> m_selectedObjects;
  Point m_massCenter;
  Ground m_ground;
  Cursor m_cursor;
  math137::Matrix4f m_invprojection;
  const math137::Vector4f m_defaultColor =
      math137::Vector4f(1.f, 1.f, 0.f, 1.f);
  const math137::Vector4f m_selectedColor =
      math137::Vector4f(1.f, 0.2f, 0.2f, 1.f);
  const math137::Vector4f m_centerColor = math137::Vector4f(1.f, 1.f, 1.f, 1.f);
};
