#pragma once

#include "ui/components/Scope.hpp"
#include "ui/components/UiTheme.hpp"
#include <imgui.h>

// Параметры панели фильтрации
struct FilterBarOptions {
    const char* key    = "##FilterBar";   // идентичность панели
    float       height = 0.0f;            // базовые px; 0 — theme.filterBar.height
};

// Параметры текстовой подписи панели (текст передаётся позиционно)
struct FilterBarLabelOptions {
    UiVariant variant = UiVariant::Default;
};

// Внутристраничная панель фильтрации, поиска и действий (FilterBar)
// Рендерится внутри рабочей области контента (в отличие от глобального Toolbar каркаса)
// RAII-область; левая и правая зоны — вложенные RAII-области:
//
//     if (FilterBar bar({.key = "filters"}); bar) {
//         if (auto left = bar.Left()) { FilterBar::Label("Search:"); ... }
//         if (auto right = bar.Right()) { ... }
//     }
class FilterBar : public Scope {
public:
    // Левая зона панели (область открывается в конструкторе и закрывается в деструкторе)
    class LeftScope : public Scope {
    public:
        explicit LeftScope(FilterBar& parent);
        ~LeftScope();
    private:
        FilterBar* m_parent;
    };

    // Правая зона панели, прижатая к правому краю
    class RightScope : public Scope {
    public:
        explicit RightScope(FilterBar& parent);
        ~RightScope();
    private:
        FilterBar* m_parent;
    };

    explicit FilterBar(const FilterBarOptions& options = {});
    ~FilterBar();

    LeftScope Left() { return LeftScope(*this); }
    RightScope Right() { return RightScope(*this); }

    float GetAvailableWidth() const { return m_availWidth; }

    // Доступная ширина для растягивающегося элемента в левой группе (в базовых unscaled пикселях)
    // Если minWidth <= 0, используется theme.filterBar.minSearchWidth
    float GetStretchWidth(float minWidth = 0.0f) const;

    // Шаг между элементами в одной группе (theme.filterBar.itemSpacing)
    static void NextItem();

    // Шаг между разными смысловыми группами (theme.filterBar.groupSpacing)
    static void NextGroup();

    // Произвольный шаг между элементами (базовые px)
    static void Spacing(float spacing);

    // Текстовая подпись внутри панели (вертикально отцентрированная под контролы UiSize::Small).
    // Автоматически переводит каретку к связанному контролу с зазором theme.filterBar.labelSpacing
    static void Label(const char* text, const FilterBarLabelOptions& options = {});

private:
    friend class LeftScope;
    friend class RightScope;

    const char* m_id;
    float m_baseHeight;
    float m_actualHeight = 0.0f;

    ImVec2 m_startPos = ImVec2(0, 0);
    float m_availWidth = 0.0f;
    float m_leftWidth = 0.0f;
    float m_rightWidth = 0.0f;
    ImGuiID m_storageId = 0;
};
