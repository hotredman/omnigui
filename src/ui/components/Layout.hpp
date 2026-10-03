#pragma once

#include "ui/components/UiTheme.hpp"
#include <imgui.h>

// ============================================================================
// Примитивы раскладки. Все размеры — в базовых px (масштаб темы применяется
// внутри), поэтому прикладному коду не нужен ни ImGui::, ни theme.Scale().
//
//     Text("Run #01");
//     SameLine();
//     Badge("COMPLETED", {.variant = UiVariant::Success});
//     Spacer();                  // стандартный вертикальный интервал
//     Spacer(12);                // 12 базовых px
//     AlignRight(240);           // следующий элемент — в правом краю, 240 px под группу
//     Button({.label = "Reset"});
// ============================================================================

// Вертикальный интервал. basePx <= 0 — стандартный интервал между элементами
inline void Spacer(float basePx = 0.0f) {
    if (basePx <= 0.0f) {
        ImGui::Spacing();
    } else {
        ImGui::Dummy(ImVec2(0.0f, UiTheme::Get().Scale(basePx)));
    }
}

// Следующий элемент — в той же строке. gapBasePx < 0 — стандартный зазор
inline void SameLine(float gapBasePx = -1.0f) {
    if (gapBasePx < 0.0f) {
        ImGui::SameLine();
    } else {
        ImGui::SameLine(0.0f, UiTheme::Get().Scale(gapBasePx));
    }
}

// Следующий элемент — в той же строке, прижатым к правому краю области:
// widthBasePx — суммарная ширина группы элементов, которая окажется справа
inline void AlignRight(float widthBasePx) {
    ImGui::SameLine(ImGui::GetContentRegionMax().x - UiTheme::Get().Scale(widthBasePx));
}
