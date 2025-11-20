#include "PathGenerator.hpp"
#include "Config.hpp"
#include <queue>
#include <algorithm>
#include <utility>
#include <cmath>
#include <execution>
#include "../models/curves/IntersectionCurve.hpp"
PathGenerator::PathGenerator() : m_groundSurface(std::make_shared<FlatSurface>(
                                     (Config::BLOCK_WIDTH + 6 * Config::FLAT_BLADE_RADIUS) * Config::SCALE,
                                     (Config::BLOCK_DEPTH + 6 * Config::FLAT_BLADE_RADIUS) * Config::SCALE,
                                     math137::Vector3f(-(Config::BLOCK_WIDTH + 6 * Config::FLAT_BLADE_RADIUS) * Config::SCALE * 0.5f, 0.1f, -(Config::BLOCK_DEPTH + 6 * Config::FLAT_BLADE_RADIUS) * Config::SCALE * 0.5f)))
{
}

void PathGenerator::remapPoints(
    const std::vector<std::weak_ptr<Object>> &objects)
{

  auto blockWidth = Config::BLOCK_WIDTH - 3.f;
  auto blockDepth = Config::BLOCK_DEPTH - 3.f;
  auto blockHeight = Config::BLOCK_HEIGHT;
  auto baseHeight = Config::BASE_HEIGHT;

  auto minCorner = math137::Vector3f(0, -1.5f, 0);
  auto maxCorner = math137::Vector3f(0, 0, 0);
  auto center = math137::Vector3f(0, 0, 0);

  for (const auto &weakObj : objects)
  {
    if (auto obj = weakObj.lock())
    {
      if (auto point = std::dynamic_pointer_cast<Point>(obj))
      {
        math137::Vector3f pos = point->getTranslation();
        // Update min and max corners
        minCorner.x(std::min(minCorner.x(), pos.x()));
        minCorner.z(std::min(minCorner.z(), pos.z()));
        maxCorner.x(std::max(maxCorner.x(), pos.x()));
        maxCorner.y(std::max(maxCorner.y(), pos.y()));
        maxCorner.z(std::max(maxCorner.z(), pos.z()));
      }
    }
  }

  for (const auto &weakObj : objects)
  {
    if (auto obj = weakObj.lock())
    {
      if (auto point = std::dynamic_pointer_cast<Point>(obj))
      {
        math137::Vector3f pos = point->getTranslation();

        pos.x(((pos.x() - minCorner.x()) / (maxCorner.x() - minCorner.x()) -
               0.5f) *
              blockWidth * Config::SCALE);
        pos.z(((pos.z() - minCorner.z()) / (maxCorner.z() - minCorner.z()) -
               0.5f) *
              blockDepth * Config::SCALE);
        point->setTranslation(pos);
      }
    }
  }
}

std::vector<std::vector<float>> PathGenerator::generateHeightMap()
{
  std::vector<std::vector<float>> heightMap(
      Config::HEIGHT_MAP_RESOLUTION,
      std::vector<float>(Config::HEIGHT_MAP_RESOLUTION, 0.f));
  for (const auto &surface : m_roughSurfaces)
  {
    for (float u = 0; u <= 1.0f; u += Config::SAMPLING_DISTANCE_ROUGH)
    {
      for (float v = 0; v <= 1.0f; v += Config::SAMPLING_DISTANCE_ROUGH)
      {
        math137::Vector3f pos = surface->getValue(u, v);
        math137::Vector3f du = surface->uDerivative(u, v);
        math137::Vector3f dv = surface->vDerivative(u, v);
        if (du * du < 1e-12 || dv * dv < 1e-12)
          continue;

        auto [i, j] = Config::CoordinateToHeightMapIndex(pos.x(), pos.z());
        if (pos.y() > heightMap[i][j])
        {
          heightMap[i][j] = pos.y();
        }
      }
    }
  }
  return heightMap;
}

