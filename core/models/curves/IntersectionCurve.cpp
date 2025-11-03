#include "IntersectionCurve.hpp"
#include "../IntersectionUtils.hpp"
#include "Vector.hpp"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

IntersectionCurve::IntersectionCurve(const math137::Vector4f &start,
                                     const std::weak_ptr<Intersectable> &i1,
                                     const std::weak_ptr<Intersectable> &i2,
                                     float step)
    : m_step(step), m_intersectable1(i1), m_intersectable2(i2), m_start(start),
      Object(ShaderType::OBJECT) {
  name = "Intersection " + std::to_string(m_id);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glGenTextures(2, m_textureIds);
  for (uint8_t i = 0; i < 2; i++) {
    m_openPopup[i] = false;
    m_textureData[i].resize(m_width * m_height);
    glBindTexture(GL_TEXTURE_2D, m_textureIds[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_width, m_height, 0, GL_RED,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  setVertices();
  setTextures();
}

IntersectionCurve::~IntersectionCurve() { // glDeleteTextures(2, m_textureIds);
}

void IntersectionCurve::setVertices() {
  auto [l1, p1] = IntersectionUtils::GenerateIntersectionPoints(
      m_intersectable1.lock(), m_intersectable2.lock(), m_start, m_step, false);
  std::cout << "s1: " << p1.size() << std::endl;
  std::reverse_copy(p1.begin(), p1.end(), std::back_inserter(m_params));
  if (!l1) {
    auto [l2, p2] = IntersectionUtils::GenerateIntersectionPoints(
        m_intersectable1.lock(), m_intersectable2.lock(), m_start, m_step,
        true);
    std::cout << "s2: " << p2.size() << std::endl;

    m_params.insert(m_params.end(), p2.begin(), p2.end());
  }

  std::vector<float> verts;
  verts.reserve(m_params.size() * 3);
  for (const auto &par : m_params) {
    math137::Vector3f p = m_intersectable1.lock()->getValue(par.x(), par.y());

    verts.push_back(p.x());
    verts.push_back(p.y());
    verts.push_back(p.z());
  }
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(),
               GL_STATIC_DRAW);
}

void IntersectionCurve::setTextures() {
  for (uint8_t i = 0; i < 2; i++) {
    for (uint16_t j = 1; j < m_params.size(); j++) {
      math137::Vector4f vec = m_params[j - 1];
      math137::Vector4f vec2 = m_params[j];

      float u1 = vec[2 * i];
      float v1 = vec[2 * i + 1];
      float u2 = vec2[2 * i];
      float v2 = vec2[2 * i + 1];

      float du = u2 - u1;
      float dv = v2 - v1;

      // Sprawdź, czy trzeba wrapować w poziomie lub pionie
      bool wrapU = fabsf(du) > 0.5f;
      bool wrapV = fabsf(dv) > 0.5f;

      if (wrapU || wrapV) {
        // Wyznacz kierunek wrapowania i połowę linii przez krawędź
        float midU1 = u1;
        float midV1 = v1;
        float midU2 = u2;
        float midV2 = v2;

        // U wrap
        if (wrapU) {
          if (du > 0) {
            midU1 = 1.0f;
            midU2 = 0.0f;
          } else {
            midU1 = 0.0f;
            midU2 = 1.0f;
          }
          midV1 = v1 + (midU1 - u1) * dv / du;
          midV2 = v2 + (midU2 - u2) * dv / du;
        }
        // V wrap
        if (wrapV) {
          if (dv > 0) {
            midV1 = 1.0f;
            midV2 = 0.0f;
          } else {
            midV1 = 0.0f;
            midV2 = 1.0f;
          }
          midU1 = u1 + (midV1 - v1) * du / dv;
          midU2 = u2 + (midV2 - v2) * du / dv;
        }

        // Rysowanie dwóch linii (wrap)
        int y1 = std::clamp((int)(midU2 * m_height), 0, m_height - 1);
        int x1 = std::clamp((int)(midV2 * m_width), 0, m_width - 1);
        int y2 = std::clamp((int)(u1 * m_height), 0, m_height - 1);
        int x2 = std::clamp((int)(v1 * m_width), 0, m_width - 1);
        bresenhamAlgorithm(x1, y1, x2, y2, m_textureData[i]);

        y1 = std::clamp((int)(midU1 * m_height), 0, m_height - 1);
        x1 = std::clamp((int)(midV1 * m_width), 0, m_width - 1);
        y2 = std::clamp((int)(u2 * m_height), 0, m_height - 1);
        x2 = std::clamp((int)(v2 * m_width), 0, m_width - 1);
        bresenhamAlgorithm(x1, y1, x2, y2, m_textureData[i]);
      } else {
        // Normalne rysowanie
        int x1 = std::clamp((int)(u1 * m_height), 0, m_height - 1);
        int y1 = std::clamp((int)(v1 * m_width), 0, m_width - 1);
        int x2 = std::clamp((int)(u2 * m_height), 0, m_height - 1);
        int y2 = std::clamp((int)(v2 * m_width), 0, m_width - 1);
        bresenhamAlgorithm(y1, x1, y2, x2, m_textureData[i]);
      }
    }

    glBindTexture(GL_TEXTURE_2D, m_textureIds[i]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RED,
                    GL_UNSIGNED_BYTE, m_textureData[i].data());
  }
}

void IntersectionCurve::renderFramebuffer(std::shared_ptr<Renderer> &renderer,
                                          unsigned int c) {
  glBindVertexArray(m_vao);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(c);
  glDrawArrays(GL_LINE_STRIP, 0, m_params.size());
}
void IntersectionCurve::putPixel(uint16_t x, uint16_t y,
                                 std::vector<uint8_t> &data) {
  data[y * m_width + x] = 255;
}

void IntersectionCurve::bresenhamAlgorithm(uint16_t x0, uint16_t y0,
                                           uint16_t x1, uint16_t y1,
                                           std::vector<uint8_t> &data) {
  bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
  if (steep) {
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }
  int dx = x1 - x0;
  int dy = std::abs(y1 - y0);
  int error = dx / 2;
  int ystep = (y0 < y1) ? 1 : -1;
  int y = y0;

  for (int x = x0; x <= x1; ++x) {
    if (steep)
      putPixel(y, x, data); // jeśli zamienione osie
    else
      putPixel(x, y, data);

    error -= dy;
    if (error < 0) {
      y += ystep;
      error += dx;
    }
  }
}

void IntersectionCurve::render(std::shared_ptr<Renderer> &renderer,
                               const math137::Vector4f &color) {
  glBindVertexArray(m_vao);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(color);
  glDrawArrays(GL_LINE_STRIP, 0, m_params.size());
}
bool IntersectionCurve::renderObjectMenu() {
  if (m_openMenu) {
    ImGui::Begin(("Intersection Settings " + std::to_string(m_id)).c_str(),
                 &m_openMenu);
    if (ImGui::Button("Show Parameters 1"))
      m_openPopup[0] = true;
    if (ImGui::Button("Show Parameters 2"))
      m_openPopup[1] = true;
    ImGui::End();
  }
  if (m_openPopup[0]) {
    ImGui::Begin(
        ("Parameters Space Object 1 ###1" + std::to_string(m_id)).c_str(),
        &m_openPopup[0]);
    ImGui::Image((ImTextureID)(intptr_t)m_textureIds[0],
                 ImVec2(m_width, m_height));
    ImVec2 imageMin = ImGui::GetItemRectMin();
    ImVec2 imageMax = ImGui::GetItemRectMax();
    ImVec2 mousePos = ImGui::GetIO().MousePos;
    static int pos[2];
    if (ImGui::IsItemHovered() &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      float localX = mousePos.x - imageMin.x;
      float localY = mousePos.y - imageMin.y;
      pos[0] = localX;
      pos[1] = localY;
    }

    ImGui::InputInt2("Pixel ", pos);
    if (ImGui::Button("Union Trim 1")) {
      if (auto s = m_intersectable1.lock())
        s->unionTrimmingTexture(m_textureData[0], pos[0], pos[1]);
    }
    ImGui::SameLine();
    if (ImGui::Button("Intersect Trim 1")) {
      if (auto s = m_intersectable1.lock())
        s->intersectTrimmingTexture(m_textureData[0], pos[0], pos[1]);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset trimming")) {
      if (auto s = m_intersectable1.lock())
        s->resetTrimming();
    }

    ImGui::End();
  }
  if (m_openPopup[1]) {
    ImGui::Begin(
        ("Parameters Space Object 2 ###2" + std::to_string(m_id)).c_str(),
        &m_openPopup[1]);
    ImGui::Image((ImTextureID)(intptr_t)m_textureIds[1],
                 ImVec2(m_width, m_height));
    ImVec2 imageMin = ImGui::GetItemRectMin();
    ImVec2 imageMax = ImGui::GetItemRectMax();
    ImVec2 mousePos = ImGui::GetIO().MousePos;
    static int pos[2];
    if (ImGui::IsItemHovered() &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      float localX = mousePos.x - imageMin.x;
      float localY = mousePos.y - imageMin.y;
      pos[0] = localX;
      pos[1] = localY;
    }

    ImGui::InputInt2("Pixel ", pos);
    if (ImGui::Button("Union Trim 2")) {
      if (auto s = m_intersectable2.lock())
        s->unionTrimmingTexture(m_textureData[1], pos[0], pos[1]);
    }
    ImGui::SameLine();
    if (ImGui::Button("Intersect Trim 2")) {
      if (auto s = m_intersectable2.lock())
        s->intersectTrimmingTexture(m_textureData[1], pos[0], pos[1]);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset trimming")) {
      if (auto s = m_intersectable2.lock())
        s->resetTrimming();
    }
    ImGui::End();
  }
  return false;
}
std::vector<math137::Vector3f> IntersectionCurve::getPoints() const {
  std::vector<math137::Vector3f> res;
  for (const auto &p : m_params)
    res.push_back(m_intersectable1.lock()->getValue(p.x(), p.y()));
  return res;
}
