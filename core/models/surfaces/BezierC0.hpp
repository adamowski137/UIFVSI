#pragma once

#include "../Intersectable.hpp"
#include "Surface.hpp"
#include "Vector.hpp"
#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

class BezierC0 : public Surface, public Intersectable {
public:
  BezierC0(const std::vector<std::vector<std::shared_ptr<Object>>> &points,
           uint16_t uPatches, uint16_t vPatches);
  ~BezierC0() {}

  virtual math137::Vector3f uDerivative(float u, float v) const override;
  virtual math137::Vector3f vDerivative(float u, float v) const override;
  virtual math137::Vector3f getValue(float u, float v) const override;
  virtual bool wrappableU() const override {
    return m_points[0][0].lock() == m_points[m_points.size() - 1][0].lock();
  }
  virtual bool wrappableV() const override {
    return m_points[0][0].lock() == m_points[0][m_points.size() - 1].lock();
  }

  void notify() override;
  void render(std::shared_ptr<Renderer> &renderer,
              const math137::Vector4f &color) override;
  void renderFramebuffer(std::shared_ptr<Renderer> &renderer,
                         unsigned int id) override;
  std::map<std::shared_ptr<Object>, std::set<std::shared_ptr<Object>>>
  getConnectionsGraph() const;
  std::pair<std::array<std::weak_ptr<Object>, 4>,
            std::array<std::weak_ptr<Object>, 4>>
  getEdgeFromPoints(const std::shared_ptr<Object> &p1,
                    const std::shared_ptr<Object> &p2) const;

private:
  void setVertices() override;
  void setEdges() override;
  void recalculateModel() override { m_update = false; }
  std::string getTypeName() const override { return "bezierSurfaceC0"; }
  float bernstein(int i, float t) const;
  float dbernstein(int i, float t) const;
};
