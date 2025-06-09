#pragma once

#include "Surface.hpp"
#include "Vector.hpp"
#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

class BezierC0 : public Surface {
public:
  BezierC0(const std::vector<std::shared_ptr<Object>> &points,
           uint16_t uPatches, uint16_t vPatches);
  ~BezierC0() {}

  void notify() override;
  void render(std::shared_ptr<Renderer> &renderer,
              const math137::Vector4f &color) override;
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
  static uint16_t s_count;
};
