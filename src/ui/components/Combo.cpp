#include "ui/components/Combo.hpp"
#include <imgui.h>
#include <algorithm>

namespace {

float CalculateContentWidth(const char* const items[], int itemsCount, ImFont* font, const ComboStyle& style, const UiTheme& theme, float padX) {
    theme.PushFont(font, style.fontSize);
    float maxTextW = 0.0f;
    for (int i = 0; i < itemsCount; ++i) {
        if (!items[i]) continue;
        ImVec2 sz = ImGui::CalcTextSize(items[i]);
        if (sz.x > maxTextW) {
            maxTextW = sz.x;
        }
    }
    theme.PopFont();

    float arrowW = theme.Scale(style.arrowWidth);
    float arrowPadRight = theme.Scale(style.arrowPaddingRight);
    float arrowSpacing = theme.Scale(8.0f);
    // Left padding + text width + space to arrow + arrow + arrow padding to right edge + small safety buffer
    float totalW = padX + maxTextW + arrowSpacing + arrowW + arrowPadRight + theme.Scale(6.0f);
    float minW = theme.Scale(80.0f);
    return std::max(minW, totalW);
}

void RenderComboLabel(const char* label, float width, const ComboStyle& style) {
    if (!label || label[0] == '\0') {
        return;
    }

    const UiTheme& theme = UiTheme::Get();
    ImFont* font = style.labelFont ? style.labelFont : theme.fontRegular;
    theme.PushFont(font, style.labelFontSize);

    float lineHeight = ImGui::GetTextLineHeight();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Клипируем строго по ширине width, чтобы длинный лейбл никогда не вылезал на соседнее поле
    dl->PushClipRect(pos, ImVec2(pos.x + width, pos.y + lineHeight + 2.0f), true);
    dl->AddText(pos, ImGui::GetColorU32(style.colLabel), label);
    dl->PopClipRect();

    ImGui::Dummy(ImVec2(width, lineHeight));

    theme.PopFont();

    float spacing = theme.Scale(style.labelSpacing);
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + lineHeight + spacing));
}

} // namespace

bool Combo::Render(const char* id, int& currentItem, 
                   const char* const items[], int itemsCount, 
                   UiSize size,
                   float width, 
                   UiVariant variant, 
                   const ComboStyle* customStyle)
{
    return Render(id, nullptr, currentItem, items, itemsCount, size, width, variant, customStyle);
}

