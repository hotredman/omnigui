#include "ui/components/PresetGrid.hpp"
#include "ui/components/ValueDisplay.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <string>

template<typename T>
static bool RenderInternal(T& value, const std::vector<T>& presets, const PresetGridOptions& options) {
    const UiTheme& theme = UiTheme::Get();
    const PresetGridStyle& style = options.style ? *options.style : theme.presetGrid;
    const char* unit = options.unit;
    const ControlMetrics metrics = theme.GetMetrics(options.size);
    const float sizeFactor = theme.SizeFactor(options.size);
    const int columns = options.columns;
    const bool showDisplay = options.showDisplay;
    const char* displayFormat = options.displayFormat;
    const char* btnFormat = options.buttonFormat;

    // Идентичность всей сетки: ключ или адрес значения (две сетки в одном окне не конфликтуют)
    if (options.key) ImGui::PushID(options.key); else ImGui::PushID(&value);

    float w = options.sizePx.x;
    if (w <= 0.0f) {
        float avail = ImGui::GetContentRegionAvail().x;
        w = (avail > 0.0f) ? avail : theme.Scale(200.0f);
    }

    bool changed = false;

    // 1. Информационное табло текущего значения
    if (showDisplay) {
        float dispH = (options.sizePx.y > 0.0f) ? options.sizePx.y : metrics.height;

        ValueDisplayStyle dispStyle = theme.valueDisplay;
        dispStyle.valueFont = style.displayValFont ? style.displayValFont : theme.displayBigFont;
        dispStyle.valueFontSize = style.displayValFontSize * sizeFactor;
        dispStyle.valueCompactFontSize = style.displayValFontSize * sizeFactor;
        dispStyle.unitFont = style.displayUnitFont ? style.displayUnitFont : theme.defaultFont;
        dispStyle.unitFontSize = style.displayUnitFontSize * sizeFactor;
        dispStyle.unitCompactFontSize = style.displayUnitFontSize * sizeFactor;

        ValueDisplay(static_cast<double>(value),
                     {.unit = unit, .format = displayFormat, .style = &dispStyle, .sizePx = ImVec2(w, dispH)});

        // Интерактивность табло: ввод пользовательского значения по клику
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Click to enter custom value");
        }
        if (ImGui::IsItemClicked()) {
            ImGui::OpenPopup("##CustomPresetInput");
        }

        if (ImGui::BeginPopup("##CustomPresetInput")) {
            ImGui::Text("Custom Value:");
            static float inputBuf = 0.0f;
            if (ImGui::IsWindowAppearing()) {
                inputBuf = static_cast<float>(value);
            }
            ImGui::SetNextItemWidth(theme.Scale(120.0f));
            if (ImGui::InputFloat("##inputVal", &inputBuf, 0.0f, 0.0f, displayFormat, ImGuiInputTextFlags_EnterReturnsTrue)) {
                value = static_cast<T>(inputBuf);
                changed = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("OK")) {
                value = static_cast<T>(inputBuf);
                changed = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (style.displaySpacing != 0.0f) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + theme.Scale(style.displaySpacing));
        }
    }

    // 2. Сетка кнопок пресетов
    if (presets.empty()) {
        ImGui::PopID();
        return changed;
    }

    int cols = std::max(1, columns);
    float colSpacing = theme.Scale(style.colSpacing);
    float rowSpacing = theme.Scale(style.rowSpacing);
    float btnW = std::floor((w - colSpacing * float(cols - 1)) / float(cols));
    btnW = std::max(16.0f, btnW);
    float btnH = metrics.height;

    ImFont* btnFont = style.btnFont ? style.btnFont : (theme.fontMedium ? theme.fontMedium : theme.defaultFont);
    float fontSize = theme.Scale(style.btnFontSize * sizeFactor);
    theme.PushFont(btnFont, style.btnFontSize * sizeFactor);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(colSpacing, rowSpacing));

    // Оптические метрики цифры '0' для идеального вертикального центрирования
    ImFontBaked* baked = btnFont ? btnFont->GetFontBaked(fontSize) : nullptr;
    const ImFontGlyph* digitGlyph = baked ? baked->FindGlyph('0') : nullptr;
    float glyphMidY = digitGlyph ? (digitGlyph->Y0 + digitGlyph->Y1) * 0.5f : (fontSize * 0.35f);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float rounding = theme.Scale(style.btnRounding) * sizeFactor;

    for (size_t i = 0; i < presets.size(); ++i) {
        if (i > 0 && (i % cols != 0)) {
            ImGui::SameLine();
        }

        // Логика выделения: кнопка активна ТОЛЬКО если значение точно совпадает с пресетом
        bool isSelected = (std::abs(static_cast<double>(value) - static_cast<double>(presets[i])) < 1e-4);

        char btnLabel[32];
        snprintf(btnLabel, sizeof(btnLabel), btnFormat ? btnFormat : "%.4g", presets[i]);

        // Интерактивный элемент
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::InvisibleButton("##pg_btn", ImVec2(btnW, btnH))) {
            value = presets[i];
            changed = true;
        }

        bool isHovered = ImGui::IsItemHovered();
        bool isActive = ImGui::IsItemActive();

        ImU32 bgCol = style.colBtnBg;
        ImU32 borderCol = style.colBtnBorder;
        ImU32 textCol = style.colBtnText;

        if (isSelected) {
            bgCol = style.colBtnActiveBg;
            borderCol = style.colBtnActiveBorder;
            textCol = style.colBtnActiveText;
        } else if (isHovered) {
            bgCol = style.colBtnHoverBg;
            borderCol = style.colBtnHoverBorder;
            textCol = style.colBtnHoverText;
        }

        ImVec2 pMin = ImGui::GetItemRectMin();
        ImVec2 pMax = ImGui::GetItemRectMax();

        // 1. Отрисовка фона и рамки
        dl->AddRectFilled(pMin, pMax, ImGui::GetColorU32(bgCol), rounding);
        dl->AddRect(pMin, pMax, ImGui::GetColorU32(borderCol), rounding, 0, 1.0f);

        // 2. Расчет точного оптического центрирования цифр
        ImVec2 txtSize = btnFont ? btnFont->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, btnLabel) 
                                 : ImGui::CalcTextSize(btnLabel);
        float textX = pMin.x + (btnW - txtSize.x) * 0.5f;
        float textY = pMin.y + (btnH * 0.5f) - glyphMidY;

        // Тактильный микро-сдвиг при нажатии кнопки
        if (isActive) {
            textX += 1.0f;
            textY += 1.0f;
        }

        dl->AddText(btnFont, fontSize, ImVec2(textX, textY), ImGui::GetColorU32(textCol), btnLabel);
        ImGui::PopID();
    }

    ImGui::PopStyleVar();
    theme.PopFont();

    ImGui::PopID();
    return changed;
}

bool PresetGrid(float& value, const std::vector<float>& presets, const PresetGridOptions& options) {
    return RenderInternal<float>(value, presets, options);
}

bool PresetGrid(double& value, const std::vector<double>& presets, const PresetGridOptions& options) {
    return RenderInternal<double>(value, presets, options);
}
