#include "PathGenerator.hpp"
#include "Config.hpp"
#include <queue>
#include <algorithm>
#include <utility>
#include <cmath>
#include <limits>
#include <map>
#include <unordered_map>
#include <execution>
#include "../serialize/Parser.hpp"
#include "../models/curves/IntersectionCurve.hpp"
#include <stack>
#include <fstream>
PathGenerator::PathGenerator() : m_flatGroundSurface(std::make_shared<FlatSurface>(
                                     (Config::BLOCK_WIDTH + 4 * Config::FLAT_BLADE_RADIUS) * Config::SCALE,
                                     (Config::BLOCK_DEPTH + 4 * Config::FLAT_BLADE_RADIUS) * Config::SCALE,
                                     math137::Vector3f(-(Config::BLOCK_WIDTH + 4 * Config::FLAT_BLADE_RADIUS) * Config::SCALE * 0.5f, 0.0f, -(Config::BLOCK_DEPTH + 4 * Config::FLAT_BLADE_RADIUS) * Config::SCALE * 0.5f))),
                                 m_detailGroundSurface(std::make_shared<FlatSurface>(
                                     (Config::BLOCK_WIDTH + 6 * Config::BALL_BLADE_RADIUS) * Config::SCALE,
                                     (Config::BLOCK_DEPTH + 6 * Config::BALL_BLADE_RADIUS) * Config::SCALE,
                                     math137::Vector3f(-(Config::BLOCK_WIDTH + 6 * Config::BALL_BLADE_RADIUS) * Config::SCALE * 0.5f, 0.0f, -(Config::BLOCK_DEPTH + 6 * Config::BALL_BLADE_RADIUS) * Config::SCALE * 0.5f)))
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
      Config::HEIGHT_MAP_RESOLUTION_X,
      std::vector<float>(Config::HEIGHT_MAP_RESOLUTION_Y, 0.f));
  for (const auto &surface : m_roughSurfaces)
  {
    for (float u = 0; u <= 1.0f; u += Config::SAMPLING_DISTANCE_ROUGH)
    {
      for (float v = 0; v <= 1.0f; v += Config::SAMPLING_DISTANCE_ROUGH)
      {
        math137::Vector3f pos = surface->getValue(u, v);
        math137::Vector3f du = surface->uDerivative(u, v);
        math137::Vector3f dv = surface->vDerivative(u, v);
        if (du * du < 1e-8 || dv * dv < 1e-8)
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
  // remapPoints(manager->getDrawableObjects());
  std::vector<math137::Vector3f> pathPoints;
  auto heightMap = generateHeightMap();

  auto emitPoint = [&](uint32_t i, uint32_t j, float minHeight)
  {
    float x = ((float(i) / (Config::HEIGHT_MAP_RESOLUTION_X - 1)) * ((Config::BLOCK_WIDTH + Config::HEIGHT_MAP_OFFSET) * Config::SCALE) - ((Config::BLOCK_WIDTH + Config::HEIGHT_MAP_OFFSET) * Config::SCALE * 0.5f));
    float z = ((float(j) / (Config::HEIGHT_MAP_RESOLUTION_Y - 1)) * ((Config::BLOCK_DEPTH + Config::HEIGHT_MAP_OFFSET) * Config::SCALE) - ((Config::BLOCK_DEPTH + Config::HEIGHT_MAP_OFFSET) * Config::SCALE * 0.5f));
    float h = heightMap[i][j] / Config::SCALE + Config::BASE_HEIGHT + 2.f;
    float finalHeight = std::max(h, minHeight);
    pathPoints.emplace_back(x / Config::SCALE, finalHeight, z / Config::SCALE);
  };

  for (int it = 0; it < 2; it++)
  {
    float minHeight = Config::BASE_HEIGHT + 2.f +
                      (Config::BLOCK_HEIGHT - Config::BASE_HEIGHT) * 0.5f * (1 - it);

    int jStart = (it == 0 ? 0 : int(Config::HEIGHT_MAP_RESOLUTION_Y) - 1);
    int jStop = (it == 0 ? int(Config::HEIGHT_MAP_RESOLUTION_Y) - 1 : 0);
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
      int iStart = forward ? 0 : int(Config::HEIGHT_MAP_RESOLUTION_X) - 1;
      int iStop = forward ? int(Config::HEIGHT_MAP_RESOLUTION_X) - 1 : 0;
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
        // m_roughSurfaces.push_back(std::make_shared<ShiftedSurface>(
        //     surface, Config::ROUGH_BLADE_RADIUS * Config::SCALE));
        m_roughSurfaces.push_back(std::make_shared<ShiftedSurface>(
            surface, Config::ROUGH_BLADE_RADIUS * Config::SCALE));
        m_flatSurfaces.push_back(std::make_shared<ShiftedSurface>(
            surface, Config::FLAT_BLADE_RADIUS * Config::SCALE));
      }
    }
  }
  std::vector<std::pair<int, int>> surfacePairs = {{0, 2}, {0, 6}, {1, 2}, {2, 3}, {3, 4}, {3, 5}, {2, 6}, {1, 3}};
  for (const auto &[i, j] : surfacePairs)
    manager->addIntersection(m_detailSurfaces[i], m_detailSurfaces[j], false, 0.001f);

  for (int i = 0; i < m_detailSurfaces.size(); i++)
  {
    manager->addIntersection(m_detailSurfaces[i], m_detailGroundSurface, false, 0.001f);
  }

  for (int i = 0; i < m_flatSurfaces.size(); i++)
  {
    manager->addIntersection(m_flatSurfaces[i], m_flatGroundSurface, false, 0.01f);
  }
}

