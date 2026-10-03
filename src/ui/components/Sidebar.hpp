#pragma once

#include <imgui.h>
#include <string>

#include "ui/components/Scope.hpp"
#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"

// Боковая навигационная панель (Sidebar). RAII-область: конструктор открывает
// окно, деструктор закрывает.
//
//     if (auto sidebar = Sidebar()) { ... }
class Sidebar : public Scope {
public:
    // По умолчанию геометрия рассчитывается из UiTheme::Get():
    // posY < 0 — под Header и TopBar, height < 0 — до строки состояния
    explicit Sidebar(float posY = -1.0f, float height = -1.0f);
    ~Sidebar();

    // 1. Добавление пункта меню с векторной иконкой
    bool AddItem(const char* id, const char* label, Icon icon, bool isActive = false);

    // 2. Добавление пункта меню со строковой иконкой
    bool AddItem(const char* id, const char* label, const char* iconStr, bool isActive = false);

    // 3. Шаблоны для автоматической привязки к перечислению экранов
    template<typename T>
    bool AddItem(T screenId, const char* label, Icon icon, T& currentScreen) {
        bool active = (currentScreen == screenId);
        if (AddItem(label, label, icon, active)) {
            currentScreen = screenId;
            return true;
        }
        return false;
    }

    template<typename T>
    bool AddItem(T screenId, const char* label, const char* iconStr, T& currentScreen) {
        bool active = (currentScreen == screenId);
        if (AddItem(label, label, iconStr, active)) {
            currentScreen = screenId;
            return true;
        }
        return false;
    }

    // 4. Тонкий горизонтальный разделитель
    void AddSeparator();

    // 5. Вертикальный отступ
    void AddSpacing(float height = 8.0f);

private:
    void DrawVectorIcon(Icon icon, ImDrawList* dl, ImVec2 center, float size, ImU32 color);
    float Scale(float val) const { return UiTheme::Get().Scale(val); }

    float m_width = 190.0f;
    float m_height = 0.0f;
    float m_posY = 0.0f;
};
