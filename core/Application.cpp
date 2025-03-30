#include "Application.hpp"

Application::Application()
    : m_window(1024, 768, "Universal Interface for Virtual Space Interaction"),
      m_isRunning(true) {}

void Application::run() {
  while (m_isRunning) {
    m_window.update(m_isRunning);
  }
}
