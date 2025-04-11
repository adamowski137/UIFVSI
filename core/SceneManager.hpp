#pragma once

#include "models/Object.hpp"
#include <cstdint>
#include <map>
#include <memory>
#include <set>

struct ObjectPtrCompare {
  bool operator()(const std::weak_ptr<Object> &o1,
                  const std::weak_ptr<Object> &o2) const {
    return o1.lock().get() < o2.lock().get();
  }
};

class SceneManager {
public:
  SceneManager() : m_observers() {}
  void notify(const std::weak_ptr<Object> &obj);
  void addObject(const std::weak_ptr<Object> &obj);
  void deleteObject(const std::weak_ptr<Object> &obj);
  void addObserver(const std::weak_ptr<Object> &obj,
                   const std::weak_ptr<Object> &observer);
  void deleteObserver(const std::weak_ptr<Object> &obj,
                      const std::weak_ptr<Object> &observer);

private:
  std::map<uintptr_t, std::set<std::weak_ptr<Object>, ObjectPtrCompare>>
      m_observers;
};
