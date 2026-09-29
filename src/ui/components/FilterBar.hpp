#pragma once

#include "ui/components/UiTheme.hpp"
#include <imgui.h>

// Внутристраничная панель фильтрации, поиска и действий (FilterBar)
// Рендерится внутри рабочей области контента (в отличие от глобального Toolbar каркаса)
class FilterBar {
public:
    class LeftScope {
    public:
        explicit LeftScope(FilterBar* parent);
        ~LeftScope();
        LeftScope(const LeftScope&) = delete;
        LeftScope& operator=(const LeftScope&) = delete;
        LeftScope(LeftScope&& other) noexcept;
        LeftScope& operator=(LeftScope&& other) noexcept;
        explicit operator bool() const { return m_active; }
    private:
        FilterBar* m_parent = nullptr;
        bool m_active = false;
    };

    class RightScope {
    public:
        explicit RightScope(FilterBar* parent);
        ~RightScope();
        RightScope(const RightScope&) = delete;
        RightScope& operator=(const RightScope&) = delete;
        RightScope(RightScope&& other) noexcept;
        RightScope& operator=(RightScope&& other) noexcept;
        explicit operator bool() const { return m_active; }
    private:
        FilterBar* m_parent = nullptr;
        bool m_active = false;
    };

    explicit FilterBar(const char* id = "##FilterBar", float baseHeight = 0.0f);
    ~FilterBar();

    FilterBar(const FilterBar&) = delete;
    FilterBar& operator=(const FilterBar&) = delete;

    bool Begin();
    void End();

    explicit operator bool() const { return m_open; }

    LeftScope Left();
    RightScope Right();

    float GetAvailableWidth() const { return m_availWidth; }

    // Доступная ширина для растягивающегося элемента в левой группе (в базовых unscaled пикселях)
    // Если minWidth <= 0, используется theme.filterBar.minSearchWidth
    float GetStretchWidth(float minWidth = 0.0f) const;

    // Шаг между элементами в одной группе (theme.filterBar.itemSpacing)
    static void NextItem();

    // Шаг между разными смысловыми группами (theme.filterBar.groupSpacing)
    static void NextGroup();

    // Произвольный шаг между элементами
    static void Spacing(float spacingPx);

    // Для обратной совместимости
    static void NextItem(float spacingPx) { Spacing(spacingPx); }

    // Текстовая подпись внутри панели (вертикально отцентрированная под контролы UiSize::Small).
    // Автоматически переводит каретку к связанному контролу с зазором theme.filterBar.labelSpacing
    static void Label(const char* text, UiVariant variant = UiVariant::Default);

private:
    friend class LeftScope;
    friend class RightScope;

    const char* m_id;
    float m_baseHeight;
    float m_actualHeight = 0.0f;
    bool m_open = false;
    bool m_ended = false;

    ImVec2 m_startPos = ImVec2(0, 0);
    float m_availWidth = 0.0f;
    float m_leftWidth = 0.0f;
    float m_rightWidth = 0.0f;
    ImGuiID m_storageId = 0;
};
