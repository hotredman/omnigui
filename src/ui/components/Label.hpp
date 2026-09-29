#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include <imgui.h>

class Label {
public:
    // Отрисовка метки с опциональной векторной иконкой и семантическим стилем
    static void Render(const char* text, UiVariant variant = UiVariant::Default, 
                       Icon icon = Icon::None, float iconSize = 0.0f);

    // Отрисовка метки с кастомным шрифтом и цветом
    static void RenderCustom(const char* text, ImFont* font, ImU32 color, 
                             Icon icon = Icon::None, float iconSize = 0.0f, float fontPt = 18.0f);

    // Семантические хелперы цвета
    static void Default(const char* text, Icon icon = Icon::None) {
        Render(text, UiVariant::Default, icon);
    }
    static void Primary(const char* text, Icon icon = Icon::None) {
        Render(text, UiVariant::Primary, icon);
    }
    static void Secondary(const char* text, Icon icon = Icon::None) {
        Render(text, UiVariant::Secondary, icon);
    }
    static void Success(const char* text, Icon icon = Icon::None) {
        Render(text, UiVariant::Success, icon);
    }
    static void Warning(const char* text, Icon icon = Icon::None) {
        Render(text, UiVariant::Warning, icon);
    }
    static void Danger(const char* text, Icon icon = Icon::None) {
        Render(text, UiVariant::Danger, icon);
    }
    static void Info(const char* text, Icon icon = Icon::None) {
        Render(text, UiVariant::Info, icon);
    }

    // Типографические хелперы (размер и начертание шрифта)
    static void Title(const char* text, UiVariant variant = UiVariant::Default, Icon icon = Icon::None);
    static void Muted(const char* text, Icon icon = Icon::None);
    static void Muted(const char* text, UiSize size, Icon icon = Icon::None);

    // Многострочный текст с авто-переносом по границе контейнера
    static void Wrapped(const char* text, UiVariant variant = UiVariant::Secondary);
};
