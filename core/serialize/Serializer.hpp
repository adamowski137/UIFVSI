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
  static std::shared_ptr<Object>
  DeserializePoint(const json &json,
                   std::map<uint16_t, std::shared_ptr<Object>> &pointMap);
  static math137::Vector3f DeserializeVector(const json &json);
  static math137::Quaternion DeserializeQuaternion(const json &json);

  static void DeserializeTorus(const json &json,
                               const std::unique_ptr<SceneManager> &manger);
  template <typename T>
  static void
  DeserializeCurve(const json &j,
                   const std::map<uint16_t, std::shared_ptr<Object>> pointMap,
                   const std::unique_ptr<SceneManager> &manager) {
    std::string name;
	name = j["name"];
    std::vector<std::weak_ptr<Object>> points;
    for (const auto &p : j["controlPoints"]) {
      points.push_back(pointMap.at(p["id"]));
    }

    auto obj = std::make_shared<T>(points);
    obj->name = name;
    manager->addObject(obj);

    for (const auto &p : points) {
      manager->addObserver(p, obj);
    }
  }
  static void DeserializeSurfaceC0(
      const json &j, const std::map<uint16_t, std::shared_ptr<Object>> pointMap,
      const std::unique_ptr<SceneManager> &manager);
  static void DeserializeSurfaceC2(
      const json &j, const std::map<uint16_t, std::shared_ptr<Object>> pointMap,
      const std::unique_ptr<SceneManager> &manager);
  static std::string JoinStrings(const std::vector<std::string> &strArray);
};
