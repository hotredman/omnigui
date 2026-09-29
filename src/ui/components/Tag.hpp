#pragma once

#include "ui/components/UiTheme.hpp"
#include <string>
#include <vector>

// Интерактивный или информационный тег-чип в стиле evo-machine-cs (# тег)
class Tag {
public:
    // Отрисовка одиночного тега
    // Возвращает true, если тег был нажат пользователем
    static bool Render(const char* label, const char* prefix = "#", bool clickable = true,
                       const char* tooltip = nullptr);

    // Отрисовка набора тегов с автоматическим переносом по доступной ширине
    // Возвращает true, если какой-либо тег был нажат (при этом outClickedTag заполняется выбранным тегом)
    static bool RenderList(const std::vector<std::string>& tags, float maxAvailableWidth = 0.0f,
                           std::string* outClickedTag = nullptr);
};
