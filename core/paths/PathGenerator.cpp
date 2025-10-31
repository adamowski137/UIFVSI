#include "PathGenerator.hpp"
#include "Config.hpp"
PathGenerator::PathGenerator() {}

void PathGenerator::remapPoints(const std::vector<std::weak_ptr<Object>> &objects) {
    // Remap the points from the scene manager's coordinate system to the path's coordinate system
    // all points must lie within the defined block dimensions from Config.hpp
    // the center is in (0, BASE_HEIGHT, 0)
    // the min corner is at (-BLOCK_WIDTH/2, BASE_HEIGHT, -BLOCK_DEPTH/2)
    // the max corner is at (BLOCK_WIDTH/2, BLOCK_HEIGHT, BLOCK_DEPTH/2)

    auto blockWidth = Config::BLOCK_WIDTH;
    auto blockDepth = Config::BLOCK_DEPTH;
    auto blockHeight = Config::BLOCK_HEIGHT;
    auto baseHeight = Config::BASE_HEIGHT;

    auto minCorner = math137::Vector3f(0, 0, 0);
    auto maxCorner = math137::Vector3f(0, 0, 0);
    auto center = math137::Vector3f(0, 0, 0);

    // Remap the points from the scene manager's coordinate system to the path's coordinate system
    for(const auto& weakObj : objects) {
        if (auto obj = weakObj.lock()) {
            if (auto point = std::dynamic_pointer_cast<Point>(obj)) {
                math137::Vector3f pos = point->getTranslation();
                // Update min and max corners
                minCorner.x(std::min(minCorner.x(), pos.x()));
                minCorner.y(std::min(minCorner.y(), pos.y()));
                minCorner.z(std::min(minCorner.z(), pos.z()));
                maxCorner.x(std::max(maxCorner.x(), pos.x()));
                maxCorner.y(std::max(maxCorner.y(), pos.y()));
                maxCorner.z(std::max(maxCorner.z(), pos.z()));
            }
        }
    }

    for (const auto& weakObj : objects) {
        if (auto obj = weakObj.lock()) {
            if (auto point = std::dynamic_pointer_cast<Point>(obj)) {
                math137::Vector3f pos = point->getTranslation();

                pos.x(((pos.x() - minCorner.x()) / (maxCorner.x() - minCorner.x()) - 0.5f) * blockWidth);
                pos.y(((pos.y() - minCorner.y()) / (maxCorner.y() - minCorner.y()) - 0.5f) * (blockHeight - baseHeight) + baseHeight);
                pos.z(((pos.z() - minCorner.z()) / (maxCorner.z() - minCorner.z()) - 0.5f) * blockDepth);
                point->setTranslation(pos);
            }
        }
    }
}

std::vector<std::vector<float>> PathGenerator::generateHeightMap(const std::vector<std::weak_ptr<Object>> &objects)
{
    std::vector<std::vector<float>> heightMap(Config::HEIGHT_MAP_RESOLUTION, std::vector<float>(Config::HEIGHT_MAP_RESOLUTION, Config::BASE_HEIGHT));
    for(const auto& weakObj : objects) {
        if(auto obj = weakObj.lock()) {
            if(auto surface = std::dynamic_pointer_cast<BezierC2>(obj)) {
                for(float u = 0; u <= 1.0f; u += Config::SAMPLING_DISTANCE) {
                    for(float v = 0; v <= 1.0f; v += Config::SAMPLING_DISTANCE) {
                        math137::Vector3f pos = surface->getValue(u, v);
                        auto [i, j] = Config::CoordinateToHeightMapIndex(pos.x(), pos.z());
                        if(pos.y() > heightMap[i][j]) {
                            heightMap[i][j] = pos.y();
                        }
                    }
                }
            }
        }
    }
    return heightMap;
}


