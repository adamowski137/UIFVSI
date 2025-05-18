#include "Scene.hpp"
#include "SceneManager.hpp"
#include "imgui.h"
#include "models/ObjectBuilder.hpp"
#include "models/surfaces/BezierC0.hpp"
#include "models/surfaces/BezierC2.hpp"
#include "render/Renderer.hpp"
#include "render/Shader.hpp"
#include <memory>

class SceneManager;

Scene::Scene() {}

void Scene::render(std::shared_ptr<Renderer> &renderer,
                   std::unique_ptr<SceneManager> &manager) {
  m_ground.render(renderer, m_centerColor);
  int idx = 1;
  auto objects = manager->getDrawableObjects();
  for (int i = 0; i < objects.size(); i++) {
    if (objects[i].expired())
      continue;

    glStencilFunc(GL_ALWAYS, i + 1, 0xff);
    objects[i].lock()->render(
        renderer, manager->isSelected(objects[i])  ? m_selectedColor
                  : manager->isVirtual(objects[i]) ? m_centerColor
                                                   : m_defaultColor);
  }
  manager->m_massCenter.render(renderer, m_centerColor);
  manager->m_cursor.render(renderer, m_defaultColor);
}

void Scene::renderMenu(std::unique_ptr<SceneManager> &manager) {
  if (ImGui::BeginListBox("##Objects")) {
    for (const auto &obj : manager->getObjects()) {
      bool isSelected = manager->isSelected(obj);
      if (ImGui::Selectable(obj->name.c_str(), isSelected,
                            ImGuiSelectableFlags_None)) {
        manager->toggleSelection(obj);
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        obj->setMenu(true);
      }
      if (obj->renderObjectMenu()) {
        manager->notify(obj);
      }
    }
    ImGui::EndListBox();
  }
  if (ImGui::Button("Add Torus")) {
    manager->addObject(ObjectBuilder()
                           .withNewTorus(0.5f, 0.1f)
                           .withPosition(manager->getCursorPosition())
                           .build());
  }
  ImGui::SameLine();
  if (ImGui::Button("Add Point")) {
    manager->addPoint();
  }
  ImGui::SameLine();
  if (ImGui::Button("Add Bezier Curve")) {
    manager->addBezierCurve();
  }
  if (ImGui::Button("Add BSpline")) {
    manager->addBspline();
  }
  ImGui::SameLine();
  if (ImGui::Button("Add IBSpline")) {
    manager->addIBspline();
  }
  static bool openC0Popup = false;
  static bool openC2Popup = false;
  if (ImGui::Button("Add C0 surface")) {
    openC0Popup = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("Add C2 surface")) {
    openC2Popup = true;
  }
  if (ImGui::Button("Delete Selected")) {
    manager->deleteSelected();
  }
  ImGui::Text("Cursor Position");
  static float buf[3];
  buf[0] = manager->getCursorPosition().x();
  buf[1] = manager->getCursorPosition().y();
  buf[2] = manager->getCursorPosition().z();
  if (ImGui::InputFloat3("###CursorPos", buf)) {
    manager->setCursorPosition({buf[0], buf[1], buf[2]});
  }

  if (openC0Popup) {
    getSurfaceMenu<BezierC0>(openC0Popup, manager);
  }
  if (openC2Popup) {
    getSurfaceMenu<BezierC2>(openC2Popup, manager);
  }
}