std::vector<math137::Vector3f>
PathGenerator::generatePath(const std::unique_ptr<SceneManager> &manager)
{
  std::vector<math137::Vector3f> pathPoints;
  auto heightMap = generateHeightMap();

  const uint32_t N = Config::HEIGHT_MAP_RESOLUTION;
  if (N < 2)
    return pathPoints;

  auto emitPoint = [&](uint32_t i, uint32_t j, float minHeight)
  {
    float x = ((float(i) / (N - 1)) * Config::BLOCK_WIDTH * Config::SCALE) -
              (Config::BLOCK_WIDTH * Config::SCALE * 0.5f);
    float z = ((float(j) / (N - 1)) * Config::BLOCK_DEPTH * Config::SCALE) -
              (Config::BLOCK_DEPTH * Config::SCALE * 0.5f);
    float h = heightMap[i][j] / Config::SCALE + Config::BASE_HEIGHT + 2.f;
    float finalHeight = std::max(h, minHeight);
    pathPoints.emplace_back(x / Config::SCALE, finalHeight, z / Config::SCALE);
  };

  for (int it = 0; it < 2; it++)
  {
    float minHeight = Config::BASE_HEIGHT + 2.f +
                      (Config::BLOCK_HEIGHT - Config::BASE_HEIGHT) * 0.5f * (1 - it);

    int jStart = (it == 0 ? 0 : int(N) - 1);
    int jStop = (it == 0 ? int(N) - 1 : 0);
    std::function jPred = std::less_equal<int>();
    if (it != 0)
      jPred = std::greater_equal<int>();
    auto jInc = (it == 0 ? [](int a)
                     { return a + 1; }
                         : [](int a)
                     { return a - 1; });

    bool forward = true;

    for (int j = jStart; jPred(j, jStop); j = jInc(j))
    {
      int iStart = forward ? 0 : int(N) - 1;
      int iStop = forward ? int(N) - 1 : 0;
      std::function iPred = std::less_equal<int>();
      if (!forward)
        iPred = std::greater_equal<int>();
      auto iInc = forward ? [](int a)
      { return a + 1; }
                          : [](int a)
      { return a - 1; };

      for (int i = iStart; iPred(i, iStop); i = iInc(i))
        emitPoint((uint32_t)i, (uint32_t)j, minHeight);

      forward = !forward;
    }
  }

  return pathPoints;
}

void PathGenerator::createMillingSurface(
    const std::unique_ptr<SceneManager> &manager, float radius)
{
  auto objects = manager->getDrawableObjects();
  for (const auto &weakObj : objects)
  {
    if (auto obj = weakObj.lock())
    {
      if (auto surface = std::dynamic_pointer_cast<BezierC2>(obj))
      {
        m_detailSurfaces.push_back(std::make_shared<ShiftedSurface>(
            surface, radius * Config::SCALE));
        m_roughSurfaces.push_back(std::make_shared<ShiftedSurface>(
            surface, Config::ROUGH_BLADE_RADIUS * Config::SCALE));
        m_flatSurfaces.push_back(std::make_shared<ShiftedSurface>(
            surface, Config::FLAT_BLADE_RADIUS * Config::SCALE));
      }
    }
  }
  manager->setCursorPosition({-2.5f, 0.394f, 2.786f});
  manager->addIntersection(m_detailSurfaces[0], m_detailSurfaces[3], true, 0.001f);
  std::vector<std::pair<int, int>> surfacePairs = {{0, 2}, {0, 3}, {1, 2}, {2, 3}, {2, 4}, {4, 5}, {4, 6}, {1, 4}, {2, 2}};
  for (const auto &[i, j] : surfacePairs)
    manager->addIntersection(m_detailSurfaces[i], m_detailSurfaces[j], false, 0.001f);

  std::vector<std::tuple<int, int, bool>> trimmingPairs = {
      {250, 400, false}, {0, 0, false}, {0, 0, false}, {288, 55, false}, {250, 170, false}, {500, 0, true}, {0, 0, false}, {45, 50, false}, {500, 0, false}, {0, 0, false}, {0, 0, false}, {330, 70, false}, {175, 175, false}, {500, 0, false}, {175, 300, false}, {500, 0, false}, {0, 0, false}, {400, 0, false}, {-1, -1, false}, {-1, -1, false}};
  int idx = 0;
  for (const auto &obj : manager->getObjects())
  {
    if (auto curve = std::dynamic_pointer_cast<IntersectionCurve>(obj))
    {
      if (std::get<0>(trimmingPairs[idx]) != -1)
      {
        if (std::get<2>(trimmingPairs[idx]))
        {
          curve->intersectTrimmingTexture(std::get<0>(trimmingPairs[idx]), std::get<1>(trimmingPairs[idx]), 0);
        }
        else
        {
          curve->unionTrimmingTexture(std::get<0>(trimmingPairs[idx]), std::get<1>(trimmingPairs[idx]), 0);
        }
      }
      idx++;
      if (std::get<0>(trimmingPairs[idx]) != -1)
      {
        if (std::get<2>(trimmingPairs[idx]))
        {
          curve->intersectTrimmingTexture(std::get<0>(trimmingPairs[idx]), std::get<1>(trimmingPairs[idx]), 1);
        }
        else
        {
          curve->unionTrimmingTexture(std::get<0>(trimmingPairs[idx]), std::get<1>(trimmingPairs[idx]), 1);
        }
        curve->unionTrimmingTexture(std::get<0>(trimmingPairs[idx]), std::get<1>(trimmingPairs[idx]), 1);
      }
      idx++;
    }
  }

  for (const auto &surface : m_flatSurfaces)
  {
    manager->addIntersection(surface, m_groundSurface, false, 0.001f);
  }
  manager->m_cursor.setTranslation({-6.3f, 0.3f, 3.695f});
  manager->addIntersection(m_flatSurfaces[3], m_groundSurface, true, 0.001f);
}

