#pragma once

#include "Surface.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>

class BezierC0 : public Surface {
public:
  BezierC0(const std::vector<std::shared_ptr<Object>> &points,
           uint16_t uPatches, uint16_t vPatches, bool cylinder);
  ~BezierC0() {}

  void notify() override;
  void render(std::shared_ptr<Renderer> &renderer,
              const math137::Vector4f &color) override;

private:
  void setVertices() override;
  void recalculateModel() override { m_update = false; }
  static uint16_t s_count;
};