std::vector<std::vector<math137::Vector3f>>
PathGenerator::generateBallSegments()
{
  std::vector<std::vector<math137::Vector3f>> segments;
  for (const auto &surface : m_detailSurfaces)
  {
    auto surfaceSegments = surface->extractPaths(7);
    for (auto &segment : surfaceSegments)
    {
      std::transform(segment.begin(), segment.end(), segment.begin(), [](const math137::Vector3f &pos)
                     { return pos / Config::SCALE + math137::Vector3f(0.f, Config::BASE_HEIGHT + 2.1f, 0.f); });
      segments.emplace_back(std::move(segment));
    }
  }

  return segments;
}

std::vector<std::vector<math137::Vector3f>> PathGenerator::generateFlatSegments(const std::vector<std::shared_ptr<IntersectionCurve>>& curves)
{
  std::vector<std::vector<math137::Vector3f>> segments;

  const float R = Config::FLAT_BLADE_RADIUS;
  const float eps = 0.1f * R;
  const float step = (2.f * R - eps) * Config::SCALE;

  const float width = (Config::BLOCK_WIDTH + 4.f * R) * Config::SCALE;
  const float depth = (Config::BLOCK_DEPTH + 4.f * R) * Config::SCALE;

  // x coordinates in world-space where we will run the passes
  const float halfW = width * 0.5f;
  const float halfD = depth * 0.5f;

  float stepU = step / width;
  float stepV = 0.01f;

  int texWidth = Intersectable::m_width;
  int texHeight = Intersectable::m_height;

  int texelStepU = static_cast<int>(stepU * texWidth);
  int texelStepV = static_cast<int>(stepV * texHeight);

  auto firstEligblePoint = [&](float u, float prevV, bool dir, std::vector<math137::Vector3f> &segmentPoints)
  {
    int startX = static_cast<int>(u * texWidth);
    if (dir)
    {
      for (float v = 0.f; v <= 1.f; v += stepV)
      {
        if (m_flatGroundSurface->isTrimmedUV(u, v))
        {
          if (prevV > v)
          {

            math137::Vector3f pos = m_flatGroundSurface->getValue(u - stepU, v - stepV) / Config::SCALE;
            pos.y(Config::BASE_HEIGHT + 2.f);
            segmentPoints.emplace_back(pos);
          }
          else
          {
            math137::Vector3f pos = m_flatGroundSurface->getValue(u, prevV) / Config::SCALE;
            pos.y(Config::BASE_HEIGHT + 2.f);
            segmentPoints.emplace_back(pos);
          }
          return v - stepV;
        }
      }
      math137::Vector3f pos = m_flatGroundSurface->getValue(u, prevV) / Config::SCALE;
      pos.y(Config::BASE_HEIGHT + 2.f);
      segmentPoints.emplace_back(pos);
      return 1.f;
    }
    else
    {
      for (float v = 1.f; v >= 0.f; v -= stepV)
      {
        if (m_flatGroundSurface->isTrimmedUV(u, v))
        {
          if (prevV < v)
          {
            math137::Vector3f pos = m_flatGroundSurface->getValue(u + stepU, v + stepV) / Config::SCALE;
            pos.y(Config::BASE_HEIGHT + 2.f);
            segmentPoints.emplace_back(pos);
          }
          else
          {
            math137::Vector3f pos = m_flatGroundSurface->getValue(u, prevV) / Config::SCALE;
            pos.y(Config::BASE_HEIGHT + 2.f);
            segmentPoints.emplace_back(pos);
          }
          return v + stepV;
        }
      }
      math137::Vector3f pos = m_flatGroundSurface->getValue(u, prevV) / Config::SCALE;
      pos.y(Config::BASE_HEIGHT + 2.f);
      segmentPoints.emplace_back(pos);
      return 0.f;
    }
    return -1.f;
  };

  int lineIdx = 0;
  std::vector<math137::Vector3f> segmentPoints;
  float prevV = 0.f;
  for (float u = 0.f; u <= 1.f; u += stepU)
  {
    bool forward = (lineIdx % 2) == 0; // snake direction
    lineIdx++;
    float vStart = forward ? 0.f : firstEligblePoint(u, prevV, true, segmentPoints);
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
      if (m_flatGroundSurface->isTrimmedUV(u, v))
      {
        prevV = v;
        break;
      }
      math137::Vector3f pos = m_flatGroundSurface->getValue(u, v) / Config::SCALE;
      pos.y(Config::BASE_HEIGHT + 2.f);

      segmentPoints.emplace_back(pos);
    }
  }
  lineIdx = 1;
  prevV = 0.f;
  for (float u = 1.f; u >= 0.f; u -= stepU)
  {
    bool forward = (lineIdx % 2) == 0; // snake direction
    lineIdx++;

    float vStart = forward ? firstEligblePoint(u, prevV, false, segmentPoints) : 1.f;
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
      if (m_flatGroundSurface->isTrimmedUV(u, v))
      {
        prevV = v;
        break;
      }
      math137::Vector3f pos = m_flatGroundSurface->getValue(u, v) / Config::SCALE;
      pos.y(Config::BASE_HEIGHT + 2.f);

     segmentPoints.emplace_back(pos);
    }
    std::cout << vStop <<std::endl;
  }

  if (!segmentPoints.empty())
    segments.emplace_back(std::move(segmentPoints));

  auto silhouetteSegment = generateFlatSilhouetteSegment(curves);
  segments.emplace_back(std::move(silhouetteSegment));
  return segments;
}

