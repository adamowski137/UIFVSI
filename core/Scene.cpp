#include "Scene.hpp"
#include "MatrixUtils.hpp"
#include "Vector.hpp"
#include "imgui.h"
#include "models/BezierCurve.hpp"
#include "models/Object.hpp"
#include "models/ObjectBuilder.hpp"
#include "models/Point.hpp"
#include "render/Renderer.hpp"
#include "render/Shader.hpp"
#include <memory>
#include <vector>

Scene::Scene() {}

void Scene::render(std::shared_ptr<Renderer> &renderer) {
  m_ground.render(renderer, m_centerColor);
  m_cursor.render(renderer, m_defaultColor);
  int idx = 1;
  for (const auto &obj : m_objects) {
    glStencilFunc(GL_ALWAYS, idx++, 0xff);
    obj->render(renderer, m_selectedObjects.find(obj) == m_selectedObjects.end()
                              ? m_defaultColor
                              : m_selectedColor);
  }
  m_massCenter.render(renderer, m_centerColor);
}

void Scene::renderMenu() {

  if (ImGui::BeginListBox("##Objects")) {
    for (const auto &obj : m_objects) {
      auto it = m_selectedObjects.find(obj);
      if (ImGui::Selectable(obj->name.c_str(), it != m_selectedObjects.end(),
                            ImGuiSelectableFlags_None)) {
        if (it == m_selectedObjects.end())
          m_selectedObjects.insert(obj);
        else
          m_selectedObjects.erase(obj);
        recalculateMassCenter();
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        obj->setMenu(true);
      }
      obj->renderObjectMenu();
    }
    ImGui::EndListBox();
    if (ImGui::Button("Add Torus")) {
      m_objects.push_back(ObjectBuilder()
                              .withNewTorus(0.5f, 0.1f)
                              .withPosition(m_cursor.getTranslation())
                              .build());
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Point")) {
      m_objects.push_back(ObjectBuilder()
                              .withNewPoint()
                              .withPosition(m_cursor.getTranslation())
                              .build());

      for (const auto &obj : m_selectedObjects) {
        std::shared_ptr<BezierCurve> l =
            std::dynamic_pointer_cast<BezierCurve>(obj);
        if (l)
          l->addPoint(std::dynamic_pointer_cast<Point>(
              m_objects[m_objects.size() - 1]));
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Bezier Curve")) {
      std::vector<std::shared_ptr<Point>> selectedPoints;
      for (const auto &obj : m_selectedObjects) {
        std::shared_ptr<Point> p = std::dynamic_pointer_cast<Point>(obj);
        if (p)
          selectedPoints.push_back(p);
      }
      m_objects.push_back(
          ObjectBuilder().withNewBezierCurve(selectedPoints).build());
      for (const auto &p : m_selectedObjects)
        m_sceneManager.addObserver(p, m_objects[m_objects.size() - 1]);
    }
    if (ImGui::Button("Delete Selected")) {
      deleteSelected();
    }
    ImGui::SameLine();
    if (ImGui::Button("Add To BezierCurve")) {
      addToBezierCurve();
    }
    ImGui::Text("Cursor Position");
    static float buf[3];
    buf[0] = m_cursor.getTranslation().x();
    buf[1] = m_cursor.getTranslation().y();
    buf[2] = m_cursor.getTranslation().z();
    if (ImGui::InputFloat3("###CursorPos", buf)) {
      m_cursor.setTranslation({buf[0], buf[1], buf[2]});
    }
  }
}

void Scene::recalculateMassCenter() {
  math137::Vector3f val;

  if (m_selectedObjects.size() == 0) {
    m_massCenter.setTranslation({0.f, 0.f, 0.f});
    return;
  }

  for (const auto ind : m_selectedObjects) {
    val = val + ind->getTranslation();
  }

  m_massCenter.setTranslation(val * (1.f / m_selectedObjects.size()));
}

void Scene::addObject(const std::shared_ptr<Object> &o) {
  m_objects.push_back(o);
  m_sceneManager.addObject(o);
}

void Scene::deleteSelected() {
  if (m_selectedObjects.size() != 0) {
    std::vector<std::shared_ptr<Object>> newObjects;
    newObjects.reserve(m_objects.size() - m_selectedObjects.size());
    for (const auto &obj : m_objects) {
      if (m_selectedObjects.find(obj) == m_selectedObjects.end())
        newObjects.push_back(obj);
      else
        m_sceneManager.deleteObject(obj);
    }
    m_selectedObjects.clear();
    m_objects = std::move(newObjects);
  }
}

void Scene::selectObjects(const std::set<uint8_t> &indices, bool add) {
  if (!add)
    m_selectedObjects.clear();
  for (const auto id : indices) {
    if (id == 0)
      continue;
    m_selectedObjects.insert(m_objects[id - 1]);
  }
}
std::vector<std::shared_ptr<Object>> Scene::getSelected() {
  std::vector<std::shared_ptr<Object>> result;
  result.reserve(m_selectedObjects.size());
  for (const auto idx : m_selectedObjects) {
    result.push_back(idx);
  }

  return result;
}

void Scene::addToBezierCurve() {
  std::vector<std::shared_ptr<Point>> selectedPoints;
  for (const auto &obj : m_selectedObjects) {
    std::shared_ptr<Point> p = std::dynamic_pointer_cast<Point>(obj);
    if (p)
      selectedPoints.push_back(p);
  }
  for (const auto &obj : m_selectedObjects) {
    std::shared_ptr<BezierCurve> l =
        std::dynamic_pointer_cast<BezierCurve>(obj);
    if (l) {
      for (const auto &p : selectedPoints) {
        l->addPoint(p);
        m_sceneManager.addObserver(p, obj);
      }
    }
  }
}

void Scene::notifySelected() {
  for (const auto &p : m_selectedObjects) {
    m_sceneManager.notify(p);
  }
}
