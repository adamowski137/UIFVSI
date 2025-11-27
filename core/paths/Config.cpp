#include "Config.hpp"

std::pair<uint32_t, uint32_t> Config::CoordinateToHeightMapIndex(const float x, const float z)
{
    uint32_t i = static_cast<uint32_t>(((x + (BLOCK_WIDTH + HEIGHT_MAP_OFFSET) * 0.5f * Config::SCALE) / ((BLOCK_WIDTH + HEIGHT_MAP_OFFSET) * Config::SCALE)) * (HEIGHT_MAP_RESOLUTION_X - 1));
    uint32_t j = static_cast<uint32_t>(((z + (BLOCK_DEPTH + HEIGHT_MAP_OFFSET) * 0.5f * Config::SCALE) / ((BLOCK_DEPTH + HEIGHT_MAP_OFFSET) * Config::SCALE)) * (HEIGHT_MAP_RESOLUTION_Y - 1));
    if (i >= HEIGHT_MAP_RESOLUTION_X)
        i = HEIGHT_MAP_RESOLUTION_X - 1;
    if (j >= HEIGHT_MAP_RESOLUTION_Y)
        j = HEIGHT_MAP_RESOLUTION_Y - 1;
    return {i, j};
}