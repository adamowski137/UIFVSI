#pragma once

#include <cstdint>
#include <vector>
enum class Mode { DEFAULT, MOVE, ROTATE, SCALE, CAMERA };
enum class Transformation { OBJECT, MASS, CURSOR };

class State {
public:
  inline void setMode(Mode m) { m_mode = m; }
  inline void setTransformation(Transformation t) { m_transformation = t; }
  inline void setDimensions(uint16_t w, uint16_t h) {
    m_width = w;
    m_height = h;
    stencilData.resize(w * h);
  }
  inline Mode getMode() { return m_mode; }
  inline Transformation getTransformation() { return m_transformation; }
  inline uint16_t getHeight() { return m_height; }
  inline uint16_t getWidth() { return m_width; }
  std::vector<uint8_t> stencilData;

private:
  Mode m_mode;
  Transformation m_transformation;
  uint16_t m_width;
  uint16_t m_height;
};
