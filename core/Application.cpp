#include "Application.hpp"

Application::Application()
    : m_window(1920, 1080, "Universal Interface for Virtual Space Interaction"),
      m_isRunning(true) {}

void Application::run() {
  while (m_isRunning) {
    m_window.update(m_isRunning);
  }
}
