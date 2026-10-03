#include "ui/components/Panel.hpp"

Panel::Panel(const PanelOptions& options)
    : m_background(options.cardBackground)
    , m_stickToEnd(options.stickToEnd)
{
    if (m_background) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImColor(UiTheme::Get().card.colBg).Value);
    }
    ImGuiWindowFlags flags = ImGuiWindowFlags_None;
    if (options.scroll == PanelScroll::Always) {
        flags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
    }
    const ImVec2 size = options.sizePx ? *options.sizePx : ImVec2(0.0f, 0.0f);
    m_open = ImGui::BeginChild(options.key ? options.key : "##panel", size, options.border, flags);
}

Panel::~Panel() {
    // EndChild обязателен, даже если BeginChild вернул false
    if (m_open && m_stickToEnd && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    if (m_background) {
        ImGui::PopStyleColor();
    }
}
