#include "Scene.hpp"
#include "Matrix.hpp"
#include "MatrixUtils.hpp"
#include "SceneManager.hpp"
#include "State.hpp"
#include "imgui.h"
#include "models/ObjectBuilder.hpp"
#include "models/surfaces/BezierC0.hpp"
#include "models/surfaces/BezierC2.hpp"
#include "render/Renderer.hpp"
#include "render/Shader.hpp"
#include "serialize/Serializer.hpp"
#include <ImGuiFileDialog.h>
#include <cmath>
#include <memory>
#include <string>

class SceneManager;

Scene::Scene() {}

void Scene::render(std::shared_ptr<Renderer> &renderer,
                   std::unique_ptr<SceneManager> &manager, const State &state) {
  if (state.getDisplayMode() == DisplayMode::DEFAULT) {
    // glDisable(GL_BLEND);
    renderer->setProjection(state.getProjection());
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
  if (state.getDisplayMode() == DisplayMode::STEREO) {

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
    renderer->setProjection(state.getLeftProjection());
    m_ground.render(renderer, m_leftColor);
    auto objects = manager->getDrawableObjects();
    for (int i = 0; i < objects.size(); i++) {
      if (objects[i].expired())
        continue;
      objects[i].lock()->render(renderer, m_leftColor);
    }
    renderer->setProjection(state.getRightProjection());
    glClear(GL_DEPTH_BUFFER_BIT);
    glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
    m_ground.render(renderer, m_rightColor);
    for (int i = 0; i < objects.size(); i++) {
      if (objects[i].expired())
        continue;
      objects[i].lock()->render(renderer, m_rightColor);
    }
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  }
}

void Scene::renderMenu(std::unique_ptr<SceneManager> &manager, State &state) {
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
  if (ImGui::Button("Collapse")) {
    manager->collapseSelected();
  }
  if (ImGui::Button("Add Gregory")) {
    manager->addGregoryPatch();
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
  if (ImGui::Button("Export")) {
    IGFD::FileDialogConfig config;
    config.path = ".";
    ImGuiFileDialog::Instance()->OpenDialog("ExportFile", "Export File",
                                            ".json", config);
  }
  if (ImGui::Button("Import")) {
    IGFD::FileDialogConfig config;
    config.path = ".";
    ImGuiFileDialog::Instance()->OpenDialog("ImportFile", "Import File",
                                            ".json", config);
  }
  static bool stereoPopup = false;
  if (ImGui::Button("Stereo Settings")) {
    stereoPopup = true;
  }
  static bool stereo = false;
  if (ImGui::Checkbox("Stereo", &stereo)) {
    if (stereo)
      state.setDisplayMode(DisplayMode::STEREO);
    else
      state.setDisplayMode(DisplayMode::DEFAULT);
  }

  if (ImGuiFileDialog::Instance()->Display("ExportFile")) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
      Serializer::SerializeToFile(filePath, manager);
    }

    ImGuiFileDialog::Instance()->Close();
  }

  if (ImGuiFileDialog::Instance()->Display("ImportFile")) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
      Serializer::DeserializeFromFile(filePath, manager);
    }

    ImGuiFileDialog::Instance()->Close();
  }

  if (openC0Popup) {
    getSurfaceMenu<BezierC0>(openC0Popup, manager);
  }
  if (openC2Popup) {
    getSurfaceMenu<BezierC2>(openC2Popup, manager);
  }
  if (stereoPopup) {
    static float d = 0, p = 0.2f;
    bool change = false;
    ImGui::Begin("Stereo");
    if (ImGui::SliderFloat("IOD", &d, 0, 1))
      change = true;
    if (ImGui::SliderFloat("Convergence", &p, 0.1f, 10.f))
      change = true;
    ImGui::End();
    if (change) {
      float aspect = (float)state.getWidth() / state.getHeight();
      float xOffset = d / 2.f;
      float top = state.m_near * tanf(state.m_fov / 2);
      float bottom = -top;

      float ll = -aspect * top + xOffset / p * state.m_near;
      float lr = aspect * top + xOffset / p * state.m_near;
      math137::Matrix4f m1 =
          math137::MatrixUtils::MovedProjection(ll, lr, top, bottom,
                                                state.m_near, state.m_far) *
          math137::MatrixUtils::Translate(xOffset, 0, 0);
      xOffset = -d / 2.f;
      float rl = -aspect * top + xOffset / p * state.m_near;
      float rr = aspect * top + xOffset / p * state.m_near;
      math137::Matrix4f m2 =
          math137::MatrixUtils::MovedProjection(rl, rr, top, bottom,
                                                state.m_near, state.m_far) *
          math137::MatrixUtils::Translate(xOffset, 0, 0);
      state.setMovedProjection(m1, m2);
    }
  }
}
