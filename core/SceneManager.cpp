#include "SceneManager.hpp"
#include "Vector.hpp"
#include "models/Object.hpp"
#include "models/ObjectBuilder.hpp"
#include "models/curves/Curve.hpp"
#include "models/curves/IntersectionCurve.hpp"
#include "models/primitives/Point.hpp"
#include "models/primitives/Torus.hpp"
#include "models/surfaces/BezierC0.hpp"
#include "models/surfaces/Gregory.hpp"
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

void SceneManager::clear() {
  m_observers.clear();
  m_notifyQueue.clear();
  m_virtualObjects.clear();
  m_objects.clear();
  m_allObjects.clear();
  m_parentObjects.clear();
  m_selectedObjects.clear();
}

void SceneManager::notify(const std::weak_ptr<Object> &obj) {
  for (const auto &observer : m_observers[obj]) {
    m_notifyQueue.insert(observer);
  }
  m_virtualObjects[obj].parent.lock();
}
void SceneManager::addObject(std::shared_ptr<Object> obj) {
  m_objects.push_back(obj);
  m_allObjects.push_back(obj);
  m_observers.insert({obj, {}});
}
void SceneManager::deleteObject(const std::weak_ptr<Object> &obj, bool force) {
  for (const auto &obs : m_observers[obj]) {
    if (obs.expired())
      continue;
    std::shared_ptr<Curve> c = std::dynamic_pointer_cast<Curve>(obs.lock());
    if (c)
      c->removePoint(obj);
    std::shared_ptr<Surface> s = std::dynamic_pointer_cast<Surface>(obs.lock());
    if (s && !force)
      return;
    m_notifyQueue.insert(obs);
  }
  m_observers.erase(obj);
  std::vector<int> toDelete;
  for (int i = 0; i < m_allObjects.size(); i++) {
    if (m_allObjects[i].lock() == obj.lock())
      toDelete.push_back(i);
  }
  for (int i = toDelete.size() - 1; i >= 0; i--) {
    m_allObjects.erase(m_allObjects.begin() + toDelete[i]);
  }
  toDelete.clear();
  for (int i = 0; i < m_objects.size(); i++) {
    if (m_objects[i] == obj.lock()) {
      toDelete.push_back(i);
    }
  }
  for (int i = toDelete.size() - 1; i >= 0; i--) {
    m_objects.erase(m_objects.begin() + toDelete[i]);
  }
}
void SceneManager::addObserver(const std::weak_ptr<Object> &obj,
                               const std::weak_ptr<Object> &observer) {
  m_observers[obj].insert(observer);
  m_notifyQueue.insert(observer);
}
void SceneManager::deleteObserver(const std::weak_ptr<Object> &obj,
                                  const std::weak_ptr<Object> &observer) {
  m_observers[obj].erase(observer);
  m_notifyQueue.insert(observer);
}

void SceneManager::selectObjects(const std::set<uint32_t> &indices, bool add) {
  if (!add) {
    m_selectedObjects.clear();
    deleteVirtualObjects();
  }
  for (const auto id : indices) {
    if (id == 0)
      continue;
    m_selectedObjects.push_back(m_allObjects[id - 1]);
    addVirtualObjects(m_allObjects[id - 1]);
  }
}

void SceneManager::addVirtualObjects(const std::weak_ptr<Object> &parent) {
  if (parent.expired())
    return;
  uint16_t idx = 0;
  for (const auto &obj : parent.lock()->getVirtualObjects()) {
    m_virtualObjects[obj] =
        VirtualObjectData(idx++, parent, obj.lock()->getTranslation());
    m_parentObjects[parent].emplace(obj);
    m_allObjects.push_back(obj);
  }
}

void SceneManager::addPoint() {
  std::shared_ptr<Object> point = ObjectBuilder()
                                      .withNewPoint()
                                      .withPosition(m_cursor.getTranslation())
                                      .build();
  addObject(point);
  for (const auto &obj : m_selectedObjects) {
    if (obj.expired())
      continue;
    if (std::shared_ptr<Curve> c =
            std::dynamic_pointer_cast<Curve>(obj.lock())) {
      c->addPoint(point);
      addObserver(point, obj);
      deleteVirtualObjects(c);
      addVirtualObjects(c);
    }
  }
}

void SceneManager::addBezierCurve() {
  std::shared_ptr<Object> bezier =
      ObjectBuilder().withNewBezierCurve(m_selectedObjects).build();
  addObject(bezier);
  for (const auto &p : m_selectedObjects) {
    addObserver(p, bezier);
  }
}

void SceneManager::addBspline() {
  std::shared_ptr<Object> bspline =
      ObjectBuilder().withNewBSpline(m_selectedObjects).build();
  addObject(bspline);
  for (const auto &p : m_selectedObjects) {
    addObserver(p, bspline);
  }
}

