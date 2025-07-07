#include "BSpline.hpp"
#include "../ObjectBuilder.hpp"
#include "Vector.hpp"
#include "imgui.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

BSpline::BSpline(const std::vector<std::weak_ptr<Object>> &points)
    : Curve(points) {
  name = "BSpline " + std::to_string(m_id);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  for (int i = 0; i < (m_points.size() - 3) * 4; i++)
    m_bernsteinPoints.push_back(ObjectBuilder().withNewPoint().build());
  setVertices();
}

void BSpline::setVertices() {
  if (m_points.size() < 4) {
    return;
  }
  std::vector<math137::Vector3f> points;
  for (uint16_t i = 0; i < m_points.size() - 1; i++) {
    std::shared_ptr<Object> bp0 = m_points[i].lock();
    std::shared_ptr<Object> bp1 = m_points[i + 1].lock();
    if (bp0.get() != nullptr && bp1.get() != nullptr) {
      const math137::Vector3f b0 = bp0->getTranslation();
      const math137::Vector3f b1 = bp1->getTranslation();

      points.emplace_back();
      points.emplace_back((b0 * 2.f + b1) * 0.3333f);
      points.emplace_back((b1 * 2.f + b0) * 0.3333f);
      points.emplace_back();
    }
  }
  for (int i = 1; i < m_points.size() - 2; i++) {
    points[4 * i] = (points[4 * (i - 1) + 2] + points[4 * i + 1]) * 0.5f;
    points[4 * i + 3] = (points[4 * i + 2] + points[4 * (i + 1) + 1]) * 0.5f;
  }

  std::vector<float> res;
  for (int i = 1; i < m_points.size() - 2; i++) {

    res.push_back(points[4 * i].x());
    res.push_back(points[4 * i].y());
    res.push_back(points[4 * i].z());

    res.push_back(points[4 * i + 1].x());
    res.push_back(points[4 * i + 1].y());
    res.push_back(points[4 * i + 1].z());

    res.push_back(points[4 * i + 2].x());
    res.push_back(points[4 * i + 2].y());
    res.push_back(points[4 * i + 2].z());

    res.push_back(points[4 * i + 3].x());
    res.push_back(points[4 * i + 3].y());
    res.push_back(points[4 * i + 3].z());
    m_bernsteinPoints[4 * (i - 1)]->setTranslation(points[(4 * i)]);
    m_bernsteinPoints[4 * (i - 1) + 1]->setTranslation(points[(4 * i) + 1]);
    m_bernsteinPoints[4 * (i - 1) + 2]->setTranslation(points[(4 * i) + 2]);
    m_bernsteinPoints[4 * (i - 1) + 3]->setTranslation(points[(4 * i) + 3]);
  }

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ARRAY_BUFFER, res.size() * sizeof(float), res.data(),
               GL_STATIC_DRAW);
}

void BSpline::setEdges() const {}

std::vector<std::weak_ptr<Object>> BSpline::getVirtualObjects() const {
  std::vector<std::weak_ptr<Object>> t(m_bernsteinPoints.begin(),
                                       m_bernsteinPoints.end());

  return t;
}

void BSpline::render(std::shared_ptr<Renderer> &renderer,
                     const math137::Vector4f &color) {
  if (m_points.size() < 4)
    return;
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(color);
  glPatchParameteri(GL_PATCH_VERTICES, 4);
  renderer->setDegree(4);
  glDrawArrays(GL_PATCHES, 0, 4 * (m_points.size() - 3));
}

void BSpline::renderFramebuffer(std::shared_ptr<Renderer> &renderer,
                                unsigned int c) {
  if (m_points.size() < 4)
    return;
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(c);
  glPatchParameteri(GL_PATCH_VERTICES, 4);
  renderer->setDegree(4);
  glDrawArrays(GL_PATCHES, 0, 4 * (m_points.size() - 3));
}

void BSpline::notify() { setVertices(); }

void BSpline::addPoint(const std::weak_ptr<Object> &p) {
  Curve::addPoint(p);
  ObjectBuilder ob;
  m_bernsteinPoints.push_back(ob.withNewPoint().build());
  m_bernsteinPoints.push_back(ob.withNewPoint().build());
  m_bernsteinPoints.push_back(ob.withNewPoint().build());
  m_bernsteinPoints.push_back(ob.withNewPoint().build());
}

void BSpline::removePoint(const std::weak_ptr<Object> &p) {
  Curve::removePoint(p);
  if (m_bernsteinPoints.size() < 4)
    return;
  m_bernsteinPoints.pop_back();
  m_bernsteinPoints.pop_back();
  m_bernsteinPoints.pop_back();
  m_bernsteinPoints.pop_back();
}

bool BSpline::renderObjectMenu() {
  if (!m_openMenu)
    return false;
  static int idx = 0;
  ImGui::Begin(("Settings" + name).c_str(), &m_openMenu);
  setNameMenu();
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
void BSpline::recalculateModel() { m_update = false; }
void BSpline::notifyVirtual(uint16_t i, const math137::Vector3f &pos) {
  int segment = i == 0 ? 0 : (i - 1) / 3;
  int node = i == 0 ? 0 : (i - 1) % 3 + 1;
  if (i % 3 == 0) {
    m_points[i / 3 + 1].lock()->setTranslation(
        m_points[i / 3 + 1].lock()->getTranslation() + pos * 1.5f);
  } else {
    m_points[segment + 1].lock()->setTranslation(
        m_points[segment + 1].lock()->getTranslation() + pos);
    m_points[segment + 2].lock()->setTranslation(
        m_points[segment + 2].lock()->getTranslation() + pos);
  }
  setVertices();
}
