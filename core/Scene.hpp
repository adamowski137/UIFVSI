#pragma once

#include "SceneManager.hpp"
#include "Vector.hpp"
#include "imgui.h"
#include "models/primitives/Ground.hpp"
#include "render/Renderer.hpp"
#include <memory>

class BezierC0;
class BezierC2;

class Scene {
public:
  Scene();

  void render(std::shared_ptr<Renderer> &renderer,
              std::unique_ptr<SceneManager> &manager);
  void renderMenu(std::unique_ptr<SceneManager> &manager);

private:
  Ground m_ground;
  const math137::Vector4f m_defaultColor =
      math137::Vector4f(1.f, 1.f, 0.f, 1.f);
  const math137::Vector4f m_selectedColor =
      math137::Vector4f(1.f, 0.2f, 0.2f, 1.f);
  const math137::Vector4f m_centerColor = math137::Vector4f(1.f, 1.f, 1.f, 1.f);
  template <typename T>
  void getSurfaceMenu(bool &open,
                      const std::unique_ptr<SceneManager> &manager) {
    static int u = 1, v = 1;
    static float p1 = 0.1f, p2 = 0.1f;
    static int mode;
    static const char *modes[] = {"Surface", "Cylinder"};

    ImGui::Begin("Add Surface");
    ImGui::Combo("Mode", &mode, modes, IM_ARRAYSIZE(modes));
    ImGui::SliderInt("U Patches", &u, 1, 10);
    ImGui::SliderInt("V Patches", &v, 1, 10);
    ImGui::SliderFloat("R / W", &p1, 0.01f, 1.f);
    ImGui::SliderFloat("h", &p2, 0.01f, 1.f);
    if (ImGui::Button("Cancel")) {
      open = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Add")) {
      manager->addSurface<T>(u, v, mode == 1, p1, p2);
      open = false;
    }
    ImGui::End();
  }
};
