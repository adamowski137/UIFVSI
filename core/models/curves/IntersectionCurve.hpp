#pragma once

#include "../Object.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>
#include <vector>

class Intersectable;

class IntersectionCurve : public Object {
public:
  IntersectionCurve(const math137::Vector4f &start,
                    const std::weak_ptr<Intersectable> &i1,
                    const std::weak_ptr<Intersectable> &i2, float step = 1e-3f);
  ~IntersectionCurve();
  void render(std::shared_ptr<Renderer> &renderer,
              const math137::Vector4f &color) override;
  void renderFramebuffer(std::shared_ptr<Renderer> &renderer,
                         unsigned int id) override;
  bool renderObjectMenu() override;
  std::vector<math137::Vector3f> getPoints() const;
  size_t getPointCount() const { return m_params.size(); }
  void unionTrimmingTexture(uint16_t x, uint16_t y, uint8_t surfaceIndex);
  void intersectTrimmingTexture(uint16_t x, uint16_t y, uint8_t surfaceIndex);
private:
  void setVertices();
  void setTextures();
  void bresenhamAlgorithm(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                          std::vector<uint8_t> &data);
  void putPixel(uint16_t x, uint16_t y, std::vector<uint8_t> &data);

  const uint16_t m_width = 512;
  const uint16_t m_height = 512;
  GLuint m_textureIds[2];
  bool m_openPopup[2];
  float m_step;
  std::weak_ptr<Intersectable> m_intersectable1;
  std::weak_ptr<Intersectable> m_intersectable2;
  std::vector<math137::Vector4f> m_params;
  math137::Vector4f m_start;
};