void SceneManager::addIBspline() {
  std::shared_ptr<Object> bspline =
      ObjectBuilder().withNewIBSpline(m_selectedObjects).build();
  addObject(bspline);
  for (const auto &p : m_selectedObjects) {
    addObserver(p, bspline);
  }
}

void SceneManager::deleteVirtualObjects() {
  std::vector<int> remove;
  for (int i = 0; i < m_allObjects.size(); i++) {
    if (m_virtualObjects.find(m_allObjects[i]) != m_virtualObjects.end())
      remove.push_back(i);
  }

  for (int i = remove.size() - 1; i >= 0; i--) {
    m_allObjects.erase(m_allObjects.begin() + remove[i]);
  }

  m_virtualObjects.clear();
  m_parentObjects.clear();
}
void SceneManager::deleteVirtualObjects(const std::weak_ptr<Object> &parent) {
  if (parent.expired())
    return;
  for (const auto &obj : m_parentObjects[parent]) {
    auto it = m_virtualObjects.find(obj);
    if (it != m_virtualObjects.end()) {
      m_virtualObjects.erase(it);
    }
    for (int i = 0; i < m_allObjects.size(); i++) {
      if (m_allObjects[i].lock() == obj.lock())
        m_allObjects.erase(m_allObjects.begin() + i);
    }
  }
  m_parentObjects.erase(m_parentObjects.find(parent));
}

std::vector<std::weak_ptr<Object>> SceneManager::getDrawableObjects() const {
  return m_allObjects;
}

void SceneManager::deleteSelected() {
  for (const auto &obj : m_selectedObjects) {
    deleteObject(obj);
  }
  m_selectedObjects.clear();
  recalculateMassCenter();
}

void SceneManager::collapseSelected() {
  std::shared_ptr<Object> newPoint =
      ObjectBuilder().withNewPoint().withPosition({0.f, 0.f, 0.f}).build();
  math137::Vector3f newPos;
  uint16_t count = 0;
  addObject(newPoint);
  for (const auto &p : m_selectedObjects) {
    if (std::dynamic_pointer_cast<Point>(p.lock())) {
      count++;
      newPos = newPos + p.lock()->getTranslation();
      for (const auto &obj : m_observers[p]) {
        obj.lock()->replacePoint(p, newPoint);
        addObserver(newPoint, obj);
      }
      deleteObject(p, true);
    }
  }

  newPoint->setTranslation(newPos / count);
  m_selectedObjects.clear();
  deleteVirtualObjects();
  recalculateMassCenter();
}

void SceneManager::update() {
  for (const auto &obj : m_selectedObjects) {
    if (obj.expired())
      continue;
    for (const auto &obs : m_observers[obj]) {
      m_notifyQueue.insert(obs);
    }
    if (m_virtualObjects.find(obj) == m_virtualObjects.end())
      continue;
    m_virtualObjects[obj].parent.lock()->notifyVirtual(
        m_virtualObjects[obj].index,
        obj.lock()->getTranslation() - m_virtualObjects[obj].prevPos);
    m_virtualObjects[obj].prevPos = obj.lock()->getTranslation();
  }
}

void SceneManager::recalculateMassCenter() {
  math137::Vector3f val;
  uint16_t count = 0;
  for (const auto ind : m_selectedObjects) {
    if (ind.expired())
      continue;
    count++;
    val = val + ind.lock()->getMassCenter();
  }
  if (count == 0)
    m_massCenter.setTranslation({0, 0, 0});

  m_massCenter.setTranslation(val / count);
}

void SceneManager::toggleSelection(const std::weak_ptr<Object> &obj) {
  for (int i = 0; i < m_selectedObjects.size(); i++) {
    if (m_selectedObjects[i].expired())
      continue;
    if (m_selectedObjects[i].lock() == obj.lock()) {
      m_selectedObjects.erase(m_selectedObjects.begin() + i);
      deleteVirtualObjects(obj);
      recalculateMassCenter();
      return;
    }
  }
  m_selectedObjects.push_back(obj);
  addVirtualObjects(obj);
  recalculateMassCenter();
}

bool SceneManager::isSelected(const std::weak_ptr<Object> &obj) const {
  for (const auto &p : m_selectedObjects) {
    if (p.lock() == obj.lock())
      return true;
  }
  return false;
}

bool SceneManager::isVirtual(const std::weak_ptr<Object> &obj) const {
  return !(m_virtualObjects.find(obj) == m_virtualObjects.end());
}

void SceneManager::notifyQueue() {
  for (const auto &obj : m_notifyQueue) {
    if (!obj.expired())
      obj.lock()->notify();
  }

  m_notifyQueue.clear();
}

