#include "Config.hpp"

std::pair<uint32_t, uint32_t> Config::CoordinateToHeightMapIndex(const float x, const float z)
{
    uint32_t i = static_cast<uint32_t>(((x + BLOCK_WIDTH * 0.5f * Config::SCALE) / (BLOCK_WIDTH * Config::SCALE)) * (HEIGHT_MAP_RESOLUTION - 1));
    uint32_t j = static_cast<uint32_t>(((z + BLOCK_DEPTH * 0.5f * Config::SCALE) / (BLOCK_DEPTH * Config::SCALE)) * (HEIGHT_MAP_RESOLUTION - 1));
    if (i >= HEIGHT_MAP_RESOLUTION)
        i = HEIGHT_MAP_RESOLUTION - 1;
    if (j >= HEIGHT_MAP_RESOLUTION)
        j = HEIGHT_MAP_RESOLUTION - 1;
    return {i, j};
}