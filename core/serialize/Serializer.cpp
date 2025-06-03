#include "Serializer.hpp"
#include "../models/curves/BSpline.hpp"
#include "../models/curves/BezierCurve.hpp"
#include "../models/curves/IBSpline.hpp"
#include "../models/primitives/Torus.hpp"
#include "../models/surfaces/BezierC0.hpp"
#include "../models/surfaces/BezierC2.hpp"
#include "Quaternion.hpp"
#include "Vector.hpp"
#include <cstdint>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

void Serializer::SerializeToFile(const std::string &path,
                                 const std::unique_ptr<SceneManager> &manager) {
  std::ofstream out(path);
  out << Serialize(manager);
  out.close();
}

std::string
Serializer::Serialize(const std::unique_ptr<SceneManager> &manager) {
  std::vector<std::string> points;
  std::vector<std::string> objects;

  for (const auto &obj : manager->getObjects()) {
    if (std::shared_ptr<Point> p = std::dynamic_pointer_cast<Point>(obj))
      points.push_back(Serialize(p));
    if (std::shared_ptr<Torus> p = std::dynamic_pointer_cast<Torus>(obj))
      objects.push_back(Serialize(p));
    if (std::shared_ptr<Curve> p = std::dynamic_pointer_cast<Curve>(obj))
      objects.push_back(Serialize(p));
    if (std::shared_ptr<Surface> p = std::dynamic_pointer_cast<Surface>(obj))
      objects.push_back(Serialize(p));
  }

  std::stringstream ss;
  ss << "{\n";
  ss << "\"points\": [\n";
  ss << JoinStrings(points);
  ss << "],\n";
  ss << "\"geometry\": [\n";
  ss << JoinStrings(objects);
  ss << "]\n";
  ss << "}";

  return ss.str();
}

std::string Serializer::Serialize(const std::shared_ptr<Point> &p) {
  std::stringstream ss;
  ss << "{\n";
  ss << "\"id\": " << p->m_id << ",\n";
  ss << "\"position\": " << Serialize(p->m_translation) << "\n";
  ss << "}\n";
  return ss.str();
}

std::string Serializer::Serialize(const std::shared_ptr<Torus> &t) {
  std::stringstream ss;
  ss << "{\n";
  ss << "\"objectType\": \"torus\",\n";
  ss << "\"id\": " << t->m_id << ",\n";
  ss << "\"position\": " << Serialize(t->m_translation) << ",\n";
  ss << "\"rotation\": " << Serialize(t->m_rotation) << ",\n";
  ss << "\"scale\": " << Serialize(t->m_scale) << ",\n";
  ss << "\"samples\": " << "{ \"u\": " << t->m_alphaSamples
     << ", \"v\": " << t->m_betaSamples << "},\n";
  ss << "\"smallRadius\": " << t->getSmallR() << ",\n";
  ss << "\"largeRadius\": " << t->getBigR() << "\n";
  ss << "}\n";
  return ss.str();
}

std::string Serializer::Serialize(const std::shared_ptr<Curve> &curve) {
  std::vector<std::string> references;
  for (const auto &p : curve->m_points) {
    references.push_back("{ \"id\":" + std::to_string(p.lock()->m_id) + "}");
  }

  std::stringstream ss;
  ss << "{\n";
  ss << "\"objectType\": \"" << curve->getTypeName() << "\",\n";
  ss << "\"id\": " << curve->m_id << ",\n";
  ss << "\"name\": \"" << curve->name << "\",\n";
  ss << "\"controlPoints\": [" << JoinStrings(references) << "]\n";
  ss << "}\n";

  return ss.str();
}

std::string Serializer::Serialize(const std::shared_ptr<Surface> &surface) {
  std::vector<std::string> references;
  uint16_t uPoints = 4 + (surface->m_uPatches - 1) * 3;
  uint16_t vPoints = 4 + (surface->m_vPatches - 1) * 3;
  for (uint16_t i = 0; i < uPoints; i++) {
    for (const auto &p : surface->m_points[i % surface->m_points.size()])
      references.push_back("{ \"id\":" + std::to_string(p.lock()->m_id) + "}");
  }

  std::stringstream ss;
  ss << "{\n";
  ss << "\"objectType\": \"" << surface->getTypeName() << "\",\n";
  ss << "\"id\": " << surface->m_id << ",\n";
  ss << "\"name\": \"" << surface->name << "\",\n";
  ss << "\"controlPoints\": [" << JoinStrings(references) << "],\n";
  ss << "\"size\": {\"u\": " << uPoints << ", \"v\":" << vPoints << "},\n";
  ss << "\"samples\": {\"u\": " << surface->m_divisionsU
     << ", \"v\":" << surface->m_divisionsV << "}\n";
  ss << "}\n";

  return ss.str();
}

