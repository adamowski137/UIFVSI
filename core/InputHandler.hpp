#pragma once

#include "Camera.hpp"
#include "SceneManager.hpp"
#include "State.hpp"
#include "Vector.hpp"
#include <array>
#include <cstring>
#include <memory>
#include <queue>

enum class EventType { NOACTION, MOVE, MOUSE_CLICK, KEY_PRESS, SCROLL };

struct MoveArgs {
  math137::Vector2f end;
  math137::Vector2f start;
};
struct MouseClickArgs {
  math137::Vector2f start;
  math137::Vector2f end;
  int key;
  int action;
};
struct KeyPressArgs {
  int key;
  int action;
};
struct ScrollArgs {
  float dx;
};

union InputArgs {
	InputArgs() { memset(this, 0, sizeof(*this)); }
	InputArgs(MoveArgs mv) { memset(this, 0, sizeof(*this)); move = mv; }
	InputArgs(MouseClickArgs mv) { memset(this, 0, sizeof(*this)); mouseClick = mv; }
	InputArgs(KeyPressArgs mv) { memset(this, 0, sizeof(*this)); keyPress = mv; }
	InputArgs(ScrollArgs mv) { memset(this, 0, sizeof(*this)); scroll = mv; }

  MoveArgs move;
  MouseClickArgs mouseClick;
  KeyPressArgs keyPress;
  ScrollArgs scroll;
};

struct InputEvent {
  InputEvent() { memset(this, 0, sizeof(*this)); }
  InputEvent(MoveArgs mv)
      : type(EventType::MOVE), args(std::make_unique<InputArgs>(mv)) {}
  InputEvent(MouseClickArgs mv)
      : type(EventType::MOUSE_CLICK), args(std::make_unique<InputArgs>(mv)) {}
  InputEvent(KeyPressArgs mv) : type(EventType::KEY_PRESS), args(std::make_unique<InputArgs>(mv)) {}
  InputEvent(ScrollArgs mv) : type(EventType::SCROLL), args(std::make_unique<InputArgs>(mv)) {}
  std::unique_ptr<InputArgs> args;
  EventType type;
};


class InputHandler {
public:
  InputHandler() : m_startMouse(0.0f, 0.0f), m_prevMouse(0.0f, 0.0f),
    m_leftMouse(false), m_rightMouse(false), m_keyboard() {
	  m_keyboard.fill(false);
  }
  InputHandler(const InputHandler&) = delete;
  ~InputHandler() = default;

  void registerMouseClick(int key, int action, float x, float y);
  void registerMouseMove(float dx, float dy);
  void registerKeyPress(int key, int action);
  void registerMouseScroll(float dx);

  void handleEvents(const std::unique_ptr<SceneManager> &manager, State &state,
                    Camera &camera, float dt);

private:
  float project(float x, float y);
  std::queue<InputEvent> m_eventQueue;

  math137::Vector2f m_startMouse;
  math137::Vector2f m_prevMouse;
  bool m_leftMouse;
  bool m_rightMouse;
  std::array<bool, 349> m_keyboard;
};
