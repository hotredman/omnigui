#include "ui/components/FilterBar.hpp"
#include <algorithm>
#include <cmath>


FilterBar::FilterBar(const FilterBarOptions& options)
    : m_id(options.key)
    , m_baseHeight(options.height)
{
    const UiTheme& theme = UiTheme::Get();
    float bh = (m_baseHeight > 0.0f) ? m_baseHeight : theme.filterBar.height;
    m_actualHeight = theme.Scale(bh);
    m_availWidth = ImGui::GetContentRegionAvail().x;
    m_startPos = ImGui::GetCursorPos();

    m_storageId = ImGui::GetID(m_id);
    ImGuiStorage* storage = ImGui::GetStateStorage();
    m_leftWidth = *storage->GetFloatRef(m_storageId, theme.Scale(280.0f));
    m_rightWidth = *storage->GetFloatRef(m_storageId + 1, theme.Scale(460.0f));

    m_open = true;
}

FilterBar::~FilterBar() {
    const UiTheme& theme = UiTheme::Get();
    // Переводим курсор вниз на высоту панели + стандартный зазор до таблицы
    ImGui::SetCursorPos(ImVec2(m_startPos.x, m_startPos.y + m_actualHeight + theme.SpacingSmall()));
    // ImGui требует submit-элемента после SetCursorPos, иначе границы окна не растут
    ImGui::Dummy(ImVec2(0.0f, 0.0f));
}
float FilterBar::GetStretchWidth(float minWidth) const {
    const UiTheme& theme = UiTheme::Get();
    float gap = theme.Scale(theme.filterBar.stretchGap);
    float currentX = ImGui::GetCursorPos().x;
    float usedX = std::max(0.0f, currentX - m_startPos.x);
    float avail = m_availWidth - m_rightWidth - usedX - gap;
    float minW = (minWidth > 0.0f) ? minWidth : theme.filterBar.minSearchWidth;
    float scaledMin = theme.Scale(minW);
    if (avail < scaledMin) {
        avail = scaledMin;
    }
    return avail / theme.GetScale();
}

void FilterBar::NextItem() {
    const UiTheme& theme = UiTheme::Get();
    ImGui::SameLine(0.0f, theme.Scale(theme.filterBar.itemSpacing));
}

void FilterBar::NextGroup() {
    const UiTheme& theme = UiTheme::Get();
    ImGui::SameLine(0.0f, theme.Scale(theme.filterBar.groupSpacing));
}

void FilterBar::Spacing(float spacing) {
    const UiTheme& theme = UiTheme::Get();
    ImGui::SameLine(0.0f, theme.Scale(spacing));
}

void FilterBar::Label(const char* text, const FilterBarLabelOptions& options) {
    const UiVariant variant = options.variant;
    if (!text || text[0] == '\0') return;

    const UiTheme& theme = UiTheme::Get();
    ControlMetrics m = theme.GetMetrics(UiSize::Small);

    ImFont* font = theme.fontMedium ? theme.fontMedium : theme.defaultFont;
    theme.PushFont(font, m.fontSize);

    ImVec2 textSize = ImGui::CalcTextSize(text);
    float fontSize = theme.Scale(m.fontSize);
    float padY = std::max(2.0f, (m.height - fontSize) * 0.5f);

    ImVec2 screenPos = ImGui::GetCursorScreenPos();
    float textX = screenPos.x;
    float textY = screenPos.y + padY;

    ImU32 color = (variant == UiVariant::Default) ? theme.filterBar.colLabel : theme.GetVariantStyle(variant).colText;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddText(ImVec2(textX, textY), color, text);

    // Dummy резервирует место под текст с высотой m.height для идеального ряда
    ImGui::Dummy(ImVec2(textSize.x, m.height));

    theme.PopFont();

    // Автоматический переход к связанному контролу с зазором labelSpacing
    ImGui::SameLine(0.0f, theme.Scale(theme.filterBar.labelSpacing));
}

FilterBar::LeftScope::LeftScope(FilterBar& parent)
    : m_parent(&parent)
{
    m_open = parent.m_open;
    if (m_open) {
        ImGui::SetCursorPos(m_parent->m_startPos);
        ImGui::BeginGroup();
    }
}

FilterBar::LeftScope::~LeftScope() {
    if (m_open) {
        ImGui::EndGroup();
        float w = ImGui::GetItemRectSize().x;
        m_parent->m_leftWidth = w;
        ImGuiStorage* storage = ImGui::GetStateStorage();
        *storage->GetFloatRef(m_parent->m_storageId, 0.0f) = w;
    }
}

FilterBar::RightScope::RightScope(FilterBar& parent)
    : m_parent(&parent)
{
    m_open = parent.m_open;
    if (m_open) {
        const UiTheme& theme = UiTheme::Get();
        float gap = (m_parent->m_leftWidth > 0.0f) ? theme.Scale(theme.filterBar.stretchGap) : 0.0f;
        float minRightX = m_parent->m_startPos.x + m_parent->m_leftWidth + gap;
        float desiredRightX = m_parent->m_startPos.x + std::max(0.0f, m_parent->m_availWidth - m_parent->m_rightWidth);
        float rightX = std::max(minRightX, desiredRightX);
        ImGui::SetCursorPos(ImVec2(rightX, m_parent->m_startPos.y));
        ImGui::BeginGroup();
    }
}

FilterBar::RightScope::~RightScope() {
    if (m_open) {
        ImGui::EndGroup();
        float w = ImGui::GetItemRectSize().x;
        m_parent->m_rightWidth = w;
        ImGuiStorage* storage = ImGui::GetStateStorage();
        *storage->GetFloatRef(m_parent->m_storageId + 1, 0.0f) = w;
    }
}