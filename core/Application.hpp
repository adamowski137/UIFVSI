#pragma once

#include "Window.hpp"

class Application {
public:
  Application();

  void run();

private:
  bool m_isRunning;
  Window m_window;
};
