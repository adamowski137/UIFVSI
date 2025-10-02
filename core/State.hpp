#pragma once
#include "Matrix.hpp"
#include <cmath>
#include <cstdint>
#include <vector>

enum class Mode { DEFAULT, MOVE, ROTATE, SCALE, CAMERA };
enum class Transformation { OBJECT, MASS, CURSOR };
enum class DisplayMode { DEFAULT, STEREO };

class State {
public:
	State()
		: m_mode(Mode::DEFAULT), m_displayMode(DisplayMode::DEFAULT),
		m_transformation(Transformation::OBJECT), m_width(0),
		m_height(0), rbo(0), fbo(0), textureId(0){
	}

  inline void setMode(Mode m) { m_mode = m; }
  inline void setDisplayMode(DisplayMode m) { m_displayMode = m; }
  inline void setTransformation(Transformation t) { m_transformation = t; }
  inline void setDimensions(uint16_t w, uint16_t h) {
    m_width = w;
    m_height = h;
    stencilData.resize(w * h * 4);
  }
  inline void setProjection(const math137::Matrix4f proj) {
    m_projection = proj;
  }
  inline void setMovedProjection(const math137::Matrix4f &left,
                                 const math137::Matrix4f &right) {
    m_movedProjectionLeft = left;
    m_movedProjectionRight = right;
  }
  inline Mode getMode() const { return m_mode; }
  inline DisplayMode getDisplayMode() const { return m_displayMode; }
  inline Transformation getTransformation() const { return m_transformation; }
  inline math137::Matrix4f getProjection() const { return m_projection; }
  inline math137::Matrix4f getLeftProjection() const {
    return m_movedProjectionLeft;
  }
  inline math137::Matrix4f getRightProjection() const {
    return m_movedProjectionRight;
  }
  inline uint16_t getHeight() const { return m_height; }
  inline uint16_t getWidth() const { return m_width; }
  uint32_t textureId;
  uint32_t fbo, rbo;
  std::vector<uint32_t> stencilData;
  const float m_fov = (M_PI_4);
  const float m_near = 0.1f;
  const float m_far = 100.f;

private:
  Mode m_mode;
  DisplayMode m_displayMode;
  Transformation m_transformation;
  uint16_t m_width;
  uint16_t m_height;
  math137::Matrix4f m_projection;
  math137::Matrix4f m_movedProjectionLeft, m_movedProjectionRight;
};
