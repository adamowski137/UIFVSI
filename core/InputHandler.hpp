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

struct InputEvent {
  InputEvent() { memset(this, 0, sizeof(*this)); }
  InputEvent(MoveArgs mv) : type(EventType::MOVE), move(mv) {}
  InputEvent(MouseClickArgs mv)
      : type(EventType::MOUSE_CLICK), mouseClick(mv) {}
  InputEvent(KeyPressArgs mv) : type(EventType::KEY_PRESS), keyPress(mv) {}
  InputEvent(ScrollArgs mv) : type(EventType::SCROLL), scroll(mv) {}
  EventType type;
  MoveArgs move;
  MouseClickArgs mouseClick;
  KeyPressArgs keyPress;
  ScrollArgs scroll;
};

class InputHandler {
public:
  void registerMouseClick(int key, int action, float x, float y);
  void registerMouseMove(float dx, float dy);
  void registerKeyPress(int key, int action);
  void registerMouseScroll(float dx);

  void handleEvents(const std::unique_ptr<SceneManager> &manager, State &state,
                    Camera &camera, float dt);

private:
  float project(float x, float y);
  std::queue<InputEvent> m_eventQueue;

  void handleMouseClick();
  math137::Vector2f m_startMouse;
  math137::Vector2f m_prevMouse;
  bool m_leftMouse;
  bool m_rightMouse;
  std::array<bool, 349> m_keyboard;
};
