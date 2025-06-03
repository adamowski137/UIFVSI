#pragma once

#include "../Object.hpp"
#include <cstdint>
#include <memory>

class Surface : public Object {
public:
  Surface(const std::vector<std::shared_ptr<Object>> &points, uint16_t uPatches,
          uint16_t vPatches, bool cylinder, ShaderType type);
  bool renderObjectMenu() override;

protected:
  uint16_t m_uPatches;
  uint16_t m_vPatches;

  int m_divisionsU;
  int m_divisionsV;

  bool m_cylinder;
  std::vector<std::vector<std::weak_ptr<Object>>> m_points;
  std::vector<uint16_t> m_edges;

  virtual void setVertices() = 0;
  virtual void setEdges() = 0;
  virtual void replacePoint(const std::weak_ptr<Object> &current,
                            const std::shared_ptr<Object> &newPoint) override;

private:
  virtual std::string getTypeName() const = 0;
  friend class Serializer;
};
