#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Scope.hpp"
#include <imgui.h>
#include <optional>

enum class PanelScroll {
    Auto,    // полоса прокрутки появляется при переполнении
    Always   // вертикальная полоса видна всегда
};

// Параметры панели (designated initializers):
//
//     if (Panel panel({.cardBackground = true}); panel) { ... }
//     if (Panel log({.key = "log", .scroll = PanelScroll::Always, .stickToEnd = autoScroll}); log) { ... }
struct PanelOptions {
    const char*           key            = nullptr;               // идентичность; nullptr — "##panel" (нужен, если в одной области несколько панелей)
    bool                  border         = true;
    bool                  cardBackground = false;                 // фон карточки вместо фона окна
    PanelScroll           scroll         = PanelScroll::Auto;
    bool                  stickToEnd     = false;                 // держать прокрутку внизу, пока пользователь не отлистал вверх (журналы)
    std::optional<ImVec2> sizePx;                                 // итоговый размер, px; по умолчанию — всё оставшееся место
};

// ============================================================================
// Панель — прокручиваемая область с рамкой и (опционально) фоном карточки.
// Обёртка над дочерним окном ImGui.
// ============================================================================
class Panel : public Scope {
public:
    explicit Panel(const PanelOptions& options = {});
    ~Panel();

    // Оставшееся место внутри панели, px (для вложенных элементов, которым нужен явный размер)
    ImVec2 Available() const { return ImGui::GetContentRegionAvail(); }

private:
    bool m_background = false;
    bool m_stickToEnd = false;
};
