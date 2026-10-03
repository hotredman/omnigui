#pragma once

#include "ui/components/UiTheme.hpp"
#include <imgui.h>

// Параметры баннера статуса
struct StatusBannerOptions {
    UiVariant variant  = UiVariant::Primary;   // Primary — акцентный цвет темы
    bool      busy     = false;                // фоновый процесс: добавляет «...»
    float     progress = -1.0f;                // 0..1; <0 — прогресс неизвестен (только с busy)
};

// Семантический баннер статуса или фонового процесса:
//
//     StatusBanner("Loading", {.busy = true, .progress = 0.42f});   // «Loading: 42%...»
//     StatusBanner("Low disk space", {.variant = UiVariant::Warning});
inline void StatusBanner(const char* message, const StatusBannerOptions& options = {}) {
    const UiTheme& theme = UiTheme::Get();
    const ImVec4 color = (options.variant == UiVariant::Primary)
        ? ImColor(theme.palette.accent).Value
        : ImColor(theme.GetVariantStyle(options.variant).colText).Value;

    ImGui::PushStyleColor(ImGuiCol_Text, color);
    if (options.busy) {
        if (options.progress >= 0.0f) {
            ImGui::Text("%s: %.0f%%...", message, options.progress * 100.0f);
        } else {
            ImGui::Text("%s...", message);
        }
    } else {
        ImGui::TextWrapped("%s", message);
    }
    ImGui::PopStyleColor();
    ImGui::Spacing();
}
