#pragma once

#include "../Object.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>
class Ground : public Object {
public:
  Ground();
  void render(std::shared_ptr<Renderer> &renderer,
              const math137::Vector4f &color) override;
  bool renderObjectMenu() override { return false; }

protected:
  void recalculateModel() override { m_update = false; }

private:
  const float m_gridSize = 10.f;
  const float m_gapSize = 1.f;
  uint32_t m_vao, m_vbo;
};
