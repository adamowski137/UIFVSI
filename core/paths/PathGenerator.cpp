#include "PathGenerator.hpp"
#include "Config.hpp"
#include <queue>
#include <algorithm>
#include <utility>
#include <cmath>
PathGenerator::PathGenerator() {}

void PathGenerator::remapPoints(
    const std::vector<std::weak_ptr<Object>> &objects) {

  auto blockWidth = Config::BLOCK_WIDTH;
  auto blockDepth = Config::BLOCK_DEPTH;
  auto blockHeight = Config::BLOCK_HEIGHT;
  auto baseHeight = Config::BASE_HEIGHT;

  auto minCorner = math137::Vector3f(0, 0, 0);
  auto maxCorner = math137::Vector3f(0, 0, 0);
  auto center = math137::Vector3f(0, 0, 0);

  for (const auto &weakObj : objects) {
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

  for (const auto &weakObj : objects) {
    if (auto obj = weakObj.lock()) {
      if (auto point = std::dynamic_pointer_cast<Point>(obj)) {
        math137::Vector3f pos = point->getTranslation();

        pos.x(((pos.x() - minCorner.x()) / (maxCorner.x() - minCorner.x()) -
               0.5f) *
              blockWidth);
        pos.y(((pos.y() - minCorner.y()) / (maxCorner.y() - minCorner.y()) -
               0.5f) *
                  (blockHeight - baseHeight) +
              baseHeight);
        pos.z(((pos.z() - minCorner.z()) / (maxCorner.z() - minCorner.z()) -
               0.5f) *
              blockDepth);
        point->setTranslation(pos);
      }
    }
  }
}

std::vector<std::vector<float>> PathGenerator::generateHeightMap(
    const std::vector<std::weak_ptr<Object>> &objects) {
  std::vector<std::vector<float>> heightMap(
      Config::HEIGHT_MAP_RESOLUTION,
      std::vector<float>(Config::HEIGHT_MAP_RESOLUTION, Config::BASE_HEIGHT));
  for (const auto &weakObj : objects) {
    if (auto obj = weakObj.lock()) {
      if (auto surface = std::dynamic_pointer_cast<BezierC2>(obj)) {
        for (float u = 0; u <= 1.0f; u += Config::SAMPLING_DISTANCE) {
          for (float v = 0; v <= 1.0f; v += Config::SAMPLING_DISTANCE) {
            math137::Vector3f pos = surface->getValue(u, v);
            auto [i, j] = Config::CoordinateToHeightMapIndex(pos.x(), pos.z());
            if (pos.y() > heightMap[i][j]) {
              heightMap[i][j] = pos.y();
            }
          }
        }
      }
    }
  }
  return heightMap;
}

std::vector<math137::Vector3f>
PathGenerator::generatePath(const std::unique_ptr<SceneManager> &manager) {
  std::vector<math137::Vector3f> pathPoints;
  auto objects = manager->getDrawableObjects();
  remapPoints(objects);
  auto heightMap = generateHeightMap(objects);

  for (uint32_t j = 0; j < Config::HEIGHT_MAP_RESOLUTION; j++) {
    if (j % 2 == 0) {
      for (uint32_t i = 0; i < Config::HEIGHT_MAP_RESOLUTION; i++) {
        // we need to find the height of the highest point that is in the radius
        // of the rough blade
        float maxHeight = 0.0f;
        for (int32_t k = -Config::ROUGH_BLADE_RADIUS;
             k <= Config::ROUGH_BLADE_RADIUS; k++) {
          for (int32_t l = -Config::ROUGH_BLADE_RADIUS;
               l <= Config::ROUGH_BLADE_RADIUS; l++) {
            int32_t ni = static_cast<int32_t>(i) + k;
            int32_t nj = static_cast<int32_t>(j) + l;
            if (ni >= 0 &&
                ni < static_cast<int32_t>(Config::HEIGHT_MAP_RESOLUTION) &&
                nj >= 0 &&
                nj < static_cast<int32_t>(Config::HEIGHT_MAP_RESOLUTION)) {
              float distance = sqrtf(static_cast<float>(k * k + l * l));
              if (distance <= Config::ROUGH_BLADE_RADIUS) {
                if (heightMap[ni][nj] > maxHeight) {
                  maxHeight = heightMap[ni][nj];
                }
              }
            }
          }
        }
        float x =
            ((static_cast<float>(i) / (Config::HEIGHT_MAP_RESOLUTION - 1)) *
             Config::BLOCK_WIDTH) -
            (Config::BLOCK_WIDTH / 2);
        float z =
            ((static_cast<float>(j) / (Config::HEIGHT_MAP_RESOLUTION - 1)) *
             Config::BLOCK_DEPTH) -
            (Config::BLOCK_DEPTH / 2);
        float y = maxHeight;
        pathPoints.emplace_back(math137::Vector3f(x, y, z));
      }
    } else {
      for (int32_t i = Config::HEIGHT_MAP_RESOLUTION - 1; i >= 0; i--) {
        float x =
            ((static_cast<float>(i) / (Config::HEIGHT_MAP_RESOLUTION - 1)) *
             Config::BLOCK_WIDTH) -
            (Config::BLOCK_WIDTH / 2);
        float z =
            ((static_cast<float>(j) / (Config::HEIGHT_MAP_RESOLUTION - 1)) *
             Config::BLOCK_DEPTH) -
            (Config::BLOCK_DEPTH / 2);
        float y = heightMap[i][j];
        pathPoints.emplace_back(math137::Vector3f(x, y, z));
      }
    }
  }

  return pathPoints;
}

std::vector<math137::Vector3f>
PathGenerator::generateBallPath(const std::unique_ptr<SceneManager> &manager) {
  std::vector<math137::Vector3f> pathPoints;
  auto objects = manager->getDrawableObjects();

  // More faithful to the referenced Rust solution:
  // 1) build full (u,v) sampling grid and record: pos, normal, trimmed flag
  // 2) mark initially valid samples (not trimmed, valid normal, facing up)
  // 3) conservative dilation: any initially-valid sample whose offset
  //    (pos + normal*radius) is within radius of any initially-invalid sample
  //    becomes invalid (this prevents the tool center from overlapping trimmed
  //    regions).
  // 4) extract connected components (8-neighbour) on the final valid grid.
  // 5) for each component emit a continuous stroke. Between components lift
  //    by TRAVEL_CLEARANCE and approach the next stroke.

  constexpr float eps = 1e-6f;
  for (const auto &weakObj : objects) {
    if (auto obj = weakObj.lock()) {
      if (auto surface = std::dynamic_pointer_cast<BezierC2>(obj)) {
        // build sampling coordinates
        std::vector<float> us;
        std::vector<float> vs;
        for (float u = 0.0f; u <= 1.0f + 1e-6f; u += Config::SAMPLING_DISTANCE)
          us.push_back(std::min(u, 1.0f));
        for (float v = 0.0f; v <= 1.0f + 1e-6f; v += Config::SAMPLING_DISTANCE)
          vs.push_back(std::min(v, 1.0f));

        int nu = static_cast<int>(us.size());
        int nv = static_cast<int>(vs.size());
        if (nu == 0 || nv == 0)
          continue;

        // record full sample info
        struct Sample {
          math137::Vector3f pos;
          math137::Vector3f offset; // pos + normal*radius (valid only if normal ok)
          bool trimmed = false;
          bool normal_ok = false;
        };

        std::vector<std::vector<Sample>> grid(nu, std::vector<Sample>(nv));

        for (int iu = 0; iu < nu; ++iu) {
          for (int iv = 0; iv < nv; ++iv) {
            float u = us[iu];
            float v = vs[iv];
            grid[iu][iv].trimmed = surface->isTrimmedUV(u, v);
            grid[iu][iv].pos = surface->getValue(u, v);
            math137::Vector3f du = surface->uDerivative(u, v);
            math137::Vector3f dv = surface->vDerivative(u, v);
            math137::Vector3f normal = math137::Vector3f::Cross(du, dv);
            float lenSq = normal.x() * normal.x() + normal.y() * normal.y() +
                          normal.z() * normal.z();
            if (lenSq >= eps) {
              normal.normalize();
              if (normal.y() > 1e-6f) {
                grid[iu][iv].normal_ok = true;
                grid[iu][iv].offset = grid[iu][iv].pos + (normal * Config::BALL_BLADE_RADIUS);
              }
            }
          }
        }

        // initially valid: not trimmed and normal_ok
        std::vector<std::vector<char>> valid(nu, std::vector<char>(nv, 0));
        for (int iu = 0; iu < nu; ++iu)
          for (int iv = 0; iv < nv; ++iv)
            valid[iu][iv] = (!grid[iu][iv].trimmed && grid[iu][iv].normal_ok) ? 1 : 0;

        // conservative dilation: any valid sample whose offset is within
        // BALL_BLADE_RADIUS of any invalid sample's pos becomes invalid.
        // This is O(N^2) but grids are typically small; it's simple and robust.
        float r = Config::BALL_BLADE_RADIUS;
        float r2 = r * r;
        for (int iu = 0; iu < nu; ++iu) {
          for (int iv = 0; iv < nv; ++iv) {
            if (!valid[iu][iv])
              continue;
            const math137::Vector3f &myOffset = grid[iu][iv].offset;
            bool becomes_invalid = false;
            for (int ju = 0; ju < nu && !becomes_invalid; ++ju) {
              for (int jv = 0; jv < nv; ++jv) {
                // consider any sample that is invalid initially (trimmed or normal_bad)
                if (valid[ju][jv])
                  continue;
                const math137::Vector3f &otherPos = grid[ju][jv].pos;
                float dx = myOffset.x() - otherPos.x();
                float dy = myOffset.y() - otherPos.y();
                float dz = myOffset.z() - otherPos.z();
                float dist2 = dx * dx + dy * dy + dz * dz;
                if (dist2 <= r2) {
                  becomes_invalid = true;
                  break;
                }
              }
            }
            if (becomes_invalid)
              valid[iu][iv] = 0;
          }
        }

        // Extract connected components (8-neighbour) on the final valid grid.
        std::vector<std::vector<char>> visited(nu, std::vector<char>(nv, 0));
        const int d8[8][2] = {{1, 0},  {-1, 0}, {0, 1},  {0, -1},
                              {1, 1},  {1, -1},  {-1, 1}, {-1, -1}};

        for (int iu = 0; iu < nu; ++iu) {
          for (int iv = 0; iv < nv; ++iv) {
            if (!valid[iu][iv] || visited[iu][iv])
              continue;

            // collect indices for this component
            std::vector<std::pair<int, int>> compIdx;
            std::queue<std::pair<int, int>> q;
            q.push({iu, iv});
            visited[iu][iv] = 1;
            while (!q.empty()) {
              auto cur = q.front();
              q.pop();
              int cu = cur.first;
              int cv = cur.second;
              compIdx.push_back(cur);
              for (int k = 0; k < 8; ++k) {
                int nuu = cu + d8[k][0];
                int nvv = cv + d8[k][1];
                if (nuu >= 0 && nuu < nu && nvv >= 0 && nvv < nv) {
                  if (!visited[nuu][nvv] && valid[nuu][nvv]) {
                    visited[nuu][nvv] = 1;
                    q.push({nuu, nvv});
                  }
                }
              }
            }

            if (compIdx.empty())
              continue;

            // Order component points: greedy nearest-neighbour on world offsets
            std::vector<math137::Vector3f> compPts;
            compPts.reserve(compIdx.size());
            for (auto &p : compIdx) {
              compPts.push_back(grid[p.first][p.second].offset);
            }

            // greedy nearest neighbour ordering
            std::vector<math137::Vector3f> ordered;
            ordered.reserve(compPts.size());
            std::vector<char> used(compPts.size(), 0);
            if (!compPts.empty()) {
              // start at the lexicographically smallest index to be deterministic
              size_t start = 0;
              for (size_t k = 1; k < compIdx.size(); ++k) {
                if (compIdx[k] < compIdx[start])
                  start = k;
              }
              ordered.push_back(compPts[start]);
              used[start] = 1;
              for (size_t steps = 1; steps < compPts.size(); ++steps) {
                size_t last = ordered.size() - 1;
                const auto &lp = ordered[last];
                float bestDist2 = std::numeric_limits<float>::infinity();
                size_t bestIdx = SIZE_MAX;
                for (size_t m = 0; m < compPts.size(); ++m) {
                  if (used[m])
                    continue;
                  float dx = lp.x() - compPts[m].x();
                  float dy = lp.y() - compPts[m].y();
                  float dz = lp.z() - compPts[m].z();
                  float d2 = dx * dx + dy * dy + dz * dz;
                  if (d2 < bestDist2) {
                    bestDist2 = d2;
                    bestIdx = m;
                  }
                }
                if (bestIdx == SIZE_MAX)
                  break;
                ordered.push_back(compPts[bestIdx]);
                used[bestIdx] = 1;
              }
            }

            // emit travel lift/approach if needed
            if (!ordered.empty()) {
              if (!pathPoints.empty()) {
                math137::Vector3f last = pathPoints.back();
                math137::Vector3f liftPoint = last;
                liftPoint.y(liftPoint.y() + Config::TRAVEL_CLEARANCE);
                pathPoints.emplace_back(liftPoint);

                math137::Vector3f approach = ordered.front();
                approach.y(approach.y() + Config::TRAVEL_CLEARANCE);
                pathPoints.emplace_back(approach);
              }

              // append ordered points (descent implicit)
              for (const auto &p : ordered)
                pathPoints.emplace_back(p);
            }
          }
        }
      }
    }
  }

  return pathPoints;
}
