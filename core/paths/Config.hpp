#pragma once
#include <utility>
#include <cstdint>

class Config {
public:
    static constexpr float ROUGH_BLADE_RADIUS = 8.f;
    static constexpr float BLOCK_HEIGHT = 50.f;
    static constexpr float BLOCK_WIDTH = 150.f;
    static constexpr float BLOCK_DEPTH = 150.f;
    static constexpr float BASE_HEIGHT = 15.f;
    static constexpr float SAMPLING_DISTANCE = .01f;
    static constexpr uint32_t HEIGHT_MAP_RESOLUTION = 30;
    static constexpr float BALL_BLADE_RADIUS = 4.f;
    // safe clearance height used when moving between separate surfaces
    static constexpr float TRAVEL_CLEARANCE = 10.f;
    static std::pair<uint32_t, uint32_t> CoordinateToHeightMapIndex(const float x, const float z);
};