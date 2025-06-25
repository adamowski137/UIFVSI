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
  void renderFramebuffer(std::shared_ptr<Renderer> &renderer,
                         unsigned int id) override;
  bool renderObjectMenu() override;
  void notify() override { setVertices(); }
  math137::Vector3f getMassCenter() override;

private:
  void setVertices();
  void setEdges();
  math137::Vector3f lerp(const math137::Vector3f &a, const math137::Vector3f &b,
                         float t);
  std::pair<std::array<math137::Vector3f, 4>, std::array<math137::Vector3f, 4>>
  divideCurve(float t, const std::array<std::weak_ptr<Object>, 4> &points);

  std::array<std::array<std::weak_ptr<Object>, 4>, 3> m_edges;
  std::array<std::array<std::weak_ptr<Object>, 4>, 3> m_prev;
  int m_uSubdivisions;
  int m_vSubdivisions;
};
