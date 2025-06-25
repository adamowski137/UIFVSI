#include "IBSpline.hpp"
#include "Curve.hpp"
#include "MatrixUtils.hpp"
#include "Vector.hpp"
#include "imgui.h"
#include <cstdint>

IBSpline::IBSpline(const std::vector<std::weak_ptr<Object>> &points)
    : Curve(points) {
  name = "IBSpline " + std::to_string(s_itemCount++);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  setVertices();
}

void IBSpline::render(std::shared_ptr<Renderer> &renderer,
                      const math137::Vector4f &color) {
  if (m_points.size() < 3)
    return;
  glBindVertexArray(m_vao);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(color);
  glPatchParameteri(GL_PATCH_VERTICES, 4);
  renderer->setDegree(4);
  glDrawArrays(GL_PATCHES, 0, 4 * (m_points.size() - 1));
}

void IBSpline::renderFramebuffer(std::shared_ptr<Renderer> &renderer,
                                 unsigned int c) {
  if (m_points.size() < 3)
    return;
  glBindVertexArray(m_vao);
  renderer->setShader(m_type);
  renderer->setModel(getModel());
  renderer->setColor(c);
  glPatchParameteri(GL_PATCH_VERTICES, 4);
  renderer->setDegree(4);
  glDrawArrays(GL_PATCHES, 0, 4 * (m_points.size() - 1));
}

bool IBSpline::renderObjectMenu() {
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

void IBSpline::notify() { setVertices(); }

void IBSpline::recalculateModel() { m_update = false; }
void IBSpline::setVertices() {
  int n = m_points.size() - 1;
  if (n < 2)
    return;

  std::vector<float> dist;
  dist.reserve(n);
  const float eps = 0.00000001f;

  for (uint16_t i = 0; i < n; i++) {
    math137::Vector3f vd = (m_points[i + 1].lock()->getTranslation() -
                            m_points[i].lock()->getTranslation());
    float d = vd * vd;
    dist.push_back(d < eps ? eps : d);
  }

  std::vector<float> alpha;
  std::vector<float> beta;
  std::vector<math137::Vector3f> f;
  alpha.reserve(n - 2);
  beta.reserve(n - 2);
  f.reserve(n - 1);

  for (uint16_t i = 2; i < n; i++) {
    alpha.push_back(dist[i - 1] / (dist[i - 1] + dist[i]));
    beta.push_back(dist[i - 1] / (dist[i - 1] + dist[i - 2]));
  }

  for (int i = 1; i < n; i++) {
    math137::Vector3f lhs = (m_points[i + 1].lock()->getTranslation() -
                             m_points[i].lock()->getTranslation()) *
                            (1.f / dist[i]);
    math137::Vector3f rhs = (m_points[i].lock()->getTranslation() -
                             m_points[i - 1].lock()->getTranslation()) *
                            (1.f / dist[i - 1]);

    f.push_back((lhs - rhs) * (3.f / (dist[i - 1] + dist[i])));
  }
  std::vector<math137::Vector3f> c =
      math137::MatrixUtils::SolveTriDiag(beta, alpha, f);
  c.insert(c.begin(), math137::Vector3f());
  c.push_back(math137::Vector3f());

  std::vector<math137::Vector3f> a;
  a.reserve(n);
  std::vector<math137::Vector3f> b;
  b.reserve(n);
  std::vector<math137::Vector3f> d;
  d.reserve(n);

  for (uint16_t i = 0; i < n; i++) {
    math137::Vector3f a1 = m_points[i].lock()->getTranslation();
    math137::Vector3f a2 = m_points[i + 1].lock()->getTranslation();

    a.push_back(a1);
    d.push_back((c[i + 1] - c[i]) * (0.33333f / dist[i]));
    b.push_back((a2 - a1) * (1.f / dist[i]) -
                (c[i] * 2.f + c[i + 1]) * dist[i] * 0.3333f);
  }

  std::vector<float> val;
  val.reserve((n) * 12);

  for (uint16_t i = 0; i < n; i++) {
    float h = dist[i];
    math137::Vector3f A = a[i];
    math137::Vector3f B = b[i] * h;
    math137::Vector3f C = c[i] * h * h;
    math137::Vector3f D = d[i] * h * h * h;
    math137::Vector3f b0 = A;
    math137::Vector3f b1 = A + B * 0.33333f;
    math137::Vector3f b2 = A + B * 0.66667f + C * 0.33333f;
    math137::Vector3f b3 = A + B + C + D;

    val.push_back(b0.x());
    val.push_back(b0.y());
    val.push_back(b0.z());

    val.push_back(b1.x());
    val.push_back(b1.y());
    val.push_back(b1.z());

    val.push_back(b2.x());
    val.push_back(b2.y());
    val.push_back(b2.z());

    val.push_back(b3.x());
    val.push_back(b3.y());
    val.push_back(b3.z());
  }

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ARRAY_BUFFER, val.size() * sizeof(float), val.data(),
               GL_STATIC_DRAW);
}
void IBSpline::setEdges() const {}
