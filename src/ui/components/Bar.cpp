#include "ui/components/Bar.hpp"
#include "ui/components/Button.hpp"
#include <algorithm>
#include <cmath>

// ----------------------------------------------------------------------------
// Bar
// ----------------------------------------------------------------------------

Bar::Bar(const BarMetrics& metrics, const char* windowName, const char* zonesId,
         float posY, float height)
{
    const UiTheme& theme = UiTheme::Get();
    ImGuiIO& io = ImGui::GetIO();

    m_height = (height > 0.0f) ? height : theme.Scale(metrics.height);

    m_layout.paddingX = theme.Scale(metrics.paddingX);
    m_layout.paddingY = theme.Scale(metrics.paddingY);
    m_layout.zoneSpacing = theme.Scale(metrics.zoneSpacing);
    m_layout.baseContentHeight = metrics.height - metrics.paddingY * 2.0f;
    m_layout.contentHeight = std::max(0.0f, m_height - m_layout.paddingY * 2.0f);

    ImGui::SetNextWindowPos(ImVec2(0, posY));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, m_height));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(m_layout.paddingX, m_layout.paddingY));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(metrics.colBg).Value);

    m_open = ImGui::Begin(windowName, nullptr, flags);

    if (m_open) {
        // Отрисовка нижней разделительной линии
        const float lineY = posY + m_height - metrics.separatorSize;
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(0, lineY),
            ImVec2(io.DisplaySize.x, lineY),
            metrics.colSeparator,
            metrics.separatorSize
        );

        m_layout.storageId = ImGui::GetID(zonesId);
        ImGuiStorage* storage = ImGui::GetStateStorage();
        m_layout.leftWidth = *storage->GetFloatRef(m_layout.storageId, theme.Scale(metrics.defaultLeftWidth));
        m_layout.rightWidth = *storage->GetFloatRef(m_layout.storageId + 1, theme.Scale(metrics.defaultRightWidth));
    }
}

Bar::~Bar() {
    // ImGui::End() обязателен независимо от результата Begin
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

LeftZone Bar::Left() { return LeftZone(&m_layout, m_open); }
CenterZone Bar::Center() { return CenterZone(&m_layout, m_open); }
RightZone Bar::Right() { return RightZone(&m_layout, m_open); }

// ----------------------------------------------------------------------------
// BarZone
// ----------------------------------------------------------------------------

BarZone::BarZone(BarLayout* layout, bool active)
    : m_layout(layout)
{
    m_open = active && layout;
}

float BarZone::EndZone() {
    if (!m_open) return 0.0f;
    ImGui::EndGroup();
    return ImGui::GetItemRectSize().x;
}

void BarZone::Label(const char* text, UiVariant variant) {
    if (!m_open || !text || text[0] == '\0') return;

    const UiTheme& theme = UiTheme::Get();
    ImFont* font = theme.fontMedium ? theme.fontMedium : theme.defaultFont;
    theme.PushFont(font, 16.0f);

    float lineH = ImGui::GetTextLineHeight();
    float textY = m_layout->paddingY + std::floor((m_layout->contentHeight - lineH) * 0.5f);

    ImGui::SetCursorPosY(textY);
    const SemanticStyle& style = theme.GetVariantStyle(variant);
    ImGui::PushStyleColor(ImGuiCol_Text, style.colText);
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();

    theme.PopFont();
}

bool BarZone::Button(const char* label, const ButtonOptions& options) {
    if (!m_open) return false;
    return ButtonPx(label, ImVec2(0.0f, m_layout->contentHeight), options);
}

// ----------------------------------------------------------------------------
// Left Zone
// ----------------------------------------------------------------------------

LeftZone::LeftZone(BarLayout* layout, bool active)
    : BarZone(layout, active)
{
    if (m_open) {
        ImGui::SetCursorPos(ImVec2(m_layout->paddingX, m_layout->paddingY));
        ImGui::BeginGroup();
    }
}

LeftZone::~LeftZone() {
    if (m_open) {
        float measuredW = EndZone();
        m_layout->leftWidth = measuredW;
        *ImGui::GetStateStorage()->GetFloatRef(m_layout->storageId, 0.0f) = measuredW;
    }
}

// ----------------------------------------------------------------------------
// Right Zone
// ----------------------------------------------------------------------------

RightZone::RightZone(BarLayout* layout, bool active)
    : BarZone(layout, active)
{
    if (m_open) {
        float rightStartX = ImGui::GetWindowWidth() - m_layout->rightWidth - m_layout->paddingX;
        ImGui::SetCursorPos(ImVec2(rightStartX, m_layout->paddingY));
        ImGui::BeginGroup();
    }
}

RightZone::~RightZone() {
    if (m_open) {
        float measuredW = EndZone();
        m_layout->rightWidth = measuredW;
        *ImGui::GetStateStorage()->GetFloatRef(m_layout->storageId + 1, 0.0f) = measuredW;
    }
}

// ----------------------------------------------------------------------------
// Center Zone
// ----------------------------------------------------------------------------

CenterZone::CenterZone(BarLayout* layout, bool active)
    : BarZone(layout, active)
{
    if (m_open) {
        float leftEnd = m_layout->paddingX + m_layout->leftWidth + m_layout->zoneSpacing;
        float rightStart = ImGui::GetWindowWidth() - m_layout->rightWidth
                         - m_layout->paddingX - m_layout->zoneSpacing;

        m_width = std::max(20.0f, rightStart - leftEnd);

        ImGui::SetCursorPos(ImVec2(leftEnd, m_layout->paddingY));
        ImGui::BeginGroup();
    }
}

CenterZone::~CenterZone() {
    EndZone();
}
