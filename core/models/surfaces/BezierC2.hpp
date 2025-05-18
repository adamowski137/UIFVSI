#pragma once

#include "Surface.hpp"
#include "Vector.hpp"
class BezierC2 : public Surface {
public:
  BezierC2(const std::vector<std::shared_ptr<Object>> &points,
           uint16_t uPatches, uint16_t vPatches, bool cylinder);
  ~BezierC2() {}

  void notify() override;
  void render(std::shared_ptr<Renderer> &renderer,
              const math137::Vector4f &color) override;

private:
  void setVertices() override;
  void recalculateModel() override { m_update = false; }
  static uint16_t s_count;
  std::vector<std::vector<math137::Vector3f>> m_bezierPoints;
};