std::vector<math137::Vector3f> PathGenerator::generateFlatSilhouetteSegment(const std::vector<std::shared_ptr<IntersectionCurve>>& curves)
{
  auto findIntersectionFromPoint = [](const std::vector<math137::Vector3f>& points1, const std::vector<math137::Vector3f>& points2, int index = 0) {
    
    for (const auto& p1 : points1)
    {
      for (const auto& p2 : points2)
      {
        if ((p1 - p2).x() * (p1 - p2).x() + (p1 - p2).z() * (p1 - p2).z() < 1e-4f)
        {
          std::cout << p1 << std::endl;
        }
      }
    }
    return math137::Vector3f();
  };
  auto findClosestPointIndex = [](const std::vector<math137::Vector3f>& points, const math137::Vector3f& target) {
    float minDist = std::numeric_limits<float>::max();
    math137::Vector3f closestPoint;
    int idx = -1;
    for (const auto& p : points)
    {
      float dist = (p - target).x() * (p - target).x() + (p - target).z() * (p - target).z();
      if (dist < minDist)
      {
        idx = &p - &points[0];
        minDist = dist;
        closestPoint = p;
      }
    }
    return idx;
  };

  std::array<std::pair<float, float>, 6> ends = {{
    {1.31891,-4.87327}, 
    {-5.99868,-1.72829},
    {-2.87555, 6.05587},
    {-0.67814, 0.42583},
    {1.01611, 0.45465},
    {2.9638, 1.30208 }
    }};
    std::vector<int> dir = {
      1, 1, 1, 1, 1, 1
    };

  std::vector<math137::Vector3f> silhouettePoints;
  math137::Vector3f startPoint = m_flatGroundSurface->getValue(0.f, 0.f) / Config::SCALE;
  startPoint.y(Config::BASE_HEIGHT + 2.f);
  silhouettePoints.push_back(startPoint);
  for(int i = 0; i < curves.size(); i++)
  {
    //auto intersectionPoint = findIntersectionFromPoint(curves[i]->getPoints(), curves[i+1]->getPoints());
    //std::cout << intersectionPoint<< std::endl;
    math137::Vector3f start{ends[i].first, 0.f, ends[i].second};
    math137::Vector3f end{ends[(i+1) % ends.size()].first, 0.f, ends[(i+1) % ends.size()].second};
    int startIdx = findClosestPointIndex(curves[i]->getPoints(), start);
    int endIdx = findClosestPointIndex(curves[i]->getPoints(), end);
    if (startIdx == -1 || endIdx == -1)
      continue;

      int size = curves[i]->getPointCount();
      auto points = curves[i]->getPoints();

      for(int j = startIdx; j != endIdx; j = (j + dir[i] + size) % size)
      {
        auto pos = points[j];
        pos = pos / Config::SCALE;
        pos.y(Config::BASE_HEIGHT + 2.f);
        silhouettePoints.emplace_back(pos);
      }

  }
  return silhouettePoints;
}


