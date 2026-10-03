#include "ui/components/StatusBar.hpp"
#include <imgui_internal.h>
#include <algorithm>

StatusBar::StatusBar(const StatusBarOptions& options)
    : m_customStyle(options.style)
{
    const UiTheme& theme = UiTheme::Get();
    ImGuiIO& io = ImGui::GetIO();
    m_height = theme.Scale(Style().height);

    ImGui::SetNextWindowPos(ImVec2(0.0f, io.DisplaySize.y - m_height));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, m_height));

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoScrollWithMouse |
                                   ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(theme.Scale(Style().paddingX), 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(Style().colBg).Value);

    m_open = ImGui::Begin("##StatusBarWindow", nullptr, flags);
    if (m_open) {
        // Верхняя линия-разделитель полосы
        const ImVec2 pos = ImGui::GetWindowPos();
        ImGui::GetWindowDrawList()->AddLine(
            pos, ImVec2(pos.x + io.DisplaySize.x, pos.y), Style().colTopLine,
            Style().separatorSize);
    }
}

StatusBar::~StatusBar() {
    // ImGui::End() обязателен независимо от результата Begin
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

void StatusBar::NextItem() {
    if (m_hasItems)
        ImGui::SameLine(0.0f, UiTheme::Get().Scale(Style().itemSpacing));
    m_hasItems = true;
}

void StatusBar::Text(const std::string& text, const StatusBarTextOptions& options, std::source_location loc) {
    const UiVariant variant = options.variant;
    if (!m_open) return;
    NextItem();

    const UiTheme& theme = UiTheme::Get();
    const float lineHeight = ImGui::GetTextLineHeight();
    const float rightEdge = ImGui::GetWindowPos().x + ImGui::GetWindowWidth() -
                            theme.Scale(Style().paddingX);
    const ImVec2 start(ImGui::GetCursorScreenPos().x,
                       ImGui::GetWindowPos().y + (m_height - lineHeight) * 0.5f);
    const float available = std::max(0.0f, rightEdge - start.x);
    const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    const float width = std::min(options.maxWidth > 0.0f ? theme.Scale(options.maxWidth) : available, available);
    const float itemWidth = std::min(textSize.x, width);

    // Элемент под текстом: место в строке и зона подсказки
    ImGui::SetCursorScreenPos(start);
    {
        AutoIdScope idScope(options.key, text.c_str(), loc, m_textCount++);
        ImGui::InvisibleButton("##StatusBarText", ImVec2(std::max(1.0f, itemWidth), lineHeight));
    }
    const bool truncated = textSize.x > width;
    if (truncated && ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", text.c_str());

    const ImU32 color = variant == UiVariant::Default ? Style().colText
                                                      : theme.GetVariantStyle(variant).colText;
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(color));
    ImGui::RenderTextEllipsis(ImGui::GetWindowDrawList(), start,
                              ImVec2(start.x + width, start.y + lineHeight),
                              start.x + width, text.c_str(), nullptr, &textSize);
    ImGui::PopStyleColor();
}

void StatusBar::Separator() {
    if (!m_open) return;
    NextItem();

    const float inset = m_height * 0.25f;
    const ImVec2 pos(ImGui::GetCursorScreenPos().x, ImGui::GetWindowPos().y);
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + inset));
    ImGui::Dummy(ImVec2(Style().separatorSize, m_height - inset * 2.0f));
    ImGui::GetWindowDrawList()->AddLine(ImVec2(pos.x, pos.y + inset),
                                        ImVec2(pos.x, pos.y + m_height - inset),
                                        Style().colSeparator, Style().separatorSize);
}
