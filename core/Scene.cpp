#include "Scene.hpp"
#include "Matrix.hpp"
#include "MatrixUtils.hpp"
#include "SceneManager.hpp"
#include "State.hpp"
#include "Vector.hpp"
#include "imgui.h"
#include "models/Object.hpp"
#include "models/ObjectBuilder.hpp"
#include "models/surfaces/BezierC0.hpp"
#include "models/surfaces/BezierC2.hpp"
#include "render/Renderer.hpp"
#include "render/Shader.hpp"
#include "serialize/Serializer.hpp"
#include <ImGuiFileDialog.h>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "paths/PathGenerator.hpp"
#include "serialize/Parser.hpp"
class SceneManager;

Scene::Scene() {}
void Scene::renderToFramebuffer(std::shared_ptr<Renderer> &renderer,
                                std::unique_ptr<SceneManager> &manager,
                                const Camera &camera, const State &state) {

  auto objects = manager->getDrawableObjects();
  glBindFramebuffer(GL_FRAMEBUFFER, state.fbo);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  renderer->setProjection(state.getProjection());
  renderer->setView(camera.getView());
  for (uint32_t i = 0; i < objects.size(); i++) {
    if (objects[i].expired())
      continue;
    uint32_t index = i + 1;
    objects[i].lock()->renderFramebuffer(renderer, index);
  }
}

void Scene::render(std::shared_ptr<Renderer> &renderer,
                   std::unique_ptr<SceneManager> &manager, const State &state) {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (state.getDisplayMode() == DisplayMode::DEFAULT) {
    renderer->setProjection(state.getProjection());
    m_ground.render(renderer, m_centerColor);
    auto objects = manager->getDrawableObjects();
    for (uint32_t i = 0; i < objects.size(); i++) {
      if (objects[i].expired())
        continue;
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
  if (ImGui::BeginListBox("##Objects", ImVec2(300, 500))) {
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
  static bool openIntersection = false;
  if (ImGui::Button("Add C0 surface")) {
    openC0Popup = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("Add C2 surface")) {
    openC2Popup = true;
  }
  if (ImGui::Button("Intersect")) {
    openIntersection = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("To IBSpline")) {
    manager->toIBSpline();
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
  ImGui::SameLine();
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
  static PathGenerator pathGen;
  if(ImGui::Button("Create Milling Surface")) {
    pathGen.createMillingSurface(manager, 4.f);
  }
  if(ImGui::Button("Generate Rough Path")) {
    auto path = pathGen.generatePath(manager);
    std::ofstream file("C:/Users/adam/Desktop/projekty/mill-simulator/paths/7.k16");
    Parser::parseMovesToFile(path, file);
    file.close();
  }
  if(ImGui::Button("Generate Ball Path")) {
    auto path = pathGen.generateBallPath(manager);
    std::ofstream file("C:/Users/adam/Desktop/projekty/mill-simulator/paths/8.k08");
    Parser::parseMovesToFile(path, file);
    file.close();
  }
    if(ImGui::Button("Generate Flat Path")) {
    auto path = pathGen.generateFlatPath(manager);
    std::ofstream file("C:/Users/adam/Desktop/projekty/mill-simulator/paths/9.f10");
    Parser::parseMovesToFile(path, file);
    file.close();
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
    getSurfaceMenu(openC0Popup, true, manager);
  }
  if (openC2Popup) {
    getSurfaceMenu(openC2Popup, false, manager);
  }
  if (openIntersection) {
    getIntersectionMenu(openIntersection, manager);
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
void Scene::getIntersectionMenu(bool &open,
                                const std::unique_ptr<SceneManager> &manager) {
  std::vector<std::shared_ptr<Object>> objects = manager->getObjects();
  static std::shared_ptr<Object> selected1;
  static std::shared_ptr<Object> selected2;
  static bool cursor = false;
  static float step = 1e-3;
  ImGui::Begin("Intersection", &open);
  if (ImGui::BeginCombo("Surface 1", selected1.get() == nullptr
                                         ? ""
                                         : selected1->name.c_str())) {
    for (const auto &obj : objects)
      if (std::dynamic_pointer_cast<Intersectable>(obj))
        if (ImGui::Selectable(obj->name.c_str()))
          selected1 = obj;
    ImGui::EndCombo();
  }
  if (ImGui::BeginCombo("Surface 2", selected2.get() == nullptr
                                         ? ""
                                         : selected2->name.c_str())) {
    for (const auto &obj : objects)
      if (std::dynamic_pointer_cast<Intersectable>(obj))
        if (ImGui::Selectable(obj->name.c_str()))
          selected2 = obj;
    ImGui::EndCombo();
  }
  ImGui::SliderFloat("Step", &step, 1e-3f, 1e-2);
  ImGui::Checkbox("Use cursor", &cursor);
  if (ImGui::Button("Intersect")) {
    std::shared_ptr<Intersectable> i1 =
        std::dynamic_pointer_cast<Intersectable>(selected1);
    std::shared_ptr<Intersectable> i2 =
        std::dynamic_pointer_cast<Intersectable>(selected2);
    open = false;
    manager->addIntersection(i1, i2, cursor, step);
  }
  ImGui::End();
}
void Scene::getSurfaceMenu(bool &open, bool c0,
                           const std::unique_ptr<SceneManager> &manager) {
  static int u = 1, v = 1;
  static float p1 = 1.f, p2 = 1.f;
  static int mode;
  static const char *modes[] = {"Surface", "Cylinder"};

  ImGui::Begin("Add Surface");
  ImGui::Combo("Mode", &mode, modes, IM_ARRAYSIZE(modes));
  ImGui::SliderInt("U Patches", &u, 1, 10);
  ImGui::SliderInt("V Patches", &v, 1, 10);
  ImGui::SliderFloat("R / W", &p1, 1.f, 10.f);
  ImGui::SliderFloat("h", &p2, 1.f, 10.f);
  if (ImGui::Button("Cancel")) {
    open = false;
  }
  ImGui::SameLine();
  if (ImGui::Button("Add")) {
    manager->addSurface(u, v, mode == 1, p1, p2, c0);
    open = false;
  }
  ImGui::End();
}
