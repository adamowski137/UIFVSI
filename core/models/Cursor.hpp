#pragma once

#include "Object.hpp"
#include "Vector.hpp"
#include <memory>

class Cursor : public Object {
public:
  Cursor();
  void render(std::shared_ptr<Renderer> &renderer,
              const math137::Vector4f &color) override;
  void renderObjectMenu() override {};

protected:
  void recalculateModel() override;
};