std::string Serializer::Serialize(const math137::Vector3f &vec) {
  std::stringstream ss;
  ss << "{ ";
  ss << "\"x\": " << vec.x() << ", ";
  ss << "\"y\": " << vec.y() << ", ";
  ss << "\"z\": " << vec.z() << "";
  ss << " }";
  return ss.str();
}

std::string Serializer::Serialize(const math137::Quaternion &vec) {
  std::stringstream ss;
  ss << "{ ";
  ss << "\"x\": " << vec.a << ", ";
  ss << "\"y\": " << vec.b << ", ";
  ss << "\"z\": " << vec.c << ", ";
  ss << "\"w\": " << vec.d << "";
  ss << " }";
  return ss.str();
}

std::string Serializer::JoinStrings(const std::vector<std::string> &strArray) {
  uint16_t n = strArray.size();
  if (n == 0)
    return "";
  std::stringstream ss;
  ss << strArray[0];
  for (uint16_t i = 1; i < n; i++) {
    ss << ", " << strArray[i];
  }

  return ss.str();
}

void Serializer::DeserializeFromFile(
    const std::string &path, const std::unique_ptr<SceneManager> &manager) {
  std::ifstream file(path);
  json j;
  file >> j;
  Deserialize(j, manager);
}

void Serializer::Deserialize(const json &j,
                             const std::unique_ptr<SceneManager> &manager) {
  manager->clear();
  std::map<uint16_t, std::shared_ptr<Object>> points;
  for (const auto &p : j["points"]) {
    auto point = DeserializePoint(p);
    points.emplace(point->m_id, point);
    manager->addObject(point);
  }
  for (const auto &obj : j["geometry"]) {
    std::string name = obj["objectType"];
    if (name == "torus")
      DeserializeTorus(obj, manager);
    if (name == "bezierC0")
      DeserializeCurve<BezierCurve>(obj, points, manager);
    if (name == "bezierC2")
      DeserializeCurve<BSpline>(obj, points, manager);
    if (name == "interpolatedC2")
      DeserializeCurve<IBSpline>(obj, points, manager);
    if (name == "bezierSurfaceC0")
      DeserializeSurface<BezierC0>(obj, points, manager);
    if (name == "bezierSurfaceC2")
      DeserializeSurface<BezierC2>(obj, points, manager);
  }
}

std::shared_ptr<Object> Serializer::DeserializePoint(const json &j) {
  math137::Vector3f pos;
  uint16_t id;
  id = j["id"];
  pos = DeserializeVector(j["position"]);
  return ObjectBuilder().withNewPoint().withId(id).withPosition(pos).build();
}

void Serializer::DeserializeTorus(
    const json &j, const std::unique_ptr<SceneManager> &manager) {
  math137::Vector3f pos;
  math137::Vector3f scale;
  math137::Quaternion quat;
  uint16_t alphaSamples, betaSamples;
  float r, R;
  pos = DeserializeVector(j["position"]);
  quat = DeserializeQuaternion(j["rotation"]);
  quat.normalize();
  scale = DeserializeVector(j["scale"]);
  alphaSamples = j["samples"]["u"];
  betaSamples = j["samples"]["v"];
  r = j["smallRadius"];
  R = j["largeRadius"];

  std::shared_ptr<Torus> t = std::make_shared<Torus>(R, r);
  t->m_alphaSamples = alphaSamples;
  t->m_betaSamples = betaSamples;
  t->m_translation = pos;
  t->m_rotation = quat;
  t->m_scale = scale;
  t->setVertexData();

  manager->addObject(t);
}

math137::Vector3f Serializer::DeserializeVector(const json &j) {
  return math137::Vector3f(j["x"], j["y"], j["z"]);
}

math137::Quaternion Serializer::DeserializeQuaternion(const json &j) {
  return math137::Quaternion(j["x"], j["y"], j["z"], j["w"]);
}
