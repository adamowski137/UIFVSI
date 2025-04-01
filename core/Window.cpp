#include "Window.hpp"
#include "GLFW/glfw3.h"
#include "Matrix.hpp"
#include "MatrixUtils.hpp"
#include "Shader.hpp"
#include "Vector.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "models/Line.hpp"
#include "models/Point.hpp"
#include "models/Torus.hpp"
#include <GL/gl.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>

Window::Window(uint16_t width, uint16_t height, std::string title)
    : m_height(height), m_width(width), m_camera(1.f, {0.0f, 0.0f, -1.0f}),
      m_mode(Mode::DEFAULT) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  m_window = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>(
      glfwCreateWindow(m_width, m_height, title.c_str(), NULL, NULL));
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
  m_baseShader =
      std::make_unique<Shader>("../shaders/base.vs", "../shaders/base.fs");
  m_pointShader =
      std::make_unique<Shader>("../shaders/point.vs", "../shaders/point.fs");

  m_baseShader->use();
  m_projection = math137::MatrixUtils::Projection(
      m_fov, (float)m_width / (float)m_height, 0.1f, 100.f);
  m_invprojection = math137::MatrixUtils::InvProjection(
      m_fov, (float)m_width / (float)m_height, 0.1f, 100.f);
  m_baseShader->setMat4("projection", m_projection);
  m_pointShader->use();
  m_pointShader->setMat4("projection", m_projection);
  m_massCenter = std::make_unique<Point>();
  m_cursor = std::make_unique<Cursor>();
  m_cursor->setPosition({0.0f, 0.0f, -1.0f});
  m_stencilData.resize(m_width * m_height);
  m_ground = std::make_unique<Ground>();
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

  m_baseShader->use();
  m_baseShader->setMat4("view", m_camera.getView());
  m_baseShader->setVec4("color", m_centerColor);
  m_ground->render();
  m_baseShader->setMat4("model", m_cursor->getModel());
  m_cursor->render();
  m_baseShader->setVec4("color", m_selectedColor);
  for (const auto idx : m_selectedObjects) {
    glStencilFunc(GL_ALWAYS, (uint8_t)(idx + 1), 0xff);
    m_baseShader->setMat4("model", m_objects[idx]->getModel());
    m_objects[idx]->render();
  }

  m_baseShader->setVec4("color", m_defaultColor);

  for (uint16_t i = 0; i < m_objects.size(); i++) {
    if (m_selectedObjects.find(i) != m_selectedObjects.end())
      continue;
    glStencilFunc(GL_ALWAYS, (uint8_t)(i + 1), 0xff);
    m_baseShader->setMat4("model", m_objects[i]->getModel());
    m_objects[i]->render();
  }

  m_baseShader->setMat4("model", math137::MatrixUtils::Identity());
  m_baseShader->setVec4("color", m_selectedColor);

  for (const auto idx : m_selectedLines) {
    glStencilFunc(GL_ALWAYS, (uint8_t)(m_objects.size() + idx + 1), 0xff);
    m_lines[idx]->render();
  }

  m_baseShader->setVec4("color", m_defaultColor);
  for (uint16_t i = 0; i < m_lines.size(); i++) {
    if (m_selectedLines.find(i) != m_selectedLines.end())
      continue;
    glStencilFunc(GL_ALWAYS, (uint8_t)(m_objects.size() + i + 1), 0xff);
    m_lines[i]->render();
  }
  m_pointShader->use();
  m_pointShader->setVec4("color", m_selectedColor);
  m_pointShader->setMat4("view", m_camera.getView());
  for (const auto idx : m_selectedPoints) {
    glStencilFunc(GL_ALWAYS,
                  (uint8_t)(m_objects.size() + m_lines.size() + idx + 1), 0xff);
    m_pointShader->setMat4("model", m_points[idx]->getModel());
    m_points[idx]->render();
  }
  m_pointShader->setVec4("color", m_defaultColor);
  for (uint16_t i = 0; i < m_points.size(); i++) {
    if (m_selectedPoints.find(i) != m_selectedPoints.end())
      continue;
    glStencilFunc(GL_ALWAYS,
                  (uint8_t)(m_objects.size() + m_lines.size() + i + 1), 0xff);
    m_pointShader->setMat4("model", m_points[i]->getModel());
    m_points[i]->render();
  }
  m_pointShader->setVec4("color", m_centerColor);
  m_pointShader->setMat4("model", m_massCenter->getModel());
  m_massCenter->render();

  float t = glfwGetTime();
  renderImgui(t - m_t);
  m_t = t;
  glfwSwapBuffers(m_window.get());
  glfwPollEvents();
}

