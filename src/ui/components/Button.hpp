#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include "ui/components/FlowLayout.hpp"
#include <imgui.h>

class Button {
public:
    // Базовый метод дизайн-системы: принимает базовые несмасштабированные пиксели (0 = авто)
    static bool Render(const char* label, 
                       UiVariant variant = UiVariant::Default, 
                       Icon icon = Icon::None,
                       float baseWidth = 0.0f,
                       float baseHeight = 0.0f,
                       bool disabled = false);

    // Семантический метод с типизированным размером UiSize (Large, Medium, Small, Mini)
    static bool Render(const char* label,
                       UiVariant variant,
                       Icon icon,
                       UiSize size,
                       float baseWidth = 0.0f,
                       bool disabled = false);

    // Перегрузка с явным размером ImVec2 (для контейнеров с уже рассчитанной геометрией)
    static bool Render(const char* label, 
                       UiVariant variant, 
                       ImVec2 size, 
                       Icon icon = Icon::None,
                       bool disabled = false);

    // Расчет естественной ширины кнопки по её содержимому (текст, иконка, отступы темы)
    static float CalculateWidth(const char* label, Icon icon, RowHeight height);
    static float CalculateWidth(const char* label, Icon icon = Icon::None, float baseHeight = 0.0f);
    static float CalculateWidth(const char* label, Icon icon, UiSize size) {
        const UiTheme& theme = UiTheme::Get();
        return CalculateWidth(label, icon, theme.GetMetrics(size).height / theme.GetScale());
    }

    // Кнопка-иконка без текста (квадратная, baseSize 0 = автовысота 32px)
    static bool IconOnly(const char* id, 
                         Icon icon, 
                         float baseSize = 0.0f, 
                         UiVariant variant = UiVariant::Default,
                         bool disabled = false);

    // Семантические методы (автоматический расчет размеров по контенту и теме)
    static bool Primary(const char* label, Icon icon = Icon::None, float baseWidth = 0.0f, float baseHeight = 0.0f, bool disabled = false) {
        return Render(label, UiVariant::Primary, icon, baseWidth, baseHeight, disabled);
    }
    static bool Primary(const char* label, Icon icon, UiSize size, float baseWidth = 0.0f, bool disabled = false) {
        return Render(label, UiVariant::Primary, icon, size, baseWidth, disabled);
    }

    static bool Success(const char* label, Icon icon = Icon::None, float baseWidth = 0.0f, float baseHeight = 0.0f, bool disabled = false) {
        return Render(label, UiVariant::Success, icon, baseWidth, baseHeight, disabled);
    }
    static bool Success(const char* label, Icon icon, UiSize size, float baseWidth = 0.0f, bool disabled = false) {
        return Render(label, UiVariant::Success, icon, size, baseWidth, disabled);
    }

    static bool Danger(const char* label, Icon icon = Icon::None, float baseWidth = 0.0f, float baseHeight = 0.0f, bool disabled = false) {
        return Render(label, UiVariant::Danger, icon, baseWidth, baseHeight, disabled);
    }
    static bool Danger(const char* label, Icon icon, UiSize size, float baseWidth = 0.0f, bool disabled = false) {
        return Render(label, UiVariant::Danger, icon, size, baseWidth, disabled);
    }

    static bool Warning(const char* label, Icon icon = Icon::None, float baseWidth = 0.0f, float baseHeight = 0.0f, bool disabled = false) {
        return Render(label, UiVariant::Warning, icon, baseWidth, baseHeight, disabled);
    }
    static bool Warning(const char* label, Icon icon, UiSize size, float baseWidth = 0.0f, bool disabled = false) {
        return Render(label, UiVariant::Warning, icon, size, baseWidth, disabled);
    }

    static bool Secondary(const char* label, Icon icon = Icon::None, float baseWidth = 0.0f, float baseHeight = 0.0f, bool disabled = false) {
        return Render(label, UiVariant::Secondary, icon, baseWidth, baseHeight, disabled);
    }
    static bool Secondary(const char* label, Icon icon, UiSize size, float baseWidth = 0.0f, bool disabled = false) {
        return Render(label, UiVariant::Secondary, icon, size, baseWidth, disabled);
    }

    static bool Info(const char* label, Icon icon = Icon::None, float baseWidth = 0.0f, float baseHeight = 0.0f, bool disabled = false) {
        return Render(label, UiVariant::Info, icon, baseWidth, baseHeight, disabled);
    }
    static bool Info(const char* label, Icon icon, UiSize size, float baseWidth = 0.0f, bool disabled = false) {
        return Render(label, UiVariant::Info, icon, size, baseWidth, disabled);
    }

    // Совместимые перегрузки с явным ImVec2
    static bool Primary(const char* label, ImVec2 size, Icon icon = Icon::None, bool disabled = false) {
        return Render(label, UiVariant::Primary, size, icon, disabled);
    }
    static bool Success(const char* label, ImVec2 size, Icon icon = Icon::None, bool disabled = false) {
        return Render(label, UiVariant::Success, size, icon, disabled);
    }
    static bool Danger(const char* label, ImVec2 size, Icon icon = Icon::None, bool disabled = false) {
        return Render(label, UiVariant::Danger, size, icon, disabled);
    }
    static bool Warning(const char* label, ImVec2 size, Icon icon = Icon::None, bool disabled = false) {
        return Render(label, UiVariant::Warning, size, icon, disabled);
    }
    static bool Secondary(const char* label, ImVec2 size, Icon icon = Icon::None, bool disabled = false) {
        return Render(label, UiVariant::Secondary, size, icon, disabled);
    }
    static bool Info(const char* label, ImVec2 size, Icon icon = Icon::None, bool disabled = false) {
        return Render(label, UiVariant::Info, size, icon, disabled);
    }
};
