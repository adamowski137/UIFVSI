#include "SceneManager.hpp"
#include "models/Object.hpp"
#include <cstdint>

void SceneManager::notify(const std::weak_ptr<Object> &obj) {
  for (const auto &observer :
       m_observers[reinterpret_cast<uintptr_t>(obj.lock().get())]) {
    observer.lock()->notify();
  }
}
void SceneManager::addObject(const std::weak_ptr<Object> &obj) {
  m_observers[reinterpret_cast<uintptr_t>(obj.lock().get())];
}
void SceneManager::deleteObject(const std::weak_ptr<Object> &obj) {
  m_observers.erase(
      m_observers.find(reinterpret_cast<uintptr_t>(obj.lock().get())));
}
void SceneManager::addObserver(const std::weak_ptr<Object> &obj,
                               const std::weak_ptr<Object> &observer) {
  m_observers[reinterpret_cast<uintptr_t>(obj.lock().get())].insert(observer);
}
void SceneManager::deleteObserver(const std::weak_ptr<Object> &obj,
                                  const std::weak_ptr<Object> &observer) {
  m_observers[reinterpret_cast<uintptr_t>(obj.lock().get())].erase(observer);
}
