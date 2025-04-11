#include "Transformations.hpp"
#include "Quaternion.hpp"
#include "State.hpp"
#include "Vector.hpp"
#include "models/Object.hpp"
#include <algorithm>
#include <memory>
#include <vector>

void Transformations::RotateSelected(const std::unique_ptr<Scene> &scene,
                                     State &state,
                                     const math137::Quaternion &rot) {
  std::vector<std::shared_ptr<Object>> objects = scene->getSelected();
  std::for_each(
      objects.begin(), objects.end(), [&rot, &state, &scene](auto &o) {
        Transformation trans = state.getTransformation();
        math137::Vector3f center =
            trans == Transformation::CURSOR
                ? scene->getCursorPosition()
                : (trans == Transformation::MASS ? scene->getMassCenter()
                                                 : o->getTranslation());
        o->rotate(rot, center);
      });
  if (state.getTransformation() != Transformation::OBJECT) {
    scene->notifySelected();
    scene->recalculateMassCenter();
  }
}

void Transformations::ScaleSelected(const std::unique_ptr<Scene> &scene,
                                    State &state, float dx) {
  std::vector<std::shared_ptr<Object>> objects = scene->getSelected();
  std::for_each(objects.begin(), objects.end(), [dx, &state, &scene](auto &o) {
    Transformation trans = state.getTransformation();
    math137::Vector3f center =
        trans == Transformation::CURSOR
            ? scene->getCursorPosition()
            : (trans == Transformation::MASS ? scene->getMassCenter()
                                             : o->getTranslation());
    o->scale(dx, center);
  });
  if (state.getTransformation() != Transformation::OBJECT) {
    scene->notifySelected();
    scene->recalculateMassCenter();
  }
}

void Transformations::MoveSelected(const std::unique_ptr<Scene> &scene,
                                   float dx, float dy, float dz) {
  std::vector<std::shared_ptr<Object>> objects = scene->getSelected();
  std::for_each(objects.begin(), objects.end(),
                [dx, dy, dz, &scene](auto &o) { o->move({dx, dy, dz}); });
  scene->recalculateMassCenter();
  scene->notifySelected();
}

void Transformations::SetCursor(const std::unique_ptr<Scene> &scene,
                                Camera &camera, float ndcX, float ndcY) {
  math137::Vector3f dist = (scene->getCursorPosition() - camera.getPosition());
  float l = sqrtf(dist * dist);

  math137::Vector4f clipNear = {ndcX, ndcY, -1.f, 1.f};
  math137::Vector4f clipFar = {ndcX, ndcY, 1.f, 1.f};

  math137::Matrix4f invView = camera.getInverseView();
  math137::Matrix4f invProj = scene->getInvProjection();

  math137::Vector4f viewNear = invProj * clipNear;
  math137::Vector4f viewFar = invProj * clipFar;
  viewNear = viewNear * (1 / viewNear.w());
  viewFar = viewFar * (1 / viewFar.w());

  math137::Vector4f ray = viewFar - viewNear;
  ray.normalize();

  math137::Vector4f worldRay = invView * ray;

  scene->setCursorPosition(
      camera.getPosition() +
      math137::Vector3f{worldRay.x() * l, worldRay.y() * l, worldRay.z() * l});
}
