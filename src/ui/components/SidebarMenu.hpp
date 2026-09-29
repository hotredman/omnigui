#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include <imgui.h>

// Компонент вертикального меню навигации боковой панели (SidebarMenu)
// Поддерживает:
// - Верхний фиксированный блок навигации (Item);
// - Заголовки секций (SectionTitle) и разделители (Separator);
// - Автономную скроллируемую зону под список испытаний (BeginScrollRegion / EndScrollRegion);
// - Двухстрочные элементы испытаний со статусом и датой (ItemEx);
// - Единый стиль и строгую модель взаимного выбора (Single Selection).
class SidebarMenu {
public:
    SidebarMenu(const SidebarStyle* customStyle = nullptr);
    ~SidebarMenu();

    SidebarMenu(const SidebarMenu&) = delete;
    SidebarMenu& operator=(const SidebarMenu&) = delete;

    // 1. Однострочный пункт главного меню (фиксированная зона)
    bool Item(const char* id, const char* label, Icon icon, bool isSelected = false);

    // Шаблон автоматической привязки пункта меню к значению enum экрана
    template<typename T>
    bool Item(const char* label, Icon icon, T itemValue, T& currentSelected) {
        bool active = (currentSelected == itemValue);
        if (Item(label, label, icon, active)) {
            currentSelected = itemValue;
            return true;
        }
        return false;
    }

    // 2. Разделители и заголовки групп
    void Separator();
    void SectionTitle(const char* title);
    void Spacing(float height = 8.0f);

    // 3. Скроллируемая область (занимает всё свободное пространство до низа сайдбара)
    bool BeginScrollRegion(const char* id = "##SidebarScrollRegion", float customHeight = 0.0f);
    void EndScrollRegion();

    // 4. Двухстрочный элемент списка внутри скролл-зоны (название + дата/время + статусная точка)
    bool ItemEx(const char* id,
                const char* label,
                const char* sublabel,
                ImU32 statusColor = 0,
                bool isSelected = false);

    // 5. Заглушка пустого списка
    void Empty(const char* message = "Список пуст", const char* detail = nullptr);

private:
    float Scale(float val) const { return UiTheme::Get().Scale(val); }

    const SidebarStyle* m_style = nullptr;
    bool m_scrollRegionActive = false;
};