std::vector<std::tuple<std::shared_ptr<Object>, std::shared_ptr<Object>,
                       std::shared_ptr<Object>, std::shared_ptr<BezierC0>,
                       std::shared_ptr<BezierC0>, std::shared_ptr<BezierC0>>>
SceneManager::containsCycle() const {
  std::map<std::shared_ptr<Object>, std::set<std::shared_ptr<Object>>> graph;
  std::map<std::shared_ptr<Object>, std::set<std::shared_ptr<BezierC0>>>
      pointToSurface;

  for (const auto &obj : m_selectedObjects) {
    if (std::shared_ptr<BezierC0> c0 =
            std::dynamic_pointer_cast<BezierC0>(obj.lock())) {
      auto subgraph = c0->getConnectionsGraph();
      for (const auto &v : subgraph) {
        graph[v.first].insert(v.second.begin(), v.second.end());
        pointToSurface[v.first].insert(c0);
      }
    }
  }

  std::vector<std::tuple<std::shared_ptr<Object>, std::shared_ptr<Object>,
                         std::shared_ptr<Object>, std::shared_ptr<BezierC0>,
                         std::shared_ptr<BezierC0>, std::shared_ptr<BezierC0>>>
      res;
  std::set<std::tuple<std::shared_ptr<Object>, std::shared_ptr<Object>,
                      std::shared_ptr<Object>>>
      used;
  for (const auto &[u, nu] : graph) {
    for (const auto &v : nu) {
      if (u == v)
        continue;
      const auto &nv = graph[v];
      for (const auto &w : nv) {
        if (w == u || w == v)
          continue;
        const auto &nw = graph[w];
        if (nw.find(u) != nw.end()) {
          if (checkIfCycleUsed({u, v, w}, used))
            continue;
          used.insert({u, v, w});

          res.push_back(
              {u, v, w, findCommonElement(pointToSurface[u], pointToSurface[v]),
               findCommonElement(pointToSurface[v], pointToSurface[w]),
               findCommonElement(pointToSurface[w], pointToSurface[u])});
        }
      }
    }
  }

  return res;
}
std::shared_ptr<BezierC0> SceneManager::findCommonElement(
    const std::set<std::shared_ptr<BezierC0>> &a,
    const std::set<std::shared_ptr<BezierC0>> &b) const {

  for (const auto &item : a) {
    if (b.find(item) != b.end()) {
      return item;
    }
  }

  return nullptr;
}

void SceneManager::addGregoryPatch() {
  for (const auto &cycle : containsCycle()) {
    auto [p1, p2, p3, s1, s2, s3] = cycle;
    auto [e1, d1] = s1->getEdgeFromPoints(p1, p2);
    auto [e2, d2] = s2->getEdgeFromPoints(p2, p3);
    auto [e3, d3] = s3->getEdgeFromPoints(p3, p1);

    std::array<std::array<std::weak_ptr<Object>, 4>, 3> edges = {e1, e2, e3};
    std::array<std::array<std::weak_ptr<Object>, 4>, 3> prev = {d1, d2, d3};
    std::shared_ptr<Gregory> g = std::make_shared<Gregory>(edges, prev);
    addObject(g);
    for (const auto &e : edges) {
      for (const auto &p : e) {
        addObserver(p, g);
      }
    }
    for (const auto &e : prev) {
      for (const auto &p : e) {
        addObserver(p, g);
      }
    }
  }
}
bool SceneManager::checkIfCycleUsed(
    const std::tuple<std::shared_ptr<Object>, std::shared_ptr<Object>,
                     std::shared_ptr<Object>> &c1,
    const std::set<std::tuple<std::shared_ptr<Object>, std::shared_ptr<Object>,
                              std::shared_ptr<Object>>> &used) const {
  const auto [p1, p2, p3] = c1;
  if (used.find({p1, p2, p3}) != used.end())
    return true;
  if (used.find({p1, p2, p3}) != used.end())
    return true;
  if (used.find({p3, p1, p2}) != used.end())
    return true;
  if (used.find({p2, p3, p1}) != used.end())
    return true;
  if (used.find({p3, p2, p1}) != used.end())
    return true;
  if (used.find({p1, p3, p2}) != used.end())
    return true;
  if (used.find({p2, p1, p3}) != used.end())
    return true;
  return false;
}

void SceneManager::addIntersection(const std::shared_ptr<Interceptable> &i1,
                                   const std::shared_ptr<Interceptable> &i2) {
  math137::Vector4f start = Interceptable::GradientDescent(i1, i2);
  m_cursor.setTranslation(i2->getValue(start.z(), start.w()));
  std::vector<math137::Vector3f> points =
      Interceptable::GenerateIntersectionPoints(i1, i2, start);
  std::shared_ptr<IntersectionCurve> curve =
      std::make_shared<IntersectionCurve>(points);
  addObject(curve);
}
