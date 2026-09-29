#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include <imgui.h>

class ToolButton {
public:
    // ========================================================================
    // 1. Рендеринг с векторной иконкой Icon
    // ========================================================================
    static bool Render(const char* id,
                       Icon icon,
                       UiVariant variant = UiVariant::Default,
                       const char* tooltip = nullptr,
                       bool selected = false,
                       float baseSize = 0.0f,
                       ImU32 iconColor = 0,
                       bool disabled = false);

    // Перегрузка с типизированным размером UiSize
    static bool Render(const char* id,
                       Icon icon,
                       UiSize size,
                       UiVariant variant = UiVariant::Default,
                       const char* tooltip = nullptr,
                       bool selected = false,
                       ImU32 iconColor = 0,
                       bool disabled = false);

    // Перегрузка с tooltip перед вариантом (удобно для ToolButton::Render("##id", Icon::Plus, "Создать"))
    static bool Render(const char* id,
                       Icon icon,
                       const char* tooltip,
                       UiVariant variant = UiVariant::Default,
                       bool selected = false,
                       float baseSize = 0.0f,
                       ImU32 iconColor = 0,
                       bool disabled = false) {
        return Render(id, icon, variant, tooltip, selected, baseSize, iconColor, disabled);
    }

    static bool Render(const char* id,
                       Icon icon,
                       const char* tooltip,
                       UiSize size,
                       UiVariant variant = UiVariant::Default,
                       bool selected = false,
                       ImU32 iconColor = 0,
                       bool disabled = false) {
        return Render(id, icon, size, variant, tooltip, selected, iconColor, disabled);
    }

    // ========================================================================
    // 2. Рендеринг с текстовым глифом или короткой меткой (например, "Aa", "(*)", "+")
    // ========================================================================
    static bool Render(const char* id,
                       const char* glyphOrLabel,
                       UiVariant variant = UiVariant::Default,
                       const char* tooltip = nullptr,
                       bool selected = false,
                       float baseSize = 0.0f,
                       bool disabled = false);

    // Перегрузка с tooltip перед вариантом для глифа
    static bool Render(const char* id,
                       const char* glyphOrLabel,
                       const char* tooltip,
                       UiVariant variant = UiVariant::Default,
                       bool selected = false,
                       float baseSize = 0.0f,
                       bool disabled = false) {
        return Render(id, glyphOrLabel, variant, tooltip, selected, baseSize, disabled);
    }

    // ========================================================================
    // 3. Семантические методы для векторных иконок
    // ========================================================================
    static bool Default(const char* id, Icon icon, const char* tooltip = nullptr, bool disabled = false, bool selected = false, float baseSize = 0.0f) {
        return Render(id, icon, UiVariant::Default, tooltip, selected, baseSize, 0, disabled);
    }
    static bool Primary(const char* id, Icon icon, const char* tooltip = nullptr, bool disabled = false, bool selected = false, float baseSize = 0.0f) {
        return Render(id, icon, UiVariant::Primary, tooltip, selected, baseSize, 0, disabled);
    }
    static bool Secondary(const char* id, Icon icon, const char* tooltip = nullptr, bool disabled = false, bool selected = false, float baseSize = 0.0f) {
        return Render(id, icon, UiVariant::Secondary, tooltip, selected, baseSize, 0, disabled);
    }
    static bool Success(const char* id, Icon icon, const char* tooltip = nullptr, bool disabled = false, bool selected = false, float baseSize = 0.0f) {
        return Render(id, icon, UiVariant::Success, tooltip, selected, baseSize, 0, disabled);
    }
    static bool Danger(const char* id, Icon icon, const char* tooltip = nullptr, bool disabled = false, bool selected = false, float baseSize = 0.0f) {
        return Render(id, icon, UiVariant::Danger, tooltip, selected, baseSize, 0, disabled);
    }
    static bool Warning(const char* id, Icon icon, const char* tooltip = nullptr, bool disabled = false, bool selected = false, float baseSize = 0.0f) {
        return Render(id, icon, UiVariant::Warning, tooltip, selected, baseSize, 0, disabled);
    }
    static bool Info(const char* id, Icon icon, const char* tooltip = nullptr, bool disabled = false, bool selected = false, float baseSize = 0.0f) {
        return Render(id, icon, UiVariant::Info, tooltip, selected, baseSize, 0, disabled);
    }

    // ========================================================================
    // 4. Семантические методы для текстовых глифов
    // ========================================================================
    static bool Default(const char* id, const char* glyph, const char* tooltip = nullptr, bool selected = false, float baseSize = 0.0f) {
        return Render(id, glyph, UiVariant::Default, tooltip, selected, baseSize);
    }
    static bool Primary(const char* id, const char* glyph, const char* tooltip = nullptr, bool selected = false, float baseSize = 0.0f) {
        return Render(id, glyph, UiVariant::Primary, tooltip, selected, baseSize);
    }
    static bool Secondary(const char* id, const char* glyph, const char* tooltip = nullptr, bool selected = false, float baseSize = 0.0f) {
        return Render(id, glyph, UiVariant::Secondary, tooltip, selected, baseSize);
    }
    static bool Success(const char* id, const char* glyph, const char* tooltip = nullptr, bool selected = false, float baseSize = 0.0f) {
        return Render(id, glyph, UiVariant::Success, tooltip, selected, baseSize);
    }
    static bool Danger(const char* id, const char* glyph, const char* tooltip = nullptr, bool selected = false, float baseSize = 0.0f) {
        return Render(id, glyph, UiVariant::Danger, tooltip, selected, baseSize);
    }
    static bool Warning(const char* id, const char* glyph, const char* tooltip = nullptr, bool selected = false, float baseSize = 0.0f) {
        return Render(id, glyph, UiVariant::Warning, tooltip, selected, baseSize);
    }
    static bool Info(const char* id, const char* glyph, const char* tooltip = nullptr, bool selected = false, float baseSize = 0.0f) {
        return Render(id, glyph, UiVariant::Info, tooltip, selected, baseSize);
    }
};
