#include "BezierCurve.hpp"
#include "Vector.hpp"
#include "imgui.h"
#include <GL/glew.h>
#include <cstdint>
#include <numeric>
#include <vector>

BezierCurve::BezierCurve(const std::vector<std::weak_ptr<Object>> &points)
    : Curve(points) {
  name = "BezierCurve " + std::to_string(s_itemCount++);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  setVertices();
  setEdges();
}

void BezierCurve::setVertices() {
  std::vector<float> points;
  for (uint16_t i = 0; i < m_points.size(); i++) {
    if (std::shared_ptr<Object> sp = m_points[i].lock()) {
      const math137::Vector3f &pos = sp->getTranslation();
      points.push_back(pos.x());
      points.push_back(pos.y());
      points.push_back(pos.z());
    }
  }

  if (m_points.size() < 2) {
    return;
  }

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(),
               GL_STATIC_DRAW);
}

void BezierCurve::setEdges() const {
  if (m_points.size() < 2)
    return;
  std::vector<uint16_t> indices;
  if (m_type == ShaderType::CURVE) {
    indices.reserve((m_points.size() - 1) * 2);
    for (uint16_t i = 0; i < m_points.size(); i++) {
      indices.push_back(i);
      if (i != 0 && i != m_points.size() - 1 && i % 3 == 0)
        indices.push_back(i);
    }
  }
  if (m_type == ShaderType::OBJECT) {
    indices.resize(m_points.size());
    std::iota(indices.begin(), indices.end(), 0);
    indices.reserve((m_points.size() - 1) * 2);
  }

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t),
               indices.data(), GL_STATIC_DRAW);
}

void BezierCurve::renderFramebuffer(std::shared_ptr<Renderer> &renderer,
                                    unsigned int c) {
  if (m_points.size() < 2)
    return;
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(c);
  if (m_type == ShaderType::OBJECT) {
    glDrawArrays(GL_LINE_STRIP, 0, m_points.size());
  }
  if (m_type == ShaderType::CURVE) {
    uint16_t sizeFull = (m_points.size() - 1) / 3;
    uint16_t sizeMissing = (m_points.size() - 1) % 3;
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    renderer->setDegree(4);
    glDrawElements(GL_PATCHES, 4 * sizeFull, GL_UNSIGNED_SHORT, 0);
    glPatchParameteri(GL_PATCH_VERTICES, sizeMissing + 1);
    if (sizeMissing == 0)
      return;
    renderer->setDegree(sizeMissing + 1);
    glDrawElements(GL_PATCHES, sizeMissing + 1, GL_UNSIGNED_SHORT,
                   (void *)(4 * sizeFull * sizeof(uint16_t)));
  }
}

void BezierCurve::render(std::shared_ptr<Renderer> &renderer,
                         const math137::Vector4f &color) {
  if (m_points.size() < 2)
    return;
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(color);
  if (m_type == ShaderType::OBJECT) {
    glDrawArrays(GL_LINE_STRIP, 0, m_points.size());
  }
  if (m_type == ShaderType::CURVE) {
    uint16_t sizeFull = (m_points.size() - 1) / 3;
    uint16_t sizeMissing = (m_points.size() - 1) % 3;
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    renderer->setDegree(4);
    glDrawElements(GL_PATCHES, 4 * sizeFull, GL_UNSIGNED_SHORT, 0);
    glPatchParameteri(GL_PATCH_VERTICES, sizeMissing + 1);
    if (sizeMissing == 0)
      return;
    renderer->setDegree(sizeMissing + 1);
    glDrawElements(GL_PATCHES, sizeMissing + 1, GL_UNSIGNED_SHORT,
                   (void *)(4 * sizeFull * sizeof(uint16_t)));
  }
}

void BezierCurve::notify() {
  setVertices();
  setEdges();
}

bool BezierCurve::renderObjectMenu() {
  if (!m_openMenu)
    return false;
  static const char *modes[] = {"Curve", "Line"};

  static int idx = 0;
  ImGui::Begin(("Settings" + name).c_str(), &m_openMenu);
  setNameMenu();
  if (ImGui::Combo("Mode", &idx, modes, IM_ARRAYSIZE(modes))) {
    if (idx == 0)
      m_type = ShaderType::CURVE;
    if (idx == 1)
      m_type = ShaderType::OBJECT;
    setEdges();
  }
  ImGui::Separator();
  ImGui::Text("Points");
  int deleteIndex = -1;
  for (int i = 0; i < m_points.size(); i++) {
    std::shared_ptr<Object> point = m_points[i].lock();
    if (!point)
      continue;
    ImGui::PushID(i);

    ImGui::Selectable((name.c_str() + point->name).c_str(), false,
                      ImGuiSelectableFlags_AllowDoubleClick);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      deleteIndex = i;
    }

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
      ImGui::SetDragDropPayload("DND_POINT", &i, sizeof(int));
      ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("DND_POINT")) {
        int srcIndex = *(const int *)payload->Data;
        if (srcIndex != i) {
          std::swap(m_points[srcIndex], m_points[i]);
          setVertices();
          setEdges();
        }
      }
      ImGui::EndDragDropTarget();
    }

    ImGui::PopID();
  }
  ImGui::End();
  if (deleteIndex != -1)
    m_points.erase(m_points.begin() + deleteIndex);
  return false;
}
void BezierCurve::recalculateModel() { m_update = false; }
