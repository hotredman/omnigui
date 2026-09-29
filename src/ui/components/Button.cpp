#include "ui/components/Button.hpp"
#include "ui/components/DisabledScope.hpp"
#include <algorithm>

static const char* FindTextEnd(const char* text) {
    if (!text) return nullptr;
    const char* p = text;
    while (*p) {
        if (p[0] == '#' && p[1] == '#') return p;
        p++;
    }
    return p;
}

bool Button::Render(const char* label, UiVariant variant, Icon icon, float baseWidth, float baseHeight, bool disabled) {
    const UiTheme& theme = UiTheme::Get();
    float w = (baseWidth > 0.0f) ? theme.Scale(baseWidth) : 0.0f;
    float h = (baseHeight > 0.0f) ? theme.Scale(baseHeight) : 0.0f;
    return Render(label, variant, ImVec2(w, h), icon, disabled);
}

bool Button::Render(const char* label, UiVariant variant, Icon icon, UiSize size, float baseWidth, bool disabled) {
    const UiTheme& theme = UiTheme::Get();
    ControlMetrics m = theme.GetMetrics(size);
    float w = (baseWidth > 0.0f) ? theme.Scale(baseWidth) : 0.0f;
    return Render(label, variant, ImVec2(w, m.height), icon, disabled);
}

bool Button::Render(const char* label, UiVariant variant, ImVec2 size, Icon icon, bool disabled) {
    DisabledScope disabledScope(disabled);
    const UiTheme& theme = UiTheme::Get();
    const SemanticStyle& style = theme.GetVariantStyle(variant);

    int pushedColors = 0;
    if (variant != UiVariant::Default) {
        ImGui::PushStyleColor(ImGuiCol_Button, style.colBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.colBgHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.colBgActive);
        ImGui::PushStyleColor(ImGuiCol_Text, ImColor(style.colBtnText).Value);
        pushedColors = 4;
    }

    // Стандартная высота кнопки в дизайн-системе: Medium (36px, масштабируется под DPI)
    float h = (size.y > 0.0f) ? size.y : theme.GetMetrics(UiSize::Medium).height;

    // Выбор шрифта и размера в зависимости от высоты кнопки (четная шкала 14..22pt):
    // Крупные кнопки пульта (ПУСК/СТОП >= 44px) получают 22pt Bold,
    // Выделенные кнопки управления (Пауза/Сброс аварии 38..43px) получают 20pt Medium,
    // Стандартные кнопки (34..37px) получают 18pt Medium,
    // Компактные кнопки (28..33px) получают 16pt Medium,
    // Миниатюрные кнопки (< 28px) получают 14pt Medium
    ImFont* btnFont = nullptr;
    float fontPt = 18.0f;
    if (h >= theme.Scale(44.0f)) {
        btnFont = theme.fontBold ? theme.fontBold : theme.buttonFont;
        fontPt = 22.0f;
    } else if (h >= theme.Scale(38.0f)) {
        btnFont = theme.fontMedium ? theme.fontMedium : theme.buttonFont;
        fontPt = 20.0f;
    } else if (h >= theme.Scale(34.0f)) {
        btnFont = theme.fontMedium ? theme.fontMedium : theme.buttonFont;
        fontPt = 18.0f;
    } else if (h >= theme.Scale(28.0f)) {
        btnFont = theme.fontMedium ? theme.fontMedium : theme.buttonFont;
        fontPt = 16.0f;
    } else {
        btnFont = theme.fontMedium ? theme.fontMedium : theme.buttonFont;
        fontPt = 14.0f;
    }

    theme.PushFont(btnFont, fontPt);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, theme.Scale(theme.input.frameRounding));

    const char* text_end = label ? FindTextEnd(label) : nullptr;
    bool hasLabel = (label && text_end > label);
    ImVec2 textSize = hasLabel ? ImGui::CalcTextSize(label, text_end) : ImVec2(0, 0);

    float iconSize = (icon != Icon::None) ? std::clamp(h * 0.44f, theme.Scale(12.0f), theme.Scale(24.0f)) : 0.0f;
    float gap = (hasLabel && icon != Icon::None) ? theme.Scale(8.0f) : 0.0f;
    float totalW = iconSize + gap + textSize.x;

    // Горизонтальный паддинг по умолчанию: 14px с каждой стороны (для кнопок с текстом)
    float padX = theme.Scale(14.0f);
    float minW = hasLabel ? (totalW + padX * 2.0f) : (size.x > 0.0f ? size.x : h);
    float w = (size.x > 0.0f) ? std::max(size.x, minW) : minW;

    ImVec2 screenPos = ImGui::GetCursorScreenPos();
    ImGui::PushID(label ? label : "btn");
    bool clicked = ImGui::Button("##btn", ImVec2(w, h));
    ImGui::PopID();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImU32 textColor = (pushedColors > 0) ? ImGui::GetColorU32(style.colBtnText) : ImGui::GetColorU32(ImGuiCol_Text);

    float startX = screenPos.x + (w - totalW) * 0.5f;
    if (hasLabel && startX < screenPos.x + padX) {
        startX = screenPos.x + padX;
    }
    float centerY = screenPos.y + h * 0.5f;

    if (ImGui::IsItemActive()) {
        startX += 1.0f;
        centerY += 1.0f;
    }

    if (icon != Icon::None) {
        ImVec2 iconCenter(startX + iconSize * 0.5f, centerY);
        icon.Draw(dl, iconCenter, iconSize, textColor);
    }

    if (hasLabel) {
        float textX = startX + (icon != Icon::None ? (iconSize + gap) : 0.0f);
        float textY = centerY - textSize.y * 0.5f - 1.0f;
        dl->AddText(ImVec2(textX, textY), textColor, label, text_end);
    }

    ImGui::PopStyleVar();
    theme.PopFont();
    if (pushedColors > 0) ImGui::PopStyleColor(pushedColors);

    return clicked;
}

