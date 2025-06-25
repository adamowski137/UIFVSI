#pragma once

#include "Matrix.hpp"
#include "Vector.hpp"
#include "models/Object.hpp"
#include "models/primitives/Cursor.hpp"
#include "models/primitives/Point.hpp"
#include "models/primitives/Torus.hpp"
#include "models/surfaces/BezierC0.hpp"
#include "models/surfaces/BezierC2.hpp"
#include "models/surfaces/SurfaceBuilder.hpp"
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <vector>

struct ObjectPtrCompare {
  bool operator()(const std::weak_ptr<Object> &o1,
                  const std::weak_ptr<Object> &o2) const {
    return o1.lock().get() < o2.lock().get();
  }
};

struct VirtualObjectData {
  VirtualObjectData() : index(0), parent(), prevPos() {}
  VirtualObjectData(uint16_t index, std::weak_ptr<Object> parent,
                    const math137::Vector3f &prev)
      : index(index), parent(parent), prevPos(prev) {}
  uint16_t index;
  std::weak_ptr<Object> parent;
  math137::Vector3f prevPos;
};

class SceneManager {
public:
  SceneManager()
      : m_observers(), m_parentObjects(), m_virtualObjects(), m_notifyQueue() {}
  void clear();
  void notify(const std::weak_ptr<Object> &obj);
  void addObject(std::shared_ptr<Object> obj);
  void deleteObject(const std::weak_ptr<Object> &obj, bool force = false);
  void addObserver(const std::weak_ptr<Object> &obj,
                   const std::weak_ptr<Object> &observer);
  void deleteObserver(const std::weak_ptr<Object> &obj,
                      const std::weak_ptr<Object> &observer);
  void selectObjects(const std::set<uint32_t> &indices, bool add);
  void addBezierCurve();
  void addBspline();
  void addIBspline();
  void addPoint();
  void collapseSelected();

  void addVirtualObjects(const std::weak_ptr<Object> &parent);
  void deleteVirtualObjects(const std::weak_ptr<Object> &parent);
  void deleteVirtualObjects();
  void deleteSelected();
  void update();
  void recalculateMassCenter();
  void toggleSelection(const std::weak_ptr<Object> &obj);
  void notifyQueue();
  void addGregoryPatch();
  void addIntersection(const std::shared_ptr<Intersectable> &i1,
                       const std::shared_ptr<Intersectable> &i2, bool cur,
                       float step);
  void toIBSpline();

  bool isSelected(const std::weak_ptr<Object> &obj) const;
  bool isVirtual(const std::weak_ptr<Object> &obj) const;
  std::vector<std::tuple<std::shared_ptr<Object>, std::shared_ptr<Object>,
                         std::shared_ptr<Object>, std::shared_ptr<BezierC0>,
                         std::shared_ptr<BezierC0>, std::shared_ptr<BezierC0>>>
  containsCycle() const;
  std::vector<std::shared_ptr<Intersectable>> getIntersectable() const;

  std::vector<std::weak_ptr<Object>> getDrawableObjects() const;

  inline std::vector<std::weak_ptr<Object>> getSelected() const {
    return m_selectedObjects;
  }
  inline std::vector<std::shared_ptr<Object>> getObjects() const {
    return m_objects;
  }

  inline void setCursorPosition(const math137::Vector3f &pos) {
    m_cursor.setTranslation(pos);
  }
  inline math137::Vector3f getCursorPosition() {
    return m_cursor.getTranslation();
  }
  inline math137::Vector3f getMassCenter() {
    return m_massCenter.getTranslation();
  }
  inline void setInvProjection(const math137::Matrix4f &m) {
    m_invprojection = m;
  }
  inline math137::Matrix4f getInvProjection() { return m_invprojection; }

  void addSurface(int uPatches, int vPatches, bool cylinder, float p1, float p2,
                  bool c0);
  Point m_massCenter;
  Cursor m_cursor;

private:
  std::shared_ptr<BezierC0>
  findCommonElement(const std::set<std::shared_ptr<BezierC0>> &s1,
                    const std::set<std::shared_ptr<BezierC0>> &s2) const;
  bool checkIfCycleUsed(
      const std::tuple<std::shared_ptr<Object>, std::shared_ptr<Object>,
                       std::shared_ptr<Object>> &c1,
      const std::set<
          std::tuple<std::shared_ptr<Object>, std::shared_ptr<Object>,
                     std::shared_ptr<Object>>> &used) const;

  // TODO: figure where to put this
  math137::Matrix4f m_invprojection;
  // objects
  std::vector<std::shared_ptr<Object>> m_objects;
  std::vector<std::weak_ptr<Object>> m_allObjects;
  std::set<std::weak_ptr<Object>, ObjectPtrCompare> m_notifyQueue;

  // relations
  std::vector<std::weak_ptr<Object>> m_selectedObjects;
  std::map<std::weak_ptr<Object>,
           std::set<std::weak_ptr<Object>, ObjectPtrCompare>, ObjectPtrCompare>
      m_observers;
  std::map<std::weak_ptr<Object>, VirtualObjectData, ObjectPtrCompare>
      m_virtualObjects;
  std::map<std::weak_ptr<Object>,
           std::set<std::weak_ptr<Object>, ObjectPtrCompare>, ObjectPtrCompare>
      m_parentObjects;
};