std::vector<std::vector<math137::Vector3f>>
PathGenerator::generateBallSegments(const std::unique_ptr<SceneManager> &manager)
{
  std::vector<std::vector<math137::Vector3f>> segments;
  std::shared_ptr<ShiftedSurface> prevSurface = nullptr;
  math137::Vector3f prevPoint(0, 0, 0);
  bool hasPrev = false;
  bool wasInTrimmedRegion = false;

  float start = 0.f;
  float stop = 1.f;
  std::function pred = std::less_equal<float>();
  std::function inc = std::plus<float>();
  for (const auto &surface : m_detailSurfaces)
  {
    bool dir = true;
    std::vector<math137::Vector3f> segmentPoints;
    for (float v = 0; v <= 1.0f; v += Config::SAMPLING_DISTANCE_BALL)
    {
      if (dir)
      {
        start = 0.f;
        stop = 1.f;
        pred = std::less_equal<float>();
        inc = std::plus<float>();
        dir = false;
      }
      else
      {
        start = 1.f;
        stop = 0.f;
        pred = std::greater_equal<float>();
        inc = std::minus<float>();
        dir = true;
      }

      // collect points for this u-loop (segment) separately

      for (float u = start; pred(u, stop); u = inc(u, Config::SAMPLING_DISTANCE_BALL))
      {
        // skip if explicit trimming mask marks this UV as trimmed
        if (surface->isTrimmedUV(u, v))
        {
          wasInTrimmedRegion = true;
        }

        math137::Vector3f du = surface->uDerivative(u, v);
        math137::Vector3f dv = surface->vDerivative(u, v);
        math137::Vector3f normal = math137::Vector3f::Cross(du, dv);
        normal.normalize();

        if (normal.y() < Config::NORMAL_THRESHOLD || normal.any(
                                                         [](float coord)
                                                         { return std::isnan(coord) || std::isinf(coord); }))
        {
          wasInTrimmedRegion = true;
        }

        math137::Vector3f pos = math137::Vector3f(0.f, Config::BASE_HEIGHT, 0.f) + surface->getValue(u, v) / Config::SCALE;
        if (pos.y() < Config::BASE_HEIGHT + Config::BALL_BLADE_RADIUS || pos.any(
                                                                             [](float coord)
                                                                             { return std::isnan(coord) || std::isinf(coord); }))
        {
          wasInTrimmedRegion = true;
        }
        // add the offset point (ball center position) to segment
        if (wasInTrimmedRegion && segmentPoints.size() > 0)
        {
          segments.emplace_back(std::move(segmentPoints));
          segmentPoints.clear();
        }
        else if (!wasInTrimmedRegion)
        {
          segmentPoints.emplace_back(pos);
        }

        wasInTrimmedRegion = false;
      }
    }
    if (segmentPoints.size() > 0)
    {
      segments.emplace_back(std::move(segmentPoints));
      wasInTrimmedRegion = false;
    }
  }

  return segments;
}

