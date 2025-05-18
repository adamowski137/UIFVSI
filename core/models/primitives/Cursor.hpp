#pragma once

#include "../Object.hpp"
#include "Vector.hpp"

class Cursor : public Object {
public:
  Cursor();
  void render(std::shared_ptr<Renderer> &renderer,
              const math137::Vector4f &color) override;
  bool renderObjectMenu() override { return false; };

protected:
  void recalculateModel() override;
};
