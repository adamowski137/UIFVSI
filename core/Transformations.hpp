#pragma once
#include "Camera.hpp"
#include "Quaternion.hpp"
#include "SceneManager.hpp"
#include "State.hpp"
#include <memory>

class Transformations {
public:
  static void RotateSelected(const std::unique_ptr<SceneManager> &scene,
                             State &s, const math137::Quaternion &rot);
  static void ScaleSelected(const std::unique_ptr<SceneManager> &scene,
                            State &s, float dx);
  static void MoveSelected(const std::unique_ptr<SceneManager> &scene, float dx,
                           float dy, float dz);
  static void SetCursor(const std::unique_ptr<SceneManager> &scene,
                        Camera &camera, float ndcX, float ndcY);
};