std::vector<std::vector<math137::Vector3f>> PathGenerator::generateFlatSegments(const std::unique_ptr<SceneManager> &manager)
{
  std::vector<std::vector<math137::Vector3f>> segments;

  // spacing between passes: 2R - eps (eps = 0.1R)
  const float R = Config::FLAT_BLADE_RADIUS;
  const float eps = 0.1f * R;
  const float step = (2.f * R - eps) * Config::SCALE;

  const float width = (Config::BLOCK_WIDTH + 6.f * R) * Config::SCALE;
  const float depth = (Config::BLOCK_DEPTH + 6.f * R) * Config::SCALE;

  // x coordinates in world-space where we will run the passes
  const float halfW = width * 0.5f;
  const float halfD = depth * 0.5f;

  float stepU = step / width;
  float stepV = 0.01f;

  int lineIdx = 0;
  std::vector<math137::Vector3f> segmentPoints;
  for (float u = 0.f; u <= 1.f; u += stepU)
  {
    bool forward = (lineIdx % 2) == 0; // snake direction
    lineIdx++;

    float vStart = forward ? 0.f : 1.f;
    float vStop = forward ? 1.f : 0.f;
    std::function<bool(float, float)> pred;
    std::function<float(float, float)> inc;
    if (forward)
    {
      pred = std::less_equal<float>();
      inc = std::plus<float>();
    }
    else
    {
      pred = std::greater_equal<float>();
      inc = std::minus<float>();
    }

    for (float v = vStart; pred(v, vStop); v = inc(v, stepV))
    {
      if (!m_groundSurface->isTrimmedUV(u, v))
      {
        if (segmentPoints.size() > 0)
        {
          segments.emplace_back(std::move(segmentPoints));
          segmentPoints.clear();
        }
        continue;
      }
      math137::Vector3f pos = m_groundSurface->getValue(u, v) / Config::SCALE;
      pos.y(Config::BASE_HEIGHT + 2.f);

      segmentPoints.emplace_back(pos);
    }
  }

  if (!segmentPoints.empty())
    segments.emplace_back(std::move(segmentPoints));

  auto silhouetteSegment = generateSilhouetteSegment(manager);
  segments.insert(segments.end(), silhouetteSegment.begin(), silhouetteSegment.end());

  return segments;
}

std::vector<std::vector<math137::Vector3f>> PathGenerator::generateSilhouetteSegment(const std::unique_ptr<SceneManager> &manager)
{
  auto contours = m_groundSurface->extractAllContours();
  std::vector<std::vector<math137::Vector3f>> silhouette;
  for (auto &contour : contours)
  {
    std::transform(contour.begin(), contour.end(), contour.begin(), [](const math137::Vector3f &pos)
                   { return math137::Vector3f(pos.x() / Config::SCALE, Config::BASE_HEIGHT + 2.f, pos.z() / Config::SCALE); });
    silhouette.emplace_back(std::move(contour));
  }
  return silhouette;
}

std::vector<math137::Vector3f>
PathGenerator::generateBallPath(const std::unique_ptr<SceneManager> &manager)
{
  std::vector<math137::Vector3f> pathPoints;
  auto objects = manager->getDrawableObjects();
  auto segments = generateBallSegments(manager);

  for (const auto &segment : segments)
  {
    if (segment.size() < 2)
      continue;

    if (pathPoints.size() > 0)
    {
      // add a connecting point between segments
      math137::Vector3f lastPoint = pathPoints.back();
      math137::Vector3f firstPoint = segment.front();
      lastPoint.y(Config::TRAVEL_CLEARANCE);
      pathPoints.emplace_back(lastPoint);
      firstPoint.y(Config::TRAVEL_CLEARANCE);
      pathPoints.emplace_back(firstPoint);
    }
    pathPoints.insert(pathPoints.end(), segment.begin(), segment.end());
  }
  return pathPoints;
}

std::vector<math137::Vector3f>
PathGenerator::generateFlatPath(const std::unique_ptr<SceneManager> &manager)
{
  std::vector<math137::Vector3f> pathPoints;
  auto objects = manager->getDrawableObjects();
  auto segments = generateFlatSegments(manager);

  for (const auto &segment : segments)
  {
    if (segment.size() < 2)
      continue;

    if (pathPoints.size() > 0)
    {
      // add a connecting point between segments
      math137::Vector3f lastPoint = pathPoints.back();
      math137::Vector3f firstPoint = segment.front();
      lastPoint.y(Config::TRAVEL_CLEARANCE);
      pathPoints.emplace_back(lastPoint);
      firstPoint.y(Config::TRAVEL_CLEARANCE);
      pathPoints.emplace_back(firstPoint);
    }
    pathPoints.insert(pathPoints.end(), segment.begin(), segment.end());
  }
  return pathPoints;
}
