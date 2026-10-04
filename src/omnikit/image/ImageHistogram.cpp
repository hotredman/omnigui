#include "ImageHistogram.hpp"
#include <imgui.h>

#include <algorithm>
#include <cmath>

namespace OmniKit {

ImageHistogram::ImageHistogram()
    : m_bins(256, 0.0f)
{}

void ImageHistogram::Compute(const uint16_t* data, int width, int height) {
    const int nb = 256;
    m_bins.assign(nb, 0.0f);
    if (!data || width <= 0 || height <= 0) {
        m_minValue = 0.0f;
        m_maxValue = 1.0f;
        return;
    }
    const size_t total = static_cast<size_t>(width) * height;
    uint16_t mn = data[0], mx = data[0];
    for (size_t i = 1; i < total; ++i) {
        uint16_t v = data[i];
        if (v < mn) mn = v;
        if (v > mx) mx = v;
    }
    m_minValue = static_cast<float>(mn);
    m_maxValue = static_cast<float>(mx);
    float range = (mx > mn) ? static_cast<float>(mx - mn) : 1.0f;

    for (size_t i = 0; i < total; ++i) {
        uint16_t v = data[i];
        int b = static_cast<int>((static_cast<float>(v - mn) / range) * (nb - 1) + 0.5f);
        if (b < 0) b = 0;
        if (b >= nb) b = nb - 1;
        m_bins[b] += 1.0f;
    }
}

bool ImageHistogram::Render(const WindowLevelParams& currentParams, WindowLevelParams& outParams, float width, float height, const ImageHistogramStyle* style) {
    const ImageHistogramStyle& st = style ? *style : m_style;
    outParams = currentParams;
    bool changed = false;

    if (!HasData()) {
        ImGui::TextDisabled("(no histogram data)");
        return false;
    }

    if (width <= 0.0f) {
        width = ImGui::GetContentRegionAvail().x;
    }
    if (width < 20.0f) width = 20.0f;

    ImVec2 size(width, height);
    ImVec2 o = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##omni_hist", size);

    auto xToVal = [&](float x) {
        float t = (x - o.x) / size.x;
        t = std::clamp(t, 0.0f, 1.0f);
        return m_minValue + t * (m_maxValue - m_minValue);
    };
    auto valToX = [&](float v) {
        float t = (v - m_minValue) / (m_maxValue - m_minValue);
        t = std::clamp(t, 0.0f, 1.0f);
        return o.x + t * size.x;
    };

    float lo = currentParams.level - currentParams.window * 0.5f;
    float hi = currentParams.level + currentParams.window * 0.5f;
    float xlo = valToX(lo);
    float xhi = valToX(hi);
    const float grab = 6.0f;
    const float mx = ImGui::GetIO().MousePos.x;

    bool nearLo = std::fabs(mx - xlo) <= grab;
    bool nearHi = std::fabs(mx - xhi) <= grab;
    bool overBand = (mx > xlo && mx < xhi);

    if (ImGui::IsItemActivated()) {
        if (nearLo)        m_dragMode = 1;
        else if (nearHi)   m_dragMode = 2;
        else if (overBand) m_dragMode = 3;
        else               m_dragMode = 0;
        m_dragLo = lo;
        m_dragHi = hi;
        m_dragVal = xToVal(mx);
    }

    if (ImGui::IsItemActive() && m_dragMode != 0) {
        float v = xToVal(mx);
        float nlo = lo, nhi = hi;
        if (m_dragMode == 1) {
            nlo = v;
            if (nlo > nhi - 1.0f) nlo = nhi - 1.0f;
        } else if (m_dragMode == 2) {
            nhi = v;
            if (nhi < nlo + 1.0f) nhi = nlo + 1.0f;
        } else if (m_dragMode == 3) {
            float w = m_dragHi - m_dragLo;
            float dv = v - m_dragVal;
            nlo = m_dragLo + dv;
            nhi = nlo + w;
            if (nlo < m_minValue) { nlo = m_minValue; nhi = nlo + w; }
            if (nhi > m_maxValue) { nhi = m_maxValue; nlo = nhi - w; }
        }
        outParams.window = nhi - nlo;
        outParams.level = (nlo + nhi) * 0.5f;
        changed = true;
        lo = nlo;
        hi = nhi;
        xlo = valToX(lo);
        xhi = valToX(hi);
    }
    if (!ImGui::IsItemActive()) {
        m_dragMode = 0;
    }

    if (ImGui::IsItemActive()) {
        ImGui::SetMouseCursor(m_dragMode == 3 ? ImGuiMouseCursor_Hand : ImGuiMouseCursor_ResizeEW);
    } else if (ImGui::IsItemHovered()) {
        if (nearLo || nearHi) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        else if (overBand)    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }

    // Drawing
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Dark container background
    dl->AddRectFilled(o, ImVec2(o.x + size.x, o.y + size.y), st.colBg, st.Scale(st.cornerRadius));

    int n = static_cast<int>(m_bins.size());
    float maxv = 0.0f;
    for (int i = 0; i < n; ++i) {
        float s = std::sqrt(m_bins[i]);
        if (s > maxv) maxv = s;
    }
    if (maxv <= 0.0f) maxv = 1.0f;

    // Draw bins
    for (int i = 0; i < n; ++i) {
        float h = std::sqrt(m_bins[i]) / maxv * (size.y - 4.0f);
        float x0 = o.x + static_cast<float>(i) / n * size.x;
        float x1 = o.x + static_cast<float>(i + 1) / n * size.x;
        dl->AddRectFilled(ImVec2(x0, o.y + size.y - h), ImVec2(x1, o.y + size.y), st.colBins);
    }

    // Draw active window band
    float bandMinX = std::max(o.x, std::min(xlo, xhi));
    float bandMaxX = std::min(o.x + size.x, std::max(xlo, xhi));
    if (bandMaxX > bandMinX) {
        dl->AddRectFilled(ImVec2(bandMinX, o.y), ImVec2(bandMaxX, o.y + size.y), st.colBand);
    }

    // Grab lines
    dl->AddLine(ImVec2(xlo, o.y), ImVec2(xlo, o.y + size.y), st.colLines, 2.0f * st.scale);
    dl->AddLine(ImVec2(xhi, o.y), ImVec2(xhi, o.y + size.y), st.colLines, 2.0f * st.scale);

    // Grab handles (triangles pointing down)
    float handleSize = 4.0f * st.scale;
    float handleDepth = 7.0f * st.scale;
    dl->AddTriangleFilled(ImVec2(xlo - handleSize, o.y), ImVec2(xlo + handleSize, o.y), ImVec2(xlo, o.y + handleDepth), st.colHandles);
    dl->AddTriangleFilled(ImVec2(xhi - handleSize, o.y), ImVec2(xhi + handleSize, o.y), ImVec2(xhi, o.y + handleDepth), st.colHandles);

    // Outer border
    dl->AddRect(o, ImVec2(o.x + size.x, o.y + size.y), st.colBorder, st.Scale(st.cornerRadius));

    return changed;
}

} // namespace OmniKit
