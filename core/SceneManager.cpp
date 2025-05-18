#include "SceneManager.hpp"
#include "Vector.hpp"
#include "models/Object.hpp"
#include "models/ObjectBuilder.hpp"
#include "models/curves/Curve.hpp"
#include <cstdint>
#include <memory>
#include <vector>

void SceneManager::notify(const std::weak_ptr<Object> &obj) {
  for (const auto &observer : m_observers[obj]) {
    m_notifyQueue.insert(observer);
  }
  m_virtualObjects[obj].parent.lock();
}
void SceneManager::addObject(const std::shared_ptr<Object> &obj) {
  m_objects.push_back(obj);
  m_allObjects.push_back(obj);
  m_observers.insert({obj, {}});
}
void SceneManager::deleteObject(const std::weak_ptr<Object> &obj) {
  for (const auto &obs : m_observers[obj]) {
    if (obs.expired())
      continue;
    std::shared_ptr<Curve> c = std::dynamic_pointer_cast<Curve>(obs.lock());
    if (c)
      c->removePoint(obj);
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

void SceneManager::selectObjects(const std::set<uint8_t> &indices, bool add) {
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
  if (m_selectedObjects.size() == 0) {
    m_massCenter.setTranslation({0.f, 0.f, 0.f});
    return;
  }

  math137::Vector3f val;
  uint16_t count = 0;
  for (const auto ind : m_selectedObjects) {
    if (ind.expired() ||
        std::dynamic_pointer_cast<Curve>(ind.lock()) != nullptr)
      continue;
    count++;
    val = val + ind.lock()->getTranslation();
  }
  if (count == 0)
    m_massCenter.setTranslation({0, 0, 0});

  m_massCenter.setTranslation(val * (1.f / count));
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
