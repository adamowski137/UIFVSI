#include "../Object.hpp"
#include "Vector.hpp"
#include <array>
#include <memory>
#include <utility>
class Gregory : public Object {
public:
  Gregory(const std::array<std::array<std::weak_ptr<Object>, 4>, 3> &edges,
          const std::array<std::array<std::weak_ptr<Object>, 4>, 3> &prev);

  void render(std::shared_ptr<Renderer> &renderer,
              const math137::Vector4f &color) override;
  bool renderObjectMenu() override { return false; }

private:
  void setVertices();
  math137::Vector3f lerp(const math137::Vector3f &a, const math137::Vector3f &b,
                         float t);
  std::pair<std::array<math137::Vector3f, 4>, std::array<math137::Vector3f, 4>>
  divideCurve(float t, const std::array<std::weak_ptr<Object>, 4> &points);
  std::array<math137::Vector3f, 3>
  derivativeU(const std::array<std::array<math137::Vector3f, 4>, 4> &points);
  std::array<math137::Vector3f, 3>
  twist(const std::array<std::array<math137::Vector3f, 4>, 4> &points);

  std::array<std::array<std::weak_ptr<Object>, 4>, 3> m_edges;
  std::array<std::array<std::weak_ptr<Object>, 4>, 3> m_prev;
};
