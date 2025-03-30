#pragma once

#include "Camera.hpp"
#include "Matrix.hpp"
#include "Shader.hpp"
#include "Vector.hpp"
#include "imgui.h"
#include "models/Cursor.hpp"
#include "models/Line.hpp"
#include "models/Object.hpp"
#include "models/Point.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <vector>

class GLFWwindowDeleter {
public:
  void operator()(GLFWwindow *ptr) { glfwDestroyWindow(ptr); }
};

enum class Mode { DEFAULT, MOVE, ROTATE, SCALE, CURSOR };
enum class Tranformation { OBJECT, MASS, CURSOR };

class Window {
public:
  Window(uint16_t width, uint16_t height, std::string title);
  ~Window();

  void update(bool &running);

public:
  static void keyInputCallback(GLFWwindow *window, int key, int scancode,
                               int action, int mods);
  static void cursorPositionCallback(GLFWwindow *window, double xpos,
                                     double ypos);
  static void mouseButtonCallback(GLFWwindow *window, int button, int action,
                                  int mods);
  static void resizeWindowCallback(GLFWwindow *window, int width, int height);
  static int inputTextCallback(ImGuiInputTextCallbackData *data);
  static bool InputText(const char *label, std::string &str,
                        ImGuiInputTextFlags flags = 0);

private:
  void renderImgui(float dt);
  void setVertexData();
  float project(float x, float y);
  void calculateSelectedMassCenter();

private:
  std::unique_ptr<GLFWwindow, GLFWwindowDeleter> m_window;
  std::unique_ptr<Shader> m_baseShader;
  std::unique_ptr<Shader> m_pointShader;
  Camera m_camera;

  std::vector<std::shared_ptr<Point>> m_points;
  std::vector<std::shared_ptr<Object>> m_objects;
  std::vector<std::shared_ptr<Line>> m_lines;
  std::set<uint16_t> m_selectedObjects;
  std::set<uint16_t> m_selectedPoints;
  std::set<uint16_t> m_selectedLines;
  std::unique_ptr<Point> m_massCenter;
  std::unique_ptr<Cursor> m_cursor;

  Mode m_mode;
  Tranformation m_transformation;
  bool m_activeMode;
  bool m_alternativeMode;
  float m_t;

  math137::Vector3f m_prevMose;
  math137::Matrix4f m_projection;
  math137::Matrix4f m_view;
  std::vector<uint8_t> m_stencilData;

  uint16_t m_width;
  uint16_t m_height;

  const float m_fov = (M_PI_4);
  const math137::Vector4f m_defaultColor =
      math137::Vector4f(1.f, 1.f, 0.f, 1.f);
  const math137::Vector4f m_selectedColor =
      math137::Vector4f(1.f, 0.2f, 0.2f, 1.f);
  const math137::Vector4f m_centerColor = math137::Vector4f(1.f, 1.f, 1.f, 1.f);
};
