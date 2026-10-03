#pragma once

#include <imgui.h>
#include <string>

#include "ui/components/Scope.hpp"
#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"

// Параметры пункта навигации (designated initializers):
//
//     sidebar.Item({.label = "Live Stream", .icon = Icon::Gamepad, .selected = true});
//
// Идентичность — key, иначе label.
struct SidebarItemOptions {
    std::string label;
    Icon        icon     = Icon::None;
    bool        selected = false;
    const char* key      = nullptr;
};

// Параметры двухстрочной записи списка (название + дата/статус + светодиод):
//
//     list.Entry({.label = run.title, .sublabel = "10:14 • 1240 pts", .statusColor = run.ledColor, .key = run.id.c_str()});
struct SidebarEntryOptions {
    std::string label;
    std::string sublabel;
    ImU32       statusColor = 0;       // 0 — без светодиода
    bool        selected    = false;
    const char* key         = nullptr;
};

// Прокручиваемый список внутри боковой панели (RAII-область). Создаётся
// через Sidebar::ScrollList() и занимает всё свободное место до низа панели.
class SidebarList : public Scope {
public:
    ~SidebarList();

    // Двухстрочная запись списка; true — по ней кликнули
    bool Entry(const SidebarEntryOptions& options);

    // Заглушка пустого списка
    void Empty(const std::string& message = "List is empty", const std::string& detail = "");

private:
    friend class Sidebar;
    SidebarList();
};

// ============================================================================
// Боковая навигационная панель (Sidebar). RAII-область: конструктор открывает
// окно, деструктор закрывает.
//
//     if (auto sidebar = Sidebar()) {
//         sidebar.Item({.label = "Live Stream", .icon = Icon::Gamepad}, Screen::Live, current);
//         sidebar.Separator();
//         sidebar.SectionTitle("RECORDED RUNS");
//         if (auto list = sidebar.ScrollList()) {
//             if (list.Entry({.label = "Run #01", .key = "run_01"})) { ... }
//         }
//     }
// ============================================================================
class Sidebar : public Scope {
public:
    // По умолчанию геометрия рассчитывается из UiTheme::Get():
    // posY < 0 — под Header и TopBar, height < 0 — до строки состояния
    explicit Sidebar(float posY = -1.0f, float height = -1.0f);
    ~Sidebar();

    // 1. Однострочный пункт меню с векторной иконкой; true — по нему кликнули
    bool Item(const SidebarItemOptions& options);

    // 2. Привязка пункта к значению enum: selected вычисляется сам, по клику
    // current принимает value
    template<typename T>
    bool Item(SidebarItemOptions options, T value, T& current) {
        options.selected = (current == value);
        if (Item(options)) {
            current = value;
            return true;
        }
        return false;
    }

    // 3. Разделитель, заголовок группы, вертикальный отступ (базовые px)
    void Separator();
    void SectionTitle(const std::string& title);
    void Spacer(float basePx = 8.0f);

    // 4. Прокручиваемый список под пунктами (до низа панели)
    SidebarList ScrollList();

private:
    float Scale(float val) const { return UiTheme::Get().Scale(val); }
};
