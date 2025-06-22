#pragma once

#include "../SceneManager.hpp"
#include "../models/curves/Curve.hpp"
#include "../models/primitives/Point.hpp"
#include "../models/primitives/Torus.hpp"
#include "../models/surfaces/Surface.hpp"
#include "Quaternion.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

class Serializer {
public:
  static void SerializeToFile(const std::string &path,
                              const std::unique_ptr<SceneManager> &manager);

  static void DeserializeFromFile(const std::string &path,
                                  const std::unique_ptr<SceneManager> &manager);

private:
  static std::string Serialize(const std::unique_ptr<SceneManager> &manger);
  static std::string Serialize(const std::shared_ptr<Torus> &t);
  static std::string Serialize(const std::shared_ptr<Point> &p);
  static std::string Serialize(const std::shared_ptr<Curve> &c);
  static std::string Serialize(const std::shared_ptr<Surface> &s);

  static std::string Serialize(const math137::Vector3f &vec);
  static std::string Serialize(const math137::Quaternion &vec);

  static void Deserialize(const json &json,
                          const std::unique_ptr<SceneManager> &manager);
  static std::shared_ptr<Object> DeserializePoint(const json &json);
  static math137::Vector3f DeserializeVector(const json &json);
  static math137::Quaternion DeserializeQuaternion(const json &json);

  static void DeserializeTorus(const json &json,
                               const std::unique_ptr<SceneManager> &manger);
  template <typename T>
  static void
  DeserializeCurve(const json &j,
                   const std::map<uint16_t, std::shared_ptr<Object>> pointMap,
                   const std::unique_ptr<SceneManager> &manager) {
    std::vector<std::weak_ptr<Object>> points;
    for (const auto &p : j["controlPoints"]) {
      points.push_back(pointMap.at(p["id"]));
    }

    auto obj = std::make_shared<T>(points);

    manager->addObject(obj);

    for (const auto &p : points) {
      manager->addObserver(p, obj);
    }
  }
  template <typename T>
  static void
  DeserializeSurface(const json &j,
                     const std::map<uint16_t, std::shared_ptr<Object>> pointMap,
                     const std::unique_ptr<SceneManager> &manager) {
    uint16_t uPoints, vPoints, m_uSubdivisions, m_vSubdivisions;
    std::vector<std::shared_ptr<Object>> points;

    uPoints = j["size"]["u"];
    vPoints = j["size"]["v"];

    m_uSubdivisions = j["samples"]["u"];
    m_vSubdivisions = j["samples"]["v"];

    for (const auto &p : j["controlPoints"]) {
      points.push_back(pointMap.at(p["id"]));
    }

    bool cylinder = false;
    uint16_t uPatches = 1 + (uPoints - 4) / 3;
    uint16_t vPatches = 1 + (vPoints - 4) / 3;

    std::vector<std::shared_ptr<Object>> transposed(points.size());

    for (uint16_t i = 0; i < points.size(); i++) {
      uint16_t u = i / vPoints;
      uint16_t v = i % vPoints;
      transposed[i] = points[v * uPoints + u];
    }

    auto obj = std::make_shared<T>(transposed, uPatches, vPatches);

    manager->addObject(obj);

    for (const auto &p : points) {
      manager->addObserver(p, obj);
    }
  }

  static std::string JoinStrings(const std::vector<std::string> &strArray);
};
