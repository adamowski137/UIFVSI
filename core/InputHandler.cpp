#include "InputHandler.hpp"
#include "Quaternion.hpp"
#include "SceneManager.hpp"
#include "State.hpp"
#include "Transformations.hpp"
#include "Vector.hpp"
#include "render/Shader.hpp"
#include "serialize/Serializer.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <set>

void InputHandler::registerMouseClick(int key, int action, float x, float y) {
  MouseClickArgs mc{.start = m_startMouse,
                    .end = math137::Vector2f(x, y),
                    .key = key,
                    .action = action};
  m_eventQueue.emplace(mc);

  if (key == GLFW_MOUSE_BUTTON_LEFT) {
    m_leftMouse = action == GLFW_PRESS;
    m_startMouse = math137::Vector2f(x, y);
    m_prevMouse = math137::Vector2f(x, y);
  }
  if (key == GLFW_MOUSE_BUTTON_RIGHT) {
    m_rightMouse = action == GLFW_PRESS;
  }
}

void InputHandler::registerMouseMove(float x, float y) {
  if (!m_leftMouse)
    return;

  MoveArgs mm{.end = math137::Vector2f{x, y}, .start = m_prevMouse};
  m_eventQueue.emplace(mm);
  m_prevMouse = math137::Vector2f(x, y);
}

void InputHandler::registerKeyPress(int key, int action) {
  KeyPressArgs kp{.key = key, .action = action};
  m_eventQueue.emplace(kp);
  m_keyboard[key] = (action == GLFW_PRESS);
  if (action == GLFW_PRESS && key == GLFW_KEY_LEFT_CONTROL) {
      std::cout << "klikniêto" << std::endl;
  }
  if (action == GLFW_RELEASE && key == GLFW_KEY_LEFT_CONTROL) {
      std::cout << "puszczono" << std::endl;
  }
}

void InputHandler::registerMouseScroll(float dx) {
  ScrollArgs sc{.dx = dx};
  m_eventQueue.emplace(sc);
}

float InputHandler::project(float x, float y) {
  float r = 1.0f;
  float d = x * x + y * y;
  if (2 * d <= r * r) {
    return sqrtf(r * r - d);
  }
  return r * r / (2 * sqrtf(d));
}

