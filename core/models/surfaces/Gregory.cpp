#include "Gregory.hpp"
#include "MatrixUtils.hpp"
#include "Vector.hpp"
#include "imgui.h"
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

Gregory::Gregory(
    const std::array<std::array<std::weak_ptr<Object>, 4>, 3> &edges,
    const std::array<std::array<std::weak_ptr<Object>, 4>, 3> &prev)
    : m_edges(edges), m_prev(prev), m_uSubdivisions(4), m_vSubdivisions(4),
      Object(ShaderType::GREGORY) {
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  setVertices();
  setEdges();
  name = ("Gregory" + std::to_string(m_id));
}
math137::Vector3f Gregory::lerp(const math137::Vector3f &a,
                                const math137::Vector3f &b, float t) {
  return a * (1 - t) + b * t;
}

std::pair<std::array<math137::Vector3f, 4>, std::array<math137::Vector3f, 4>>
Gregory::divideCurve(float t,
                     const std::array<std::weak_ptr<Object>, 4> &points) {
  math137::Vector3f p0 = points[0].lock()->getTranslation();
  math137::Vector3f p1 = points[1].lock()->getTranslation();
  math137::Vector3f p2 = points[2].lock()->getTranslation();
  math137::Vector3f p3 = points[3].lock()->getTranslation();

  math137::Vector3f A = lerp(p0, p1, t);
  math137::Vector3f B = lerp(p1, p2, t);
  math137::Vector3f C = lerp(p2, p3, t);

  math137::Vector3f D = lerp(A, B, t);
  math137::Vector3f E = lerp(B, C, t);
  math137::Vector3f F = lerp(D, E, t);

  return {{p0, A, D, F}, {F, E, C, p3}};
}

bool Gregory::renderObjectMenu() {
  if (!m_openMenu)
    return false;
  ImGui::Begin(("Settings" + name).c_str(), &m_openMenu);
  static const char *modes[] = {"Surface", "Grid"};
  static int mode = 0;
  if (ImGui::Combo("Mode", &mode, modes, IM_ARRAYSIZE(modes))) {
    if (mode == 0)
      m_type = ShaderType::GREGORY;
    else
      m_type = ShaderType::OBJECT;
    setEdges();
  }
  ImGui::SliderInt("U subdivisions", &m_uSubdivisions, 1, 10);
  ImGui::SliderInt("V subdivisions", &m_vSubdivisions, 1, 10);
  ImGui::End();
  return false;
}

math137::Vector3f Gregory::getMassCenter() {
  math137::Vector3f res;
  for (const auto &v1 : m_edges)
    for (const auto &p : v1)
      res = res + p.lock()->getMassCenter();

  for (const auto &v1 : m_prev)
    for (const auto &p : v1)
      res = res + p.lock()->getMassCenter();

  return res / (m_edges.size() * 8);
}

