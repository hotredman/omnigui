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
    int         level       = 0;           // уровень вложенности в дереве (0 — без отступа)
};

// Отступ одного уровня дерева (базовые px)
constexpr float kSidebarTreeIndent = 14.0f;

// Реакция на клик по узлу дерева: стрелка раскрытия — Toggle, остальная строка — Activate
enum class SidebarTreeClick { None, Toggle, Activate };

// Параметры узла дерева: стрелка раскрытия, название (до двух строк), справа счётчик и
// индикатор статуса. Высота узла растёт с числом строк названия.
//
//     auto click = list.TreeNode({.label = "Project Alpha", .count = "12", .level = 0, .expanded = open});
//     if (click == SidebarTreeClick::Toggle) open = !open;
struct SidebarTreeNodeOptions {
    std::string label;
    std::string count;                     // счётчик справа; пусто — нет
    int         level       = 0;
    bool        expanded    = false;
    bool        selected    = false;
    ImU32       statusColor = 0;           // 0 — без индикатора
    const char* key         = nullptr;
};

// Название в две строки: первая — по границе слова (слово длиннее строки режется по символу),
// вторая — остаток; если и он не помещается, оканчивается многоточием. Мерка — текущий шрифт ImGui
struct SidebarWrappedLabel {
    std::string line1;
    std::string line2;   // пусто — название уместилось в одну строку
};
SidebarWrappedLabel SidebarWrapLabel(const char* text, float maxWidth);

// Прокручиваемый список внутри боковой панели (RAII-область). Создаётся
// через Sidebar::ScrollList() и занимает всё свободное место до низа панели.
class SidebarList : public Scope {
public:
    ~SidebarList();

    // Двухстрочная запись списка; true — по ней кликнули
    bool Entry(const SidebarEntryOptions& options);

    // Узел дерева со стрелкой раскрытия и счётчиком
    SidebarTreeClick TreeNode(const SidebarTreeNodeOptions& options);

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
// Геометрия боковой панели (итоговые px; по умолчанию — из темы):
//
//     Sidebar({.posYPx = 80, .heightPx = 600});
struct SidebarOptions {
    float posYPx   = -1.0f;   // < 0 — под Header и Toolbar
    float heightPx = -1.0f;   // < 0 — до строки состояния
};

class Sidebar : public Scope {
public:
    // По умолчанию геометрия рассчитывается из UiTheme::Get()
    explicit Sidebar(const SidebarOptions& options = {});
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
