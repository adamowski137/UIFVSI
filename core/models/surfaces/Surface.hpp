#pragma once

#include "../Object.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <memory>
#include <vector>

class Surface : public Object {
public:
  Surface(const std::vector<std::vector<std::shared_ptr<Object>>> &points,
          uint16_t uPatches, uint16_t vPatches, ShaderType type);
  bool renderObjectMenu() override;

protected:
  uint16_t m_uPatches;
  uint16_t m_vPatches;

  int m_divisionsU;
  int m_divisionsV;

  std::vector<std::vector<std::weak_ptr<Object>>> m_points;
  std::vector<uint16_t> m_edges;

  virtual void setVertices() = 0;
  virtual void setEdges() = 0;
  virtual void replacePoint(const std::weak_ptr<Object> &current,
                            const std::shared_ptr<Object> &newPoint) override;
  virtual math137::Vector3f getMassCenter() override;

private:
  virtual std::string getTypeName() const = 0;
  friend class Serializer;
};
