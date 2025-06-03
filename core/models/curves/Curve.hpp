#pragma once

#include "../Object.hpp"
#include <memory>
#include <string>
#include <vector>
class Curve : public Object {
public:
  Curve(const std::vector<std::weak_ptr<Object>> &points)
      : Object(ShaderType::CURVE), m_points(points.begin(), points.end()) {}
  virtual void addPoint(const std::weak_ptr<Object> &p);
  virtual void removePoint(const std::weak_ptr<Object> &p);
  virtual void replacePoint(const std::weak_ptr<Object> &current,
                            const std::shared_ptr<Object> &newPoint) override;
  bool containsPoint(const std::weak_ptr<Object> &p) const;

protected:
  virtual void setVertices() = 0;
  virtual void setEdges() const = 0;
  virtual std::string getTypeName() const = 0;

  std::vector<std::weak_ptr<Object>> m_points;

  friend class Serializer;
};