std::vector<std::vector<math137::Vector3f>> PathGenerator::generateDetailSilhouetteSegment()
{
  auto configValues = Parser::ParseConfig("C:/Users/adam/Desktop/projekty/UIFVSI/core/config/contours.txt");
  std::vector<std::vector<math137::Vector3f>> contours;
  for (const auto &[idx, x, y] : configValues)
  {
    auto contour = m_detailSurfaces[idx]->extractContour(x, y);
    std::transform(contour.begin(), contour.end(), contour.begin(), [](const math137::Vector3f &pos)
                   { return pos / Config::SCALE + math137::Vector3f(0.f, Config::BASE_HEIGHT + 2.1f, 0.f); });
    contours.emplace_back(std::move(contour));
  }
  return contours;
}

void PathGenerator::generateDetailTrimming()
{

  auto configValues = Parser::ParseConfig("C:/Users/adam/Desktop/projekty/UIFVSI/core/config/detail.txt");
  for (const auto &[idx, x, y] : configValues)
  {
    m_detailSurfaces[idx]->unionTrimmingTexture(x, y);
  }
}

std::vector<math137::Vector3f>
PathGenerator::generateBallPath(const std::unique_ptr<SceneManager> &manager)
{
  generateDetailTrimming();
  std::vector<math137::Vector3f> pathPoints;
  auto segments = generateBallSegments();
  m_detailGroundSurface->unionTrimmingTexture(1, 1);
  m_detailGroundSurface->unionTrimmingTexture(134, 404);
  m_detailGroundSurface->unionTrimmingTexture(270, 333);
  m_detailGroundSurface->unionTrimmingTexture(348, 314);
  m_detailGroundSurface->unionTrimmingTexture(260, 297);
  m_detailGroundSurface->unionTrimmingTexture(179, 214);
  m_detailGroundSurface->unionTrimmingTexture(367, 71);
  m_detailGroundSurface->unionTrimmingTexture(411, 204);
  m_detailGroundSurface->unionTrimmingTexture(333, 211);

  auto ground = m_detailGroundSurface->gridMillingPath(0.01f, 0.01f);
  auto groundSilhouette = m_detailGroundSurface->extractContour(300, 110);
  std::transform(ground.begin(), ground.end(), ground.begin(), [](const math137::Vector3f &pos)
                 { return pos / Config::SCALE + math137::Vector3f(0.f, Config::BASE_HEIGHT + 2.f, 0.f); });
  std::transform(groundSilhouette.begin(), groundSilhouette.end(), groundSilhouette.begin(), [](const math137::Vector3f &pos)
                 { return pos / Config::SCALE + math137::Vector3f(0.f, Config::BASE_HEIGHT + 2.f, 0.f); });
  segments.emplace_back(std::move(ground));
  segments.emplace_back(std::move(groundSilhouette));
  // std::vector<std::vector<math137::Vector3f>> segments;
  auto silhouetteSegment = generateDetailSilhouetteSegment();

  segments.insert(segments.end(), silhouetteSegment.begin(), silhouetteSegment.end());

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
  auto trimings = Parser::ParseConfig("C:/Users/adam/Desktop/projekty/UIFVSI/core/config/ground.txt");
  for (const auto &[idx, x, y] : trimings)
  {
    m_flatGroundSurface->unionTrimmingTexture(x, y);
  }
  // collect intersection curves from the scene manager
  std::vector<std::shared_ptr<IntersectionCurve>> curves;
  for (const auto &obj : manager->getObjects()) {
    if (!obj) continue;
    if (auto ic = std::dynamic_pointer_cast<IntersectionCurve>(obj))
      curves.push_back(ic);
  }

  // read `flat.txt` as an ordered list of curve indices (preferred)
  std::vector<std::shared_ptr<IntersectionCurve>> selectedCurves;
  std::ifstream in("C:/Users/adam/Desktop/projekty/UIFVSI/core/config/flat.txt");
  if (in && !curves.empty()) {
    int idx;
    while (in >> idx) {
      selectedCurves.push_back(curves[13 + idx]);
      std::cout << curves[13 + idx]->name << std::endl;
    }
  }
  if (selectedCurves.empty())
    selectedCurves = curves; // fallback to all curves if config empty or missing

  auto segment = generateFlatSegments(selectedCurves);

  for (const auto &seg : segment)
  {
    pathPoints.insert(pathPoints.end(), seg.begin(), seg.end());
  }
  return pathPoints;
}
