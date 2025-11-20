#pragma once
#include <vector>
#include <Vector.hpp>
#include "../SceneManager.hpp"
#include "../models/surfaces/ShiftedSurface.hpp"
#include "../models/surfaces/FlatSurface.hpp"

class PathGenerator {
public:
    PathGenerator();
    std::vector<math137::Vector3f> generatePath(const std::unique_ptr<SceneManager> &manager);
    std::vector<math137::Vector3f> generateBallPath(const std::unique_ptr<SceneManager> &manager);
    std::vector<math137::Vector3f> generateFlatPath(const std::unique_ptr<SceneManager> &manager);
    std::vector<std::vector<math137::Vector3f>> generateBallSegments(const std::unique_ptr<SceneManager> &manager);
    std::vector<std::vector<math137::Vector3f>> generateFlatSegments(const std::unique_ptr<SceneManager> &manager);
    std::vector<std::vector<math137::Vector3f>> generateSilhouetteSegment(const std::unique_ptr<SceneManager> &manager);
    void createMillingSurface(const std::unique_ptr<SceneManager> &manager, float radius);
private:
    void remapPoints(const std::vector<std::weak_ptr<Object>> &objects);
    std::vector<std::vector<float>> generateHeightMap();
    std::vector<std::shared_ptr<ShiftedSurface>> m_detailSurfaces;
    std::vector<std::shared_ptr<ShiftedSurface>> m_roughSurfaces;
    std::vector<std::shared_ptr<ShiftedSurface>> m_flatSurfaces;
    std::shared_ptr<FlatSurface> m_groundSurface;
};
