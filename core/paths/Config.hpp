#pragma once
#include <cstdint>
#include <utility>

class Config {
public:
  static constexpr float SCALE = 0.1f;
  static constexpr float ROUGH_BLADE_RADIUS = 8.f;
  static constexpr float BLOCK_HEIGHT = 50.f;
  static constexpr float BLOCK_WIDTH = 150.f;
  static constexpr float BLOCK_DEPTH = 150.f;
  static constexpr float BASE_HEIGHT = 15.f;
  static constexpr float SAMPLING_DISTANCE_ROUGH = .01f;
  static constexpr float SAMPLING_DISTANCE_BALL = .01f;
  static constexpr uint32_t HEIGHT_MAP_RESOLUTION = 20;
  static constexpr float BALL_BLADE_RADIUS = 4.f;
  static constexpr float FLAT_BLADE_RADIUS = 5.f;
  static constexpr float TRAVEL_CLEARANCE = BLOCK_HEIGHT;
  static constexpr float NORMAL_THRESHOLD = 0.1f;
  static std::pair<uint32_t, uint32_t>
  CoordinateToHeightMapIndex(const float x, const float z);
};