void InputHandler::handleEvents(const std::unique_ptr<SceneManager> &manager,
                                State &state, Camera &camera, float dt) {
  while (!m_eventQueue.empty()) {
    InputEvent event = m_eventQueue.front();
    m_eventQueue.pop();

    switch (event.type) {
    case EventType::MOVE: {
      math137::Vector2f end = event.move.end;
      math137::Vector2f start = event.move.start;
      math137::Vector2f positonChange = end - start;
      if (positonChange * positonChange < 1e-6)
        break;
      switch (state.getMode()) {
      case Mode::MOVE: {
        Transformations::MoveSelected(
            manager, positonChange.x() / state.getWidth(),
            m_keyboard[GLFW_KEY_LEFT_CONTROL]
                ? 0
                : positonChange.y() / state.getHeight(),
            m_keyboard[GLFW_KEY_LEFT_CONTROL]
                ? positonChange.y() / state.getHeight()
                : 0);
        break;
      }
      case Mode::ROTATE: {
        float ndcsx = (2 * start.x()) / (state.getWidth() - 1) - 1.f;
        float ndcsy = 1.f - (2 * start.y()) / (state.getHeight() - 1);
        float ndcex = (2 * end.x()) / (state.getWidth() - 1) - 1.f;
        float ndcey = 1.f - (2 * end.y()) / (state.getHeight() - 1);
        float psz = project(ndcsx, ndcsy);
        float pez = project(ndcex, ndcey);
        Transformations::RotateSelected(
            manager, state,
            math137::Quaternion::FromVectors({ndcsx, ndcsy, psz},
                                             {ndcex, ndcey, pez}));
        break;
      }
      case Mode::SCALE: {
        Transformations::ScaleSelected(manager, state,
                                       positonChange.y() / state.getHeight());
        break;
      }
      case Mode::CAMERA: {
        camera.rotateCamera(positonChange.x(), positonChange.y());
        break;
      }
      default:
        break;
      }
      break;
    }
    case EventType::SCROLL: {
      camera.changeDistance(-event.scroll.dx);
      break;
    }
    case EventType::KEY_PRESS: {
      int key = event.keyPress.key;
      int action = event.keyPress.action;
      if (action == GLFW_RELEASE && key == GLFW_KEY_D) {
        state.setMode(Mode::DEFAULT);
      }
      if (action == GLFW_RELEASE && key == GLFW_KEY_C) {
        state.setMode(Mode::CAMERA);
      }
      if (action == GLFW_RELEASE && key == GLFW_KEY_M) {
        state.setMode(Mode::MOVE);
      }
      if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
        state.setMode(Mode::ROTATE);
      }
      if (action == GLFW_RELEASE && key == GLFW_KEY_S) {
        m_keyboard[GLFW_KEY_LEFT_CONTROL] ? Serializer::SerializeToFile("export.json", manager) : state.setMode(Mode::SCALE);
      }
      if (action == GLFW_RELEASE && key == GLFW_KEY_O) {
        state.setTransformation(Transformation::OBJECT);
      }
      if (action == GLFW_RELEASE && key == GLFW_KEY_Z) {
        state.setTransformation(Transformation::CURSOR);
      }
      if (action == GLFW_RELEASE && key == GLFW_KEY_X) {
        state.setTransformation(Transformation::MASS);
      }
      if (action == GLFW_RELEASE && key == GLFW_KEY_SPACE) {
        camera.setTarget(manager->getMassCenter());
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_UP) {
        camera.moveTarget({10 * dt, 0.f, 0.f});
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_DOWN) {
        camera.moveTarget({-10 * dt, 0.f, 0.f});
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_RIGHT) {
        camera.moveTarget({0.f, 0.f, 10 * dt});
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_LEFT) {
        camera.moveTarget({0.f, 0.f, -10 * dt});
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_COMMA) {
        camera.moveTarget({0.f, 10 * dt, 0.f});
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_PERIOD) {
        camera.moveTarget({0.f, -10 * dt, 0.f});
      }

      break;
    }
    case EventType::MOUSE_CLICK: {
      int key = event.mouseClick.key;
      int action = event.mouseClick.action;
      math137::Vector2f start = event.mouseClick.start;
      math137::Vector2f end = event.mouseClick.end;
      if (action == GLFW_RELEASE && key == GLFW_MOUSE_BUTTON_RIGHT) {
        float ndcsx = (2 * end.x()) / (state.getWidth() - 1) - 1.f;
        float ndcsy = 1.f - (2 * end.y()) / (state.getHeight() - 1);
        Transformations::SetCursor(manager, camera, ndcsx, ndcsy);
      }
      if (action == GLFW_RELEASE && key == GLFW_MOUSE_BUTTON_LEFT &&
          state.getMode() == Mode::DEFAULT) {
        uint16_t startX = fminf(end.x(), start.x());
        uint16_t startY =
            fminf(state.getHeight() - end.y(), state.getHeight() - start.y());
        uint16_t endX = fmaxf(end.x(), start.x());
        uint16_t endY =
            fmaxf(state.getHeight() - end.y(), state.getHeight() - start.y());
        int size = 10;
        uint16_t height = fmax(endY - startY, size);
        uint16_t width = fmax(endX - startX, size);
        glBindFramebuffer(GL_FRAMEBUFFER, state.fbo);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(startX - size / 2, startY - size / 2, width, height,
                     GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
                     state.stencilData.data());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        std::set<uint32_t> data;
        for (int i = 0; i < (width * height); i++) {
          uint32_t id = state.stencilData[i];
          id = ((id & 0xFF) << 24) + (((id >> 8) & 0xFF) << 16) + (((id >> 16) & 0xFF) << 8) +
               (id >> 24);
          data.insert(id);
        }
        manager->selectObjects(data, m_keyboard[GLFW_KEY_LEFT_CONTROL]);
        manager->recalculateMassCenter();
      }
      break;
    }
    default:
      break;
    }
  }
}