float Window::project(float x, float y) {
  float r = 1.0f;
  float d = x * x + y * y;
  if (2 * d <= r * r) {
    return sqrtf(r * r - d);
  }
  return r * r / (2 * sqrtf(d));
}

void Window::calculateSelectedMassCenter() {
  math137::Vector3f val;
  if (m_selectedObjects.size() + m_selectedPoints.size() == 0) {
    m_massCenter->setPosition(val);
  }
  for (const auto idx : m_selectedObjects) {
    val = val + m_objects[idx]->getTranslation();
  }
  for (const auto idx : m_selectedPoints) {
    val = val + m_points[idx]->getPosition();
  }

  m_massCenter->setPosition(
      val * (1.0f / (m_selectedPoints.size() + m_selectedObjects.size())));
}

void Window::setCursorPos(int x, int y) {
  float ndcX = (2.f * x) / (m_width - 1) - 1.f;
  float ndcY = 1.f - (2.f * y) / (m_height - 1);
  math137::Vector3f dist = (m_cursor->getPosition() - m_camera.getPosition());
  float l = sqrtf(dist * dist);

  math137::Vector4f clipNear = {ndcX, ndcY, -1.f, 1.f};
  math137::Vector4f clipFar = {ndcX, ndcY, 1.f, 1.f};

  math137::Matrix4f invView = m_camera.getInverseView();
  math137::Matrix4f invProj = m_invprojection;

  math137::Vector4f viewNear = invProj * clipNear;
  math137::Vector4f viewFar = invProj * clipFar;
  viewNear = viewNear * (1 / viewNear.w());
  viewFar = viewFar * (1 / viewFar.w());

  math137::Vector4f ray = viewFar - viewNear;
  ray.normalize();

  math137::Vector4f worldRay = invView * ray;

  m_cursor->setPosition(
      m_camera.getPosition() +
      math137::Vector3f{worldRay.x() * l, worldRay.y() * l, worldRay.z() * l});
}

