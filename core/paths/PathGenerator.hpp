#pragma once
#include <vector>
#include <Vector.hpp>
#include "../SceneManager.hpp"

class PathGenerator {
public:
    PathGenerator();
    std::vector<math137::Vector3f> generatePath(const std::unique_ptr<SceneManager> &manager);
    std::vector<math137::Vector3f> generateBallPath(const std::unique_ptr<SceneManager> &manager);
private:
    void remapPoints(const std::vector<std::weak_ptr<Object>> &objects);
    std::vector<std::vector<float>> generateHeightMap(const std::vector<std::weak_ptr<Object>> &objects);
};
