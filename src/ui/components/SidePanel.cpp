#include "ui/components/SidePanel.hpp"
#include <algorithm>
#include <cmath>

SidePanel::SidePanel(const SidePanelOptions& options) {
    Init(options, options.width, nullptr, false);
}

SidePanel::SidePanel(float& width, const SidePanelOptions& options) {
    Init(options, width, &width, true);
}

void SidePanel::Init(const SidePanelOptions& options, float width, float* widthRef, bool resizable) {
    const UiTheme& theme = UiTheme::Get();
    m_id = options.key ? options.key : "##SidePanel";
    m_width = width;
    m_widthRef = widthRef;
    m_side = options.side;
    m_resizable = resizable;
    m_minWidth = resizable ? options.minWidth : width;
    m_maxWidth = resizable ? options.maxWidth : width;
    m_style = options.style ? *options.style : theme.sidePanel;

    // 1. Для правой панели со сплиттером сначала отрисовываем сплиттер слева от панели
    if (m_resizable && m_side == Side::Right) {
        RenderSplitter();
    }

    // 2. Открытие дочернего региона панели
    float panelW = theme.Scale(m_width);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;

    if ((m_style.colBg & IM_COL32_A_MASK) != 0) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, m_style.colBg);
        m_styleColorPushes++;
    }
    if (m_style.borderSize > 0.0f) {
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, m_style.borderSize);
        m_styleVarPushes++;
    }
    if (m_style.padding.x > 0.0f || m_style.padding.y > 0.0f) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, 
                            ImVec2(theme.Scale(m_style.padding.x), theme.Scale(m_style.padding.y)));
        m_styleVarPushes++;
    }

    m_open = ImGui::BeginChild(m_id.c_str(), ImVec2(panelW, 0.0f), m_style.borderSize > 0.0f, flags);
    m_childStarted = true;
    // Подложка, рамка и отступы применены к окну панели; содержимое их не наследует
    if (m_styleColorPushes > 0) ImGui::PopStyleColor(m_styleColorPushes);
    if (m_styleVarPushes > 0) ImGui::PopStyleVar(m_styleVarPushes);
    m_styleColorPushes = 0;
    m_styleVarPushes = 0;
}

SidePanel::~SidePanel() {
    End();
}

void SidePanel::End() {
    if (!m_childStarted) return;
    m_childStarted = false;

    // В Dear ImGui для каждого BeginChild() ОБЯЗАТЕЛЕН вызов EndChild(),
    // независимо от возвращенного значения (true/false)
    ImGui::EndChild();

    // Для левой панели со сплиттером сплиттер отрисовывается справа от панели
    if (m_resizable && m_side == Side::Left) {
        ImGui::SameLine(0.0f, 0.0f);
        RenderSplitter();
    }

    m_open = false;
}

void SidePanel::RenderSplitter() {
    const UiTheme& theme = UiTheme::Get();
    float splitterW = theme.Scale(m_style.splitterWidth);
    float availH = ImGui::GetContentRegionAvail().y;

    ImGui::PushID(m_id.c_str());
    ImGui::InvisibleButton("##splitter", ImVec2(splitterW, availH));
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();

    if (hovered || active) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }

    if (active && m_widthRef) {
        float scale = std::max(0.1f, theme.GetScale());
        float unscaledDelta = ImGui::GetIO().MouseDelta.x / scale;

        if (m_side == Side::Right) {
            *m_widthRef -= unscaledDelta;
        } else {
            *m_widthRef += unscaledDelta;
        }
        *m_widthRef = std::clamp(*m_widthRef, m_minWidth, m_maxWidth);
        m_width = *m_widthRef;
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 minPos = ImGui::GetItemRectMin();
    ImVec2 maxPos = ImGui::GetItemRectMax();
    float midX = std::floor((minPos.x + maxPos.x) * 0.5f);

    ImU32 col = active ? m_style.colSplitterActive 
                       : (hovered ? m_style.colSplitterHover : m_style.colSplitter);
    float lineW = theme.Scale(m_style.splitterLineWidth);
    if (active || hovered) {
        lineW = std::max(lineW, theme.Scale(2.0f));
    }
    dl->AddLine(ImVec2(midX, minPos.y), ImVec2(midX, maxPos.y), col, lineW);

    ImGui::PopID();
    ImGui::SameLine(0.0f, 0.0f);
}

float SidePanel::GetTotalWidth() const {
    return CalcTotalWidth(m_width, m_resizable, m_style);
}

float SidePanel::CalcTotalWidth(float width, bool resizable, const SidePanelStyle& style) {
    const UiTheme& theme = UiTheme::Get();
    float total = theme.Scale(width);
    if (resizable) {
        total += theme.Scale(style.splitterWidth);
    }
    return total;
}
