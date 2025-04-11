#include "Window.hpp"
#include "GLFW/glfw3.h"
#include "MatrixUtils.hpp"
#include "Scene.hpp"
#include "Vector.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "models/Torus.hpp"
#include <GL/gl.h>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>

Window::Window(uint16_t width, uint16_t height, std::string title)
    : m_camera(1.f, {0.0f, 0.0f, -1.0f}) {
  m_state.setDimensions(width, height);
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  m_window = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>(
      glfwCreateWindow(width, height, title.c_str(), NULL, NULL));
  if (m_window.get() == NULL) {
    throw std::runtime_error("Failed to create GLFW window");
  }
  glfwMakeContextCurrent(m_window.get());
  glewInit();

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui_ImplGlfw_InitForOpenGL(m_window.get(), true);
  ImGui_ImplOpenGL3_Init();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_STENCIL_TEST);
  glStencilMask(0xff);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glfwSetWindowUserPointer(m_window.get(), reinterpret_cast<void *>(this));
  glfwSetScrollCallback(m_window.get(), scrollInputCallback);
  glfwSetKeyCallback(m_window.get(), keyInputCallback);
  glfwSetMouseButtonCallback(m_window.get(), mouseButtonCallback);
  glfwSetCursorPosCallback(m_window.get(), cursorPositionCallback);
  glfwSetFramebufferSizeCallback(m_window.get(), resizeWindowCallback);
  m_scene = std::make_unique<Scene>();
  m_renderer = std::make_shared<Renderer>();
  m_renderer->setProjection(math137::MatrixUtils::Projection(
      m_fov, (float)width / (float)height, 0.1f, 100.f));
  m_scene->setInvProjection(math137::MatrixUtils::InvProjection(
      m_fov, (float)width / (float)height, 0.1f, 100.f));
  m_scene->addObject(std::make_shared<Torus>(0.5f, 0.1f));
}

Window::~Window() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
}

void Window::update(bool &running) {

  running = !glfwWindowShouldClose(m_window.get());

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  m_renderer->setView(m_camera.getView());
  m_scene->render(m_renderer);

  float t = glfwGetTime();
  renderImgui(t - m_t);
  m_t = t;
  glfwSwapBuffers(m_window.get());
  glfwPollEvents();
  m_inputHandler.handleEvents(m_scene, m_state, m_camera);
}

void Window::renderImgui(float dt) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  m_scene->renderMenu();
  ImGui::Text("%d frames", (int)(1 / dt));
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Window::keyInputCallback(GLFWwindow *window, int key, int scancode,
                              int action, int mods) {
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  w->m_inputHandler.registerKeyPress(key, action);
}

void Window::cursorPositionCallback(GLFWwindow *window, double xpos,
                                    double ypos) {
  ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  w->m_inputHandler.registerMouseMove(xpos, ypos);
}

void Window::mouseButtonCallback(GLFWwindow *window, int button, int action,
                                 int mods) {
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
  if (ImGui::GetIO().WantCaptureMouse)
    return;
  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  w->m_inputHandler.registerMouseClick(button, action, x, y);
}

void Window::scrollInputCallback(GLFWwindow *window, double xOffset,
                                 double yOffset) {

  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  w->m_camera.changeDistance(yOffset);
}
void Window::resizeWindowCallback(GLFWwindow *window, int width, int height) {

  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  w->m_state.setDimensions(width, height);
  glViewport(0, 0, width, height);
  w->m_renderer->setProjection(math137::MatrixUtils::Projection(
      w->m_fov, (float)width / height, 0.1f, 100.f));
  w->m_scene->setInvProjection(math137::MatrixUtils::InvProjection(
      w->m_fov, (float)width / height, 0.1f, 100.f));
}