void Window::renderImgui(float dt) {
  static int selectedMode = 0;
  static int selectedTrans = 0;
  const char *modes[] = {"Default", "Move", "Rotate", "Scale", "Cursor"};
  const char *tranformations[] = {"Object", "Mass", "Cursor"};
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::Text("Select Mode");
  if (ImGui::Combo("##Mode", &selectedMode, modes, IM_ARRAYSIZE(modes))) {
    m_mode = static_cast<Mode>(selectedMode);
  }
  ImGui::Text("Select Transformation Center");
  if (ImGui::Combo("##Trans", &selectedTrans, tranformations,
                   IM_ARRAYSIZE(tranformations))) {
    m_transformation = static_cast<Tranformation>(selectedTrans);
  }
  if (ImGui::TreeNodeEx("Items", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::TreeNodeEx("Objects", ImGuiTreeNodeFlags_None)) {
      for (int i = 0; i < m_objects.size(); i++) {
        auto it = m_selectedObjects.find(i);
        ImGuiTreeNodeFlags flags =
            it == m_selectedObjects.end()
                ? ImGuiTreeNodeFlags_OpenOnArrow
                : ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Selected;
        bool open = ImGui::TreeNodeEx((m_objects[i]->name).c_str(), flags);
        if (ImGui::IsItemClicked()) {
          if (it != m_selectedObjects.end()) {
            m_selectedObjects.erase(it);
          } else {
            m_selectedObjects.insert(i);
          }
          calculateSelectedMassCenter();
        }

        if (open) {
          m_objects[i]->setNameMenu();
          ImGui::Text("pos: %f, %f, %f", m_objects[i]->getTranslation().x(),
                      m_objects[i]->getTranslation().y(),
                      m_objects[i]->getTranslation().z());
          ImGui::TreePop();
        }
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Lines", ImGuiSelectableFlags_None)) {
      for (uint16_t i = 0; i < m_lines.size(); i++) {
        auto it = m_selectedLines.find(i);
        ImGuiTreeNodeFlags flags =
            it == m_selectedLines.end()
                ? ImGuiTreeNodeFlags_OpenOnArrow
                : ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Selected;
        bool open = ImGui::TreeNodeEx((m_lines[i]->name).c_str(), flags);
        if (ImGui::IsItemClicked()) {
          if (it != m_selectedLines.end()) {
            m_selectedLines.erase(it);
          } else {
            m_selectedLines.insert(i);
          }
        }
        if (open) {
          m_lines[i]->setNameMenu();
          m_lines[i]->renderObjectMenu();
          ImGui::TreePop();
        }
      }
      if (ImGui::Button("Add points")) {
        for (const auto l : m_selectedLines) {
          for (const auto p : m_selectedPoints) {
            m_lines[l]->addPoint(m_points[p]);
          }
        }
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Points", ImGuiSelectableFlags_None)) {
      for (uint16_t i = 0; i < m_points.size(); i++) {
        auto it = m_selectedPoints.find(i);
        if (ImGui::Selectable((m_points[i]->name).c_str(),
                              it != m_selectedPoints.end(),
                              ImGuiSelectableFlags_None)) {
          if (it != m_selectedPoints.end()) {
            m_selectedPoints.erase(it);
          } else {
            m_selectedPoints.insert(i);
          }
          calculateSelectedMassCenter();
        }
      }
      ImGui::TreePop();
    }
    ImGui::TreePop();
  }

  ImGui::Separator();
  if (ImGui::Button("Add torus")) {
    m_objects.push_back(std::shared_ptr<Object>(new Torus(0.5f, 0.1f)));
    m_objects[m_objects.size() - 1]->setTranslation(m_cursor->getPosition());
  }
  ImGui::SameLine();
  if (ImGui::Button("Add point")) {
    m_points.push_back(std::shared_ptr<Point>(new Point()));
    m_points[m_points.size() - 1]->setPosition(m_cursor->getPosition());
  }
  ImGui::SameLine();
  if (ImGui::Button("Add line")) {
    m_lines.push_back(std::shared_ptr<Line>(new Line()));
    for (const auto p : m_selectedPoints) {
      m_lines[m_lines.size() - 1]->addPoint(m_points[p]);
    }
  }

  if (ImGui::Button("Delete Selected")) {
    if (m_selectedObjects.size() == m_objects.size()) {
      m_selectedObjects.clear();
      m_objects.clear();
    }
    if (m_selectedPoints.size() == m_points.size()) {
      m_selectedPoints.clear();
      m_points.clear();
    }
    if (m_selectedLines.size() == m_lines.size()) {
      m_selectedLines.clear();
      m_lines.clear();
    }

    if (m_selectedObjects.size() != 0) {
      std::vector<std::shared_ptr<Object>> newObjects;
      newObjects.reserve(m_objects.size() - m_selectedObjects.size());
      for (int i = 0; i < m_objects.size(); i++) {
        if (m_selectedObjects.find(i) == m_selectedObjects.end())
          newObjects.push_back(m_objects[i]);
      }
      m_selectedObjects.clear();
      m_objects = std::move(newObjects);
    }
    if (m_selectedPoints.size() != 0) {
      std::vector<std::shared_ptr<Point>> newPoints;
      newPoints.reserve(m_points.size() - m_selectedPoints.size());
      for (int i = 0; i < m_points.size(); i++) {
        if (m_selectedPoints.find(i) == m_selectedPoints.end())
          newPoints.push_back(m_points[i]);
      }
      m_selectedPoints.clear();
      m_points = std::move(newPoints);
    }
    if (m_selectedLines.size() != 0) {
      std::vector<std::shared_ptr<Line>> newObjects;
      newObjects.reserve(m_lines.size() - m_selectedLines.size());
      for (int i = 0; i < m_objects.size(); i++) {
        if (m_selectedLines.find(i) == m_selectedLines.end())
          newObjects.push_back(m_lines[i]);
      }
      m_selectedLines.clear();
      m_lines = std::move(newObjects);
    }
  }
  static float pos[3] = {};
  pos[0] = m_cursor->getPosition().x();
  pos[1] = m_cursor->getPosition().y();
  pos[2] = m_cursor->getPosition().z();

  if (ImGui::InputFloat3("Position", pos, "%.3f")) {
    m_cursor->setPosition({pos[0], pos[1], pos[2]});
  }

  ImGui::Separator();

  for (const auto id : m_selectedObjects) {
    m_objects[id]->renderObjectMenu();
  }

  ImGui::Text("%d frames", (int)(1 / dt));
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Window::keyInputCallback(GLFWwindow *window, int key, int scancode,
                              int action, int mods) {
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) {
    w->m_alternativeMode = true;
  }
  if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) {
    w->m_alternativeMode = false;
  }

  if (key == GLFW_KEY_W && action == GLFW_PRESS) {
    w->m_camera.rotateCamera(0, 0.1f);
  }
  if (key == GLFW_KEY_S && action == GLFW_PRESS) {
    w->m_camera.rotateCamera(0, -0.1f);
  }
  if (key == GLFW_KEY_A && action == GLFW_PRESS) {
    w->m_camera.rotateCamera(-0.1f, 0);
  }
  if (key == GLFW_KEY_D && action == GLFW_PRESS) {
    w->m_camera.rotateCamera(0.1f, 0);
  }
}

void Window::cursorPositionCallback(GLFWwindow *window, double xpos,
                                    double ypos) {
  ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  if (!w->m_activeMode)
    return;

  if (w->m_mode == Mode::DEFAULT)
    return;

  float px = (2 * xpos - w->m_width - 1) / w->m_width;
  float py = (2 * ypos - w->m_height - 1) / w->m_height;
  float pz = 0;
  if (w->m_mode == Mode::CURSOR) {
    w->m_cursor->move({px - w->m_prevMose.x(),
                       !w->m_alternativeMode ? w->m_prevMose.y() - py : 0.f,
                       w->m_alternativeMode ? w->m_prevMose.y() - py : 0.f});
  }
  if (w->m_mode == Mode::MOVE) {
    std::for_each(w->m_selectedObjects.begin(), w->m_selectedObjects.end(),
                  [&w, px, py](int v) {
                    w->m_objects[v]->move(
                        {px - w->m_prevMose.x(),
                         !w->m_alternativeMode ? w->m_prevMose.y() - py : 0.f,
                         w->m_alternativeMode ? w->m_prevMose.y() - py : 0.f});
                  });
    std::for_each(w->m_selectedPoints.begin(), w->m_selectedPoints.end(),
                  [&w, px, py](int v) {
                    w->m_points[v]->move(
                        {px - w->m_prevMose.x(),
                         !w->m_alternativeMode ? w->m_prevMose.y() - py : 0.f,
                         w->m_alternativeMode ? w->m_prevMose.y() - py : 0.f});
                  });

    std::for_each(w->m_lines.begin(), w->m_lines.end(),
                  [](const std::shared_ptr<Line> &l) { l->setVertexData(); });
    w->calculateSelectedMassCenter();
  }
  if (w->m_mode == Mode::ROTATE) {
    pz = w->project(px, py);
    std::for_each(w->m_selectedObjects.begin(), w->m_selectedObjects.end(),
                  [&w, px, py, pz](int v) {
                    math137::Vector3f rotationCenter =
                        w->m_transformation == Tranformation::CURSOR
                            ? w->m_cursor->getPosition()
                        : w->m_transformation == Tranformation::OBJECT
                            ? w->m_objects[v]->getTranslation()
                            : w->m_massCenter->getPosition();
                    w->m_objects[v]->rotate(
                        math137::Quaternion::FromVectors(
                            {px, py, pz}, {w->m_prevMose.x(), w->m_prevMose.y(),
                                           w->m_prevMose.z()}),
                        rotationCenter);
                  });
    std::for_each(w->m_selectedPoints.begin(), w->m_selectedPoints.end(),
                  [&w, px, py, pz](int v) {
                    math137::Vector3f rotationCenter =
                        w->m_transformation == Tranformation::CURSOR
                            ? w->m_cursor->getPosition()
                        : w->m_transformation == Tranformation::OBJECT
                            ? w->m_points[v]->getPosition()
                            : w->m_massCenter->getPosition();
                    w->m_points[v]->rotate(
                        math137::Quaternion::FromVectors(
                            {px, py, pz}, {w->m_prevMose.x(), w->m_prevMose.y(),
                                           w->m_prevMose.z()}),
                        rotationCenter);
                  });
    std::for_each(w->m_lines.begin(), w->m_lines.end(),
                  [](const std::shared_ptr<Line> &l) { l->setVertexData(); });
    w->calculateSelectedMassCenter();
  }
  if (w->m_mode == Mode::SCALE) {
    std::for_each(w->m_selectedObjects.begin(), w->m_selectedObjects.end(),
                  [&w, py](uint16_t v) {
                    math137::Vector3f rotationCenter =
                        w->m_transformation == Tranformation::CURSOR
                            ? w->m_cursor->getPosition()
                        : w->m_transformation == Tranformation::OBJECT
                            ? w->m_objects[v]->getTranslation()
                            : w->m_massCenter->getPosition();
                    w->m_objects[v]->scale(1 + (w->m_prevMose.y() - py),
                                           rotationCenter);
                  });
    w->calculateSelectedMassCenter();
  }
  w->m_prevMose = {px, py, pz};
}

void Window::mouseButtonCallback(GLFWwindow *window, int button, int action,
                                 int mods) {
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
  if (ImGui::GetIO().WantCaptureMouse)
    return;
  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    w->setCursorPos(x, y);
  }
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    w->m_activeMode = true;
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    float px = (2 * x - w->m_width) / w->m_width;
    float py = (2 * y - w->m_height) / w->m_height;
    float pz = w->project(px, py);
    w->m_prevMose = math137::Vector3f{px, py, pz};
  }
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    if (w->m_mode == Mode::DEFAULT) {
      double x, y;
      glfwGetCursorPos(window, &x, &y);
      y = w->m_height - y;
      uint16_t px = (w->m_prevMose.x() * w->m_width + w->m_width) / 2;
      uint16_t py =
          w->m_height - (w->m_prevMose.y() * w->m_height + w->m_height) / 2;
      uint16_t startX = fmin(x, (double)px);
      uint16_t startY = fmin(y, (double)py);
      uint16_t endX = fmax(x, (double)px);
      uint16_t endY = fmax(y, (double)py);
      int size = 10;
      uint16_t height = fmax(endY - startY, size);
      uint16_t width = fmax(endX - startX, size);
      glReadPixels(startX - size / 2, startY - size / 2, width, height,
                   GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, w->m_stencilData.data());
      std::set<uint8_t> vert(w->m_stencilData.begin(),
                             w->m_stencilData.begin() + width * height);
      for (uint8_t stencilValue : vert) {
        if (stencilValue == 0)
          continue;
        if (stencilValue <= w->m_objects.size()) {
          auto it = w->m_selectedObjects.find(stencilValue - 1);
          if (it == w->m_selectedObjects.end())
            w->m_selectedObjects.insert(stencilValue - 1);
          else
            w->m_selectedObjects.erase(it);
        } else if (stencilValue <= w->m_lines.size() + w->m_objects.size()) {
          stencilValue -= w->m_objects.size();
          auto it = w->m_selectedLines.find(stencilValue - 1);
          if (it == w->m_selectedLines.end())
            w->m_selectedLines.insert(stencilValue - 1);
          else
            w->m_selectedLines.erase(it);
        } else {
          stencilValue -= w->m_objects.size() + w->m_lines.size();
          auto it = w->m_selectedPoints.find(stencilValue - 1);
          if (it == w->m_selectedPoints.end())
            w->m_selectedPoints.insert(stencilValue - 1);
          else
            w->m_selectedPoints.erase(it);
        }
      }
      w->calculateSelectedMassCenter();
    }
    w->m_activeMode = false;
  }
}

void Window::scrollInputCallback(GLFWwindow *window, double xOffset,
                                 double yOffset) {

  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  w->m_camera.changeDistance(yOffset);
}
void Window::resizeWindowCallback(GLFWwindow *window, int width, int height) {

  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  w->m_width = width;
  w->m_height = height;
  w->m_projection = math137::MatrixUtils::Projection(
      w->m_fov, (float)w->m_width / (float)w->m_height, 0.1f, 100.f);
  w->m_invprojection = math137::MatrixUtils::InvProjection(
      w->m_fov, (float)w->m_width / (float)w->m_height, 0.1f, 100.f);
  w->m_baseShader->use();
  w->m_baseShader->setMat4("projection", w->m_projection);
  w->m_pointShader->use();
  w->m_pointShader->setMat4("projection", w->m_projection);
  glViewport(0, 0, width, height);
  w->m_stencilData.resize(w->m_width * w->m_height);
}