bool Button::IconOnly(const char* id, Icon icon, float baseSize, UiVariant variant, bool disabled) {
    const UiTheme& theme = UiTheme::Get();
    float s = (baseSize > 0.0f) ? theme.Scale(baseSize) : theme.Scale(32.0f);
    if (id && id[0] == '#' && id[1] == '#') {
        return Render(id, variant, ImVec2(s, s), icon, disabled);
    }
    char safeId[64];
    std::snprintf(safeId, sizeof(safeId), "##%s", id ? id : "icon_btn");
    return Render(safeId, variant, ImVec2(s, s), icon, disabled);
}

float Button::CalculateWidth(const char* label, Icon icon, RowHeight height) {
    return CalculateWidth(label, icon, height.IsCustom() ? height.baselinePx : 0.0f);
}

float Button::CalculateWidth(const char* label, Icon icon, float baseHeight) {
    const UiTheme& theme = UiTheme::Get();
    float h = (baseHeight > 0.0f) ? theme.Scale(baseHeight) : theme.GetMetrics(UiSize::Medium).height;

    ImFont* btnFont = nullptr;
    float fontPt = 18.0f;
    if (h >= theme.Scale(44.0f)) {
        btnFont = theme.fontBold ? theme.fontBold : theme.buttonFont;
        fontPt = 22.0f;
    } else if (h >= theme.Scale(38.0f)) {
        btnFont = theme.fontMedium ? theme.fontMedium : theme.buttonFont;
        fontPt = 20.0f;
    } else if (h >= theme.Scale(34.0f)) {
        btnFont = theme.fontMedium ? theme.fontMedium : theme.buttonFont;
        fontPt = 18.0f;
    } else if (h >= theme.Scale(28.0f)) {
        btnFont = theme.fontMedium ? theme.fontMedium : theme.buttonFont;
        fontPt = 16.0f;
    } else {
        btnFont = theme.fontMedium ? theme.fontMedium : theme.buttonFont;
        fontPt = 14.0f;
    }

    theme.PushFont(btnFont, fontPt);
    const char* text_end = label ? FindTextEnd(label) : nullptr;
    bool hasLabel = (label && text_end > label);
    ImVec2 textSize = hasLabel ? ImGui::CalcTextSize(label, text_end) : ImVec2(0, 0);
    theme.PopFont();

    float iconSize = (icon != Icon::None) ? std::clamp(h * 0.44f, theme.Scale(12.0f), theme.Scale(24.0f)) : 0.0f;
    float gap = (hasLabel && icon != Icon::None) ? theme.Scale(8.0f) : 0.0f;
    float totalW = iconSize + gap + textSize.x;
    float padX = theme.Scale(14.0f);
    return hasLabel ? (totalW + padX * 2.0f) : h;
}