bool Combo::Render(const char* id, const char* label, int& currentItem, 
                   const char* const items[], int itemsCount, 
                   UiSize size,
                   float width, 
                   UiVariant variant, 
                   const ComboStyle* customStyle)
{
    const UiTheme& theme = UiTheme::Get();
    const ComboStyle& style = customStyle ? *customStyle : theme.combo;
    ControlMetrics m = theme.GetMetrics(size);

    ImFont* valFont = style.font ? style.font : (m.font ? m.font : theme.defaultFont);

    float w = 0.0f;
    if (width > 0.0f) {
        w = width;
    } else if (width < 0.0f) {
        float avail = ImGui::GetContentRegionAvail().x;
        w = std::max(theme.Scale(80.0f), avail + width + 1.0f);
    } else {
        w = CalculateContentWidth(items, itemsCount, valFont, style, theme, m.paddingX);
        if (label && label[0] != '\0') {
            ImFont* lblFont = style.labelFont ? style.labelFont : theme.fontRegular;
            theme.PushFont(lblFont, style.labelFontSize);
            float labelW = ImGui::CalcTextSize(label).x + theme.Scale(8.0f);
            theme.PopFont();
            if (labelW > w) w = labelW;
        }
    }

    ImGui::BeginGroup();

    RenderComboLabel(label, w, style);
    ImGui::SetNextItemWidth(w);

    float fontSize = theme.Scale(style.fontSize);
    float padY = std::max(2.0f, (m.height - fontSize) * 0.5f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m.rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(m.paddingX, padY));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.borderSize);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, theme.Scale(style.popupRounding));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, style.popupBorderSize);

    ImU32 borderColor = (variant == UiVariant::Default) ? style.colBorder : theme.GetVariantStyle(variant).colBorder;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(style.colBg).Value);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImColor(style.colBgHovered).Value);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImColor(style.colBgActive).Value);
    ImGui::PushStyleColor(ImGuiCol_Button, ImColor(style.colBg).Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(style.colBgHovered).Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(style.colBgActive).Value);
    ImGui::PushStyleColor(ImGuiCol_Border, ImColor(borderColor).Value);
    ImGui::PushStyleColor(ImGuiCol_Text, ImColor(style.colText).Value);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImColor(style.colPopupBg).Value);

    theme.PushFont(valFont, style.fontSize);

    const char* preview = (currentItem >= 0 && currentItem < itemsCount) ? items[currentItem] : "";
    bool changed = false;

    if (ImGui::BeginCombo(id, preview, ImGuiComboFlags_NoArrowButton)) {
        for (int i = 0; i < itemsCount; ++i) {
            bool isSelected = (currentItem == i);
            if (ImGui::Selectable(items[i], isSelected)) {
                currentItem = i;
                changed = true;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    theme.PopFont();
    ImGui::PopStyleColor(9);
    ImGui::PopStyleVar(5);

    // Отрисовка аккуратной стрелочки-шеврона справа
    {
        ImVec2 itemMin = ImGui::GetItemRectMin();
        ImVec2 itemMax = ImGui::GetItemRectMax();
        bool isHovered = ImGui::IsItemHovered();

        float arrowW = theme.Scale(style.arrowWidth);
        float arrowH = theme.Scale(style.arrowHeight);
        float rightPad = theme.Scale(style.arrowPaddingRight);

        float cx = itemMax.x - rightPad - arrowW * 0.5f;
        float cy = itemMin.y + (itemMax.y - itemMin.y) * 0.5f;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p1(cx - arrowW * 0.5f, cy - arrowH * 0.5f);
        ImVec2 p2(cx + arrowW * 0.5f, cy - arrowH * 0.5f);
        ImVec2 p3(cx, cy + arrowH * 0.5f);

        ImU32 arrowColor = isHovered ? style.colArrowHovered : style.colArrow;
        dl->AddTriangleFilled(p1, p2, p3, ImGui::GetColorU32(arrowColor));
    }

    ImGui::EndGroup();
    return changed;
}

bool Combo::Render(const char* id, int& currentItem, 
                   const char* const items[], int itemsCount, 
                   float width, float height, 
                   UiVariant variant, 
                   const ComboStyle* customStyle)
{
    return Render(id, nullptr, currentItem, items, itemsCount, width, height, variant, customStyle);
}

bool Combo::Render(const char* id, const char* label, int& currentItem, 
                   const char* const items[], int itemsCount, 
                   float width, float height, 
                   UiVariant variant, 
                   const ComboStyle* customStyle)
{
    if (height <= 0.0f) {
        return Render(id, label, currentItem, items, itemsCount, UiSize::Medium, width, variant, customStyle);
    }

    const UiTheme& theme = UiTheme::Get();
    const ComboStyle& style = customStyle ? *customStyle : theme.combo;
    ImFont* valFont = style.font ? style.font : theme.defaultFont;

    float w = 0.0f;
    if (width > 0.0f) {
        w = width;
    } else if (width < 0.0f) {
        float avail = ImGui::GetContentRegionAvail().x;
        w = std::max(theme.Scale(80.0f), avail + width + 1.0f);
    } else {
        w = CalculateContentWidth(items, itemsCount, valFont, style, theme, theme.Scale(style.framePaddingX));
        if (label && label[0] != '\0') {
            ImFont* lblFont = style.labelFont ? style.labelFont : theme.fontRegular;
            theme.PushFont(lblFont, style.labelFontSize);
            float labelW = ImGui::CalcTextSize(label).x + theme.Scale(8.0f);
            theme.PopFont();
            if (labelW > w) w = labelW;
        }
    }

    ImGui::BeginGroup();
    RenderComboLabel(label, w, style);
    ImGui::SetNextItemWidth(w);

    float fontSize = theme.Scale(style.fontSize);
    float padY = std::max(2.0f, (height - fontSize) * 0.5f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, theme.Scale(style.frameRounding));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(theme.Scale(style.framePaddingX), padY));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.borderSize);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, theme.Scale(style.popupRounding));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, style.popupBorderSize);

    ImU32 borderColor = (variant == UiVariant::Default) ? style.colBorder : theme.GetVariantStyle(variant).colBorder;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(style.colBg).Value);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImColor(style.colBgHovered).Value);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImColor(style.colBgActive).Value);
    ImGui::PushStyleColor(ImGuiCol_Button, ImColor(style.colBg).Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(style.colBgHovered).Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(style.colBgActive).Value);
    ImGui::PushStyleColor(ImGuiCol_Border, ImColor(borderColor).Value);
    ImGui::PushStyleColor(ImGuiCol_Text, ImColor(style.colText).Value);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImColor(style.colPopupBg).Value);

    theme.PushFont(valFont, style.fontSize);

    const char* preview = (currentItem >= 0 && currentItem < itemsCount) ? items[currentItem] : "";
    bool changed = false;

    if (ImGui::BeginCombo(id, preview, ImGuiComboFlags_NoArrowButton)) {
        for (int i = 0; i < itemsCount; ++i) {
            bool isSelected = (currentItem == i);
            if (ImGui::Selectable(items[i], isSelected)) {
                currentItem = i;
                changed = true;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    theme.PopFont();
    ImGui::PopStyleColor(9);
    ImGui::PopStyleVar(5);

    // Chevron
    {
        ImVec2 itemMin = ImGui::GetItemRectMin();
        ImVec2 itemMax = ImGui::GetItemRectMax();
        bool isHovered = ImGui::IsItemHovered();

        float arrowW = theme.Scale(style.arrowWidth);
        float arrowH = theme.Scale(style.arrowHeight);
        float rightPad = theme.Scale(style.arrowPaddingRight);

        float cx = itemMax.x - rightPad - arrowW * 0.5f;
        float cy = itemMin.y + (itemMax.y - itemMin.y) * 0.5f;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p1(cx - arrowW * 0.5f, cy - arrowH * 0.5f);
        ImVec2 p2(cx + arrowW * 0.5f, cy - arrowH * 0.5f);
        ImVec2 p3(cx, cy + arrowH * 0.5f);

        ImU32 arrowColor = isHovered ? style.colArrowHovered : style.colArrow;
        dl->AddTriangleFilled(p1, p2, p3, ImGui::GetColorU32(arrowColor));
    }

    ImGui::EndGroup();
    return changed;
}

bool Combo::Render(const char* id, int& currentItem, 
                   const std::vector<std::string>& items, 
                   UiSize size,
                   float width, 
                   UiVariant variant, 
                   const ComboStyle* customStyle)
{
    return Render(id, nullptr, currentItem, items, size, width, variant, customStyle);
}

bool Combo::Render(const char* id, const char* label, int& currentItem, 
                   const std::vector<std::string>& items, 
                   UiSize size,
                   float width, 
                   UiVariant variant, 
                   const ComboStyle* customStyle)
{
    std::vector<const char*> cstrings;
    cstrings.reserve(items.size());
    for (const auto& item : items) {
        cstrings.push_back(item.c_str());
    }
    return Render(id, label, currentItem, cstrings.data(), static_cast<int>(cstrings.size()), size, width, variant, customStyle);
}

bool Combo::Render(const char* id, int& currentItem, 
                   const std::vector<std::string>& items, 
                   float width, float height, 
                   UiVariant variant, 
                   const ComboStyle* customStyle)
{
    return Render(id, nullptr, currentItem, items, width, height, variant, customStyle);
}

bool Combo::Render(const char* id, const char* label, int& currentItem, 
                   const std::vector<std::string>& items, 
                   float width, float height, 
                   UiVariant variant, 
                   const ComboStyle* customStyle)
{
    std::vector<const char*> cstrings;
    cstrings.reserve(items.size());
    for (const auto& item : items) {
        cstrings.push_back(item.c_str());
    }
    return Render(id, label, currentItem, cstrings.data(), static_cast<int>(cstrings.size()), width, height, variant, customStyle);
}