std::vector<math137::Vector3f> PathGenerator::generatePath(const std::unique_ptr<SceneManager> &manager) {
    std::vector<math137::Vector3f> pathPoints;
    auto objects = manager->getDrawableObjects();
    remapPoints(objects);
    auto heightMap = generateHeightMap(objects);

    for (uint32_t j = 0; j < Config::HEIGHT_MAP_RESOLUTION; j++) {
        if (j % 2 == 0) {
            for (uint32_t i = 0; i < Config::HEIGHT_MAP_RESOLUTION; i++) {
                //we need to find the height of the highest point that is in the radius of the rough blade
                float maxHeight = 0.0f;
                for (int32_t k = -Config::ROUGH_BLADE_RADIUS; k <= Config::ROUGH_BLADE_RADIUS; k++) {
                    for (int32_t l = -Config::ROUGH_BLADE_RADIUS; l <= Config::ROUGH_BLADE_RADIUS; l++) {
                        int32_t ni = static_cast<int32_t>(i) + k;
                        int32_t nj = static_cast<int32_t>(j) + l;
                        if (ni >= 0 && ni < static_cast<int32_t>(Config::HEIGHT_MAP_RESOLUTION) &&
                            nj >= 0 && nj < static_cast<int32_t>(Config::HEIGHT_MAP_RESOLUTION)) {
                            float distance = sqrtf(static_cast<float>(k * k + l * l));
                            if (distance <= Config::ROUGH_BLADE_RADIUS) {
                                if (heightMap[ni][nj] > maxHeight) {
                                    maxHeight = heightMap[ni][nj];
                                }
                            }
                        }
                    }
                }
                float x = ((static_cast<float>(i) / (Config::HEIGHT_MAP_RESOLUTION - 1)) * Config::BLOCK_WIDTH) - (Config::BLOCK_WIDTH / 2);
                float z = ((static_cast<float>(j) / (Config::HEIGHT_MAP_RESOLUTION - 1)) * Config::BLOCK_DEPTH) - (Config::BLOCK_DEPTH / 2);
                float y = maxHeight;
                pathPoints.emplace_back(math137::Vector3f(x, y, z));
            }
        } else {
            for (int32_t i = Config::HEIGHT_MAP_RESOLUTION - 1; i >= 0; i--) {
                float x = ((static_cast<float>(i) / (Config::HEIGHT_MAP_RESOLUTION - 1)) * Config::BLOCK_WIDTH) - (Config::BLOCK_WIDTH / 2);
                float z = ((static_cast<float>(j) / (Config::HEIGHT_MAP_RESOLUTION - 1)) * Config::BLOCK_DEPTH) - (Config::BLOCK_DEPTH / 2);
                float y = heightMap[i][j];
                pathPoints.emplace_back(math137::Vector3f(x, y, z));
            }
        }
    }

    return pathPoints;
}

std::vector<math137::Vector3f> PathGenerator::generateBallPath(const std::unique_ptr<SceneManager> &manager)
{
    std::vector<math137::Vector3f> pathPoints;
    auto objects = manager->getDrawableObjects();

    // Najprostszy algorytm generowania ścieżki dla freza kulistego z użyciem
    // informacji z modułu przecięć (trimming texture):
    // - dla każdej powierzchni (BezierC2) próbkować parametry (u,v)
    // - dla każdej próbki pominąć, jeśli została przycięta (isTrimmedUV)
    // - obliczyć wektor normalny = cross(du, dv) i sprawdzić, czy jest skierowany do góry
    // - przesunąć punkt wzdłuż normalnego o promień freza i dodać do ścieżki

    constexpr float eps = 1e-6f;
    for (const auto &weakObj : objects) {
        if (auto obj = weakObj.lock()) {
            if (auto surface = std::dynamic_pointer_cast<BezierC2>(obj)) {
                // collect points for this surface first
                std::vector<math137::Vector3f> surfacePoints;
                for (float u = 0.0f; u <= 1.0f; u += Config::SAMPLING_DISTANCE) {
                    for (float v = 0.0f; v <= 1.0f; v += Config::SAMPLING_DISTANCE) {
                        // jeśli parametr (u,v) został oznaczony jako przycięty przez moduł przecięć,
                        // pomiń tę próbkę (trimming texture używa 1 == trimmed/discard)
                        if (surface->isTrimmedUV(u, v))
                            continue;

                        math137::Vector3f pos = surface->getValue(u, v);
                        math137::Vector3f du = surface->uDerivative(u, v);
                        math137::Vector3f dv = surface->vDerivative(u, v);
                        math137::Vector3f normal = math137::Vector3f::Cross(du, dv);

                        // sprawdź długość normalnego
                        float lenSq = normal.x() * normal.x() + normal.y() * normal.y() + normal.z() * normal.z();
                        if (lenSq < eps)
                            continue; // pomijamy punkty o nieokreślonym normalnym

                        normal.normalize();

                        // uwzględniamy tylko fragmenty powierzchni skierowane w górę
                        if (normal.y() <= 1e-6f)
                            continue;

                        // przesunięcie wzdłuż normalnego o promień freza
                        math137::Vector3f offset = pos + (normal * Config::BALL_BLADE_RADIUS);
                        surfacePoints.emplace_back(offset);
                    }
                }

                if (surfacePoints.empty())
                    continue;

                // jeśli to nie pierwsza powierzchnia, dodaj ruch podnoszenia i przemieszczenia nad nową powierzchnię
                if (!pathPoints.empty()) {
                    // podnieś z ostatniej pozycji
                    math137::Vector3f last = pathPoints.back();
                    math137::Vector3f liftPoint = last;
                    liftPoint.y(liftPoint.y() + Config::TRAVEL_CLEARANCE);
                    pathPoints.emplace_back(liftPoint);

                    // przesuń się nad pierwszy punkt na nowej powierzchni na wysokości clearance
                    math137::Vector3f approach = surfacePoints.front();
                    approach.y(approach.y() + Config::TRAVEL_CLEARANCE);
                    pathPoints.emplace_back(approach);
                    // teraz opadamy automatycznie do pierwszego punktu surfacePoints (next appended)
                }

                // dołącz punkty powierzchni
                for (const auto &p : surfacePoints) {
                    pathPoints.emplace_back(p);
                }
            }
        }
    }

    return pathPoints;
}
