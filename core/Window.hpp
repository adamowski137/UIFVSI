#pragma once

#include "Camera.hpp"
#include "InputHandler.hpp"
#include "Scene.hpp"
#include "State.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>

class GLFWwindowDeleter {
public:
  void operator()(GLFWwindow *ptr) { glfwDestroyWindow(ptr); }
};

class Window {
public:
  Window(uint16_t width, uint16_t height, std::string title);
  ~Window();

  void update(bool &running);

public:
  static void scrollInputCallback(GLFWwindow *window, double xOffset,
                                  double yOffset);
  static void keyInputCallback(GLFWwindow *window, int key, int scancode,
                               int action, int mods);
  static void cursorPositionCallback(GLFWwindow *window, double xpos,
                                     double ypos);
  static void mouseButtonCallback(GLFWwindow *window, int button, int action,
                                  int mods);
  static void resizeWindowCallback(GLFWwindow *window, int width, int height);

private:
  void renderImgui(float dt);

private:
  std::unique_ptr<GLFWwindow, GLFWwindowDeleter> m_window;
  std::shared_ptr<Renderer> m_renderer;
  std::unique_ptr<Scene> m_scene;
  std::unique_ptr<SceneManager> m_manager;

  Camera m_camera;
  State m_state;
  InputHandler m_inputHandler;

  float m_t;
};
