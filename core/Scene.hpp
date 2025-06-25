#pragma once

#include "Camera.hpp"
#include "SceneManager.hpp"
#include "State.hpp"
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
  void renderToFramebuffer(std::shared_ptr<Renderer> &renderer,
                           std::unique_ptr<SceneManager> &manager,
                           const Camera &camera, const State &state);
  void render(std::shared_ptr<Renderer> &renderer,
              std::unique_ptr<SceneManager> &manager, const State &state);
  void renderMenu(std::unique_ptr<SceneManager> &manager, State &state);

private:
  Ground m_ground;
  const math137::Vector4f m_defaultColor =
      math137::Vector4f(1.f, 1.f, 0.f, 1.f);
  const math137::Vector4f m_selectedColor =
      math137::Vector4f(1.f, 0.2f, 0.2f, 1.f);
  const math137::Vector4f m_centerColor = math137::Vector4f(1.f, 1.f, 1.f, 1.f);
  const math137::Vector4f m_leftColor = math137::Vector4f(1.f, 0.f, 0.f, 1.f);
  const math137::Vector4f m_rightColor = math137::Vector4f(0.f, 1.f, 1.f, 1.f);
  void getIntersectionMenu(bool &open,
                           const std::unique_ptr<SceneManager> &manager);
  void getSurfaceMenu(bool &open, bool c0,
                      const std::unique_ptr<SceneManager> &manager);
};
