#include "../Object.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <vector>

class IntersectionCurve : public Object {
public:
  IntersectionCurve(const std::vector<math137::Vector3f> &points);
  void render(std::shared_ptr<Renderer> &renderer,
              const math137::Vector4f &color) override;
  bool renderObjectMenu() override;

private:
  void setVetrices();
  std::vector<math137::Vector3f> m_points;
  static uint16_t s_count;
};