void Gregory::setVertices() {
  math137::Vector3f p3[3], p2[3], p1[3], q[3];
  std::array<std::array<math137::Vector3f, 4>, 6> edges;
  std::array<std::array<math137::Vector3f, 2>, 6> next;

  float dt = 1.f;
  for (uint16_t i = 0; i < 3; i++) {
    const auto [e1, e2] = divideCurve(0.5f, m_edges[i]);
    const auto [prev1, prev2] = divideCurve(0.5f, m_prev[i]);
    edges[2 * i] = e1;
    edges[2 * i + 1] = e2;
    next[2 * i][0] = e1[1] + (e1[1] - prev1[1]) * dt;
    next[2 * i][1] = e1[2] + (e1[2] - prev1[2]) * dt;
    next[2 * i + 1][0] = e2[1] + (e2[1] - prev2[1]) * dt;
    next[2 * i + 1][1] = e2[2] + (e2[2] - prev2[2]) * dt;
    p3[i] = e2[0];
    p2[i] = e2[0] + (e2[0] - prev2[0]) * dt;
    q[i] = (p2[i] * 3.f - p3[i]) * 0.5f;
  }

  math137::Vector3f p0 = (q[0] + q[1] + q[2]) / 3.f;
  for (uint16_t i = 0; i < 3; i++) {
    p1[i] = (q[i] * 2 + p0) / 3;
  }
  std::array<std::array<math137::Vector3f, 4>, 3> middle = {
      std::array<math137::Vector3f, 4>{p3[0], p2[0], p1[0], p0},
      {p3[1], p2[1], p1[1], p0},
      {p3[2], p2[2], p1[2], p0}};
  std::array<std::array<math137::Vector3f, 20>, 3> res;

  for (uint16_t i = 0; i < 3; i++) {
    std::array<math137::Vector3f, 4> vDif = {
        next[(2 * i + 1) % 6][0],
        next[(2 * i + 1) % 6][1],
        next[(2 * i + 1) % 6][0] + (middle[i][2] - middle[i][1]),
        next[(2 * i + 2) % 6][1],
    };
    std::array<math137::Vector3f, 4> uDif = {
        next[2 * i + 1][0],
        next[(2 * i + 2) % 6][0],
        next[(2 * i + 2) % 6][0] +
            (middle[(i + 1) % 3][2] - middle[(i + 1) % 3][1]),
        next[(2 * i + 2) % 6][1],
    };
    std::array<math137::Vector3f, 20> val = {
        edges[2 * i + 1][0],
        edges[2 * i + 1][1],
        edges[2 * i + 1][2],
        edges[2 * i + 1][3],
        middle[i][1],
        edges[(2 * i + 2) % 6][1],
        middle[i][2],
        edges[(2 * i + 2) % 6][2],
        middle[(i + 1) % 3][3],
        middle[(i + 1) % 3][2],
        middle[(i + 1) % 3][1],
        middle[(i + 1) % 3][0],
        uDif[0],
        uDif[1],
        uDif[2],
        uDif[3],
        vDif[0],
        vDif[1],
        vDif[2],
        vDif[3],
    };
    res[i] = val;
  }

  std::vector<float> vert;

  for (const auto &v : res) {
    for (const auto &vv : v) {
      vert.push_back(vv.x());
      vert.push_back(vv.y());
      vert.push_back(vv.z());
    }
  }

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, vert.size() * sizeof(float), vert.data(),
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Gregory::setEdges() {

  if (m_type == ShaderType::GREGORY) {

    std::vector<uint16_t> ind = {0, 4,  6,  8,  1,  9,  2,  10, 3,  5,
                                 7, 11, 12, 14, 13, 15, 16, 18, 17, 19};

    for (uint16_t i = 0; i < 2; i++) {
      for (uint16_t j = 0; j < 20; j++)
        ind.push_back((i + 1) * 20 + ind[j]);
    }

    for (uint16_t i = 0; i < 60; i++)
      ind.push_back(i);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof(uint16_t),
                 ind.data(), GL_STATIC_DRAW);
  } else {
    std::vector<uint16_t> ind = {// border 0
                                 0, 4, 4, 6, 6, 8,
                                 // border 1
                                 8, 9, 9, 10, 10, 11,
                                 // derivatives u
                                 4, 12, 5, 13, 6, 14, 7, 15,
                                 // derivatives v
                                 1, 16, 2, 17, 9, 18, 10, 19};
    size_t size = ind.size();
    for (uint16_t i = 0; i < 2; i++) {
      for (uint16_t j = 0; j < size; j++)
        ind.push_back((i + 1) * 20 + ind[j]);
    }
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof(uint16_t),
                 ind.data(), GL_STATIC_DRAW);
  }
}

void Gregory::render(std::shared_ptr<Renderer> &renderer,
                     const math137::Vector4f &color) {
  glBindVertexArray(m_vao);
  renderer->setShader(m_type);
  renderer->setModel(math137::MatrixUtils::Identity());
  renderer->setColor(color);
  if (m_type == ShaderType::GREGORY) {
    glPatchParameteri(GL_PATCH_VERTICES, 20);
    renderer->setUVSubdivisions(m_vSubdivisions, m_uSubdivisions);
    glDrawElements(GL_PATCHES, 60, GL_UNSIGNED_SHORT, (void *)0);
    renderer->setUVSubdivisions(m_uSubdivisions, m_vSubdivisions);
    glDrawElements(GL_PATCHES, 3 * 20, GL_UNSIGNED_SHORT,
                   (void *)(60 * sizeof(uint16_t)));
  } else {
    glDrawElements(GL_LINES, 3 * 28, GL_UNSIGNED_SHORT, (void *)0);
  }
}
