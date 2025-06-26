#include "Window.hpp"
#include "GLFW/glfw3.h"
#include "MatrixUtils.hpp"
#include "Scene.hpp"
#include "Vector.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "render/Shader.hpp"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#define DEBUG 0

GLenum glCheckError_(const char *file, int line) {
  GLenum errorCode;
  while ((errorCode = glGetError()) != GL_NO_ERROR) {
    std::string error;
    switch (errorCode) {
    case GL_INVALID_ENUM:
      error = "INVALID_ENUM";
      break;
    case GL_INVALID_VALUE:
      error = "INVALID_VALUE";
      break;
    case GL_INVALID_OPERATION:
      error = "INVALID_OPERATION";
      break;
    case GL_STACK_OVERFLOW:
      error = "STACK_OVERFLOW";
      break;
    case GL_STACK_UNDERFLOW:
      error = "STACK_UNDERFLOW";
      break;
    case GL_OUT_OF_MEMORY:
      error = "OUT_OF_MEMORY";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      error = "INVALID_FRAMEBUFFER_OPERATION";
      break;
    }
    std::cout << error << " | " << file << " (" << line << ")" << std::endl;
  }
  return errorCode;
}

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id,
                            GLenum severity, GLsizei length,
                            const char *message, const void *userParam) {
  // ignore non-significant error/warning codes
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
    return;

  std::cout << "---------------" << std::endl;
  std::cout << "Debug message (" << id << "): " << message << std::endl;

  switch (source) {
  case GL_DEBUG_SOURCE_API:
    std::cout << "Source: API";
    break;
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
    std::cout << "Source: Window System";
    break;
  case GL_DEBUG_SOURCE_SHADER_COMPILER:
    std::cout << "Source: Shader Compiler";
    break;
  case GL_DEBUG_SOURCE_THIRD_PARTY:
    std::cout << "Source: Third Party";
    break;
  case GL_DEBUG_SOURCE_APPLICATION:
    std::cout << "Source: Application";
    break;
  case GL_DEBUG_SOURCE_OTHER:
    std::cout << "Source: Other";
    break;
  }
  std::cout << std::endl;

  switch (type) {
  case GL_DEBUG_TYPE_ERROR:
    std::cout << "Type: Error";
    break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    std::cout << "Type: Deprecated Behaviour";
    break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    std::cout << "Type: Undefined Behaviour";
    break;
  case GL_DEBUG_TYPE_PORTABILITY:
    std::cout << "Type: Portability";
    break;
  case GL_DEBUG_TYPE_PERFORMANCE:
    std::cout << "Type: Performance";
    break;
  case GL_DEBUG_TYPE_MARKER:
    std::cout << "Type: Marker";
    break;
  case GL_DEBUG_TYPE_PUSH_GROUP:
    std::cout << "Type: Push Group";
    break;
  case GL_DEBUG_TYPE_POP_GROUP:
    std::cout << "Type: Pop Group";
    break;
  case GL_DEBUG_TYPE_OTHER:
    std::cout << "Type: Other";
    break;
  }
  std::cout << std::endl;

  switch (severity) {
  case GL_DEBUG_SEVERITY_HIGH:
    std::cout << "Severity: high";
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    std::cout << "Severity: medium";
    break;
  case GL_DEBUG_SEVERITY_LOW:
    std::cout << "Severity: low";
    break;
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    std::cout << "Severity: notification";
    break;
  }
  std::cout << std::endl;
  std::cout << std::endl;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)

Window::Window(uint16_t width, uint16_t height, std::string title)
    : m_camera(1.f, {0.0f, 0.0f, 0.0f}) {
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
#if DEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(glDebugOutput, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                        GL_TRUE);

#endif // 0
  glfwSetWindowUserPointer(m_window.get(), reinterpret_cast<void *>(this));
  glfwSetScrollCallback(m_window.get(), scrollInputCallback);
  glfwSetKeyCallback(m_window.get(), keyInputCallback);
  glfwSetMouseButtonCallback(m_window.get(), mouseButtonCallback);
  glfwSetCursorPosCallback(m_window.get(), cursorPositionCallback);
  glfwSetFramebufferSizeCallback(m_window.get(), resizeWindowCallback);
  m_renderer = std::make_shared<Renderer>();
  m_scene = std::make_unique<Scene>();
  m_manager = std::make_unique<SceneManager>();
  m_renderer->setProjection(math137::MatrixUtils::Projection(
      m_state.m_fov, (float)width / (float)height, m_state.m_near,
      m_state.m_far));
  m_manager->setInvProjection(math137::MatrixUtils::InvProjection(
      m_state.m_fov, (float)width / (float)height, m_state.m_near,
      m_state.m_far));
  glGenTextures(1, &m_state.textureId);
  glBindTexture(GL_TEXTURE_2D, m_state.textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_INT_8_8_8_8, nullptr);

  glGenFramebuffers(1, &m_state.fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, m_state.fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         m_state.textureId, 0);
  glGenRenderbuffers(1, &m_state.rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, m_state.rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, m_state.rbo);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Window::~Window() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
}

void Window::update(bool &running) {

  running = !glfwWindowShouldClose(m_window.get());

  m_renderer->setView(m_camera.getView());
  m_manager->notifyQueue();
  m_scene->renderToFramebuffer(m_renderer, m_manager, m_camera, m_state);
  m_scene->render(m_renderer, m_manager, m_state);

  float t = glfwGetTime();
  renderImgui(t - m_t);

  glfwSwapBuffers(m_window.get());
  glfwPollEvents();
  m_inputHandler.handleEvents(m_manager, m_state, m_camera, t - m_t);
  m_t = t;
}

void Window::renderImgui(float dt) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  m_scene->renderMenu(m_manager, m_state);
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
  ImGui_ImplGlfw_ScrollCallback(window, xOffset, yOffset);
  if (ImGui::GetIO().WantCaptureMouse)
    return;
  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  w->m_camera.changeDistance(0.05f * yOffset);
}
void Window::resizeWindowCallback(GLFWwindow *window, int width, int height) {

  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  w->m_state.setDimensions(width, height);
  glViewport(0, 0, width, height);
  w->m_state.setProjection(
      math137::MatrixUtils::Projection(w->m_state.m_fov, (float)width / height,
                                       w->m_state.m_near, w->m_state.m_far));
  w->m_renderer->setProjection(w->m_state.getProjection());
  w->m_manager->setInvProjection(math137::MatrixUtils::InvProjection(
      w->m_state.m_fov, (float)width / height, w->m_state.m_near,
      w->m_state.m_far));
}
