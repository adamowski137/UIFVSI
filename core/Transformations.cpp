#include "Transformations.hpp"
#include "Quaternion.hpp"
#include "State.hpp"
#include "Vector.hpp"
#include "models/Object.hpp"
#include <algorithm>
#include <memory>
#include <vector>

void Transformations::RotateSelected(
    const std::unique_ptr<SceneManager> &sceneManager, State &state,
    const math137::Quaternion &rot) {
  std::vector<std::weak_ptr<Object>> objects = sceneManager->getSelected();
  std::for_each(
      objects.begin(), objects.end(), [&rot, &state, &sceneManager](auto &o) {
        Transformation trans = state.getTransformation();
        math137::Vector3f center =
            trans == Transformation::CURSOR
                ? sceneManager->getCursorPosition()
                : (trans == Transformation::MASS ? sceneManager->getMassCenter()
                                                 : o.lock()->getTranslation());
        o.lock()->rotate(rot, center);
      });
  if (state.getTransformation() != Transformation::OBJECT) {
    sceneManager->update();
    sceneManager->recalculateMassCenter();
  }
}

void Transformations::ScaleSelected(
    const std::unique_ptr<SceneManager> &sceneManager, State &state, float dx) {
  std::vector<std::weak_ptr<Object>> objects = sceneManager->getSelected();
  std::for_each(
      objects.begin(), objects.end(), [dx, &state, &sceneManager](auto &o) {
        Transformation trans = state.getTransformation();
        math137::Vector3f center =
            trans == Transformation::CURSOR
                ? sceneManager->getCursorPosition()
                : (trans == Transformation::MASS ? sceneManager->getMassCenter()
                                                 : o.lock()->getTranslation());
        o.lock()->scale(dx, center);
      });
  if (state.getTransformation() != Transformation::OBJECT) {
    sceneManager->update();
    sceneManager->recalculateMassCenter();
  }
}

void Transformations::MoveSelected(
    const std::unique_ptr<SceneManager> &sceneManager, float dx, float dy,
    float dz) {
  std::vector<std::weak_ptr<Object>> objects = sceneManager->getSelected();
  std::for_each(
      objects.begin(), objects.end(),
      [dx, dy, dz, &sceneManager](auto &o) { o.lock()->move({dx, dy, dz}); });
  sceneManager->recalculateMassCenter();
  sceneManager->update();
}

void Transformations::SetCursor(
    const std::unique_ptr<SceneManager> &sceneManager, Camera &camera,
    float ndcX, float ndcY) {
  math137::Vector3f dist =
      (sceneManager->getCursorPosition() - camera.getPosition());
  float l = sqrtf(dist * dist);

  math137::Vector4f clipNear = {ndcX, ndcY, -1.f, 1.f};
  math137::Vector4f clipFar = {ndcX, ndcY, 1.f, 1.f};

  math137::Matrix4f invView = camera.getInverseView();
  math137::Matrix4f invProj = sceneManager->getInvProjection();

  math137::Vector4f viewNear = invProj * clipNear;
  math137::Vector4f viewFar = invProj * clipFar;
  viewNear = viewNear * (1 / viewNear.w());
  viewFar = viewFar * (1 / viewFar.w());

  math137::Vector4f ray = viewFar - viewNear;
  ray.normalize();

  math137::Vector4f worldRay = invView * ray;

  sceneManager->setCursorPosition(
      camera.getPosition() +
      math137::Vector3f{worldRay.x() * l, worldRay.y() * l, worldRay.z() * l});
}
