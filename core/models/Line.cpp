#include "Line.hpp"
#include "Vector.hpp"
#include "imgui.h"
#include <GL/glew.h>
#include <cstdint>
#include <memory>
#include <pstl/glue_algorithm_defs.h>
#include <stack>
#include <vector>

uint16_t Line::s_count = 0;

Line::Line() {
  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ebo);
  name = "Line " + std::to_string(s_count++);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

  setVertexData();
}

void Line::setVertexData() const {
  std::vector<float> points;
  points.reserve(m_points.size() * 3);
  std::stack<uint16_t> deleted;
  for (uint16_t i = 0; i < m_points.size(); i++) {
    if (std::shared_ptr<Point> sp = m_points[i].lock()) {
      const math137::Vector3f &pos = sp->getPosition();
      points.push_back(pos.x());
      points.push_back(pos.y());
      points.push_back(pos.z());
    } else {
      deleted.push(i);
    }
  }

  while (!deleted.empty()) {
    points[deleted.top()] = points.back();
    points.pop_back();
    deleted.pop();
  }

  if (m_points.size() < 2) {
    return;
  }

  std::vector<uint16_t> indices;
  indices.reserve((m_points.size() - 1) * 2);
  for (uint16_t i = 0; i < m_points.size() - 1; i++) {
    indices.push_back(i);
    indices.push_back(i + 1);
  }

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(),
               GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t),
               indices.data(), GL_STATIC_DRAW);
}

void Line::render() const {
  if (m_points.size() < 2)
    return;
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glDrawElements(GL_LINES, (m_points.size() - 1) * 2, GL_UNSIGNED_SHORT, 0);
}

void Line::addPoint(std::shared_ptr<Point> p) {
  m_points.push_back(p);
  setVertexData();
}

void Line::removePoint(const std::shared_ptr<Point> &p) {
  for (int i = 0; i < m_points.size(); i++) {
    if (m_points[i].lock() == p) {
      m_points[i] = m_points.back();
      m_points.pop_back();
    }
  }
  setVertexData();
}

bool Line::containsPoint(const std::shared_ptr<Point> &p) const {
  // auto it = std::find(
  //     m_points.begin(), m_points.end(),
  //     [&p](const std::weak_ptr<Point> &p2) { return p2.lock() == p; });
  // return it != m_points.end();
  return false;
}

void Line::renderObjectMenu() {
  int deleteIndex = -1;
  for (int i = 0; i < m_points.size(); i++) {
    std::shared_ptr<Point> point = m_points[i].lock();
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
          setVertexData();
        }
      }
      ImGui::EndDragDropTarget();
    }

    ImGui::PopID();
  }
  if (deleteIndex != -1)
    m_points.erase(m_points.begin() + deleteIndex);
}
