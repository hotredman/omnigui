#pragma once

#include "ui/components/Scope.hpp"
#include <imgui.h>
#include <vector>
#include <algorithm>
#include <cmath>

// ============================================================================
// Доля строки в 12-колоночной сетке (Bootstrap / Grid style)
// ============================================================================
struct Col {
    int span = 6; // По умолчанию 6 из 12 (двухколоночный режим = 50% ширины)
    float reservedPx = 0.0f; // Для Col::Fill(): резервируемая ширина справа (например, под кнопки)

    constexpr Col() : span(6), reservedPx(0.0f) {}
    constexpr Col(int s) : span(s < -1 ? -1 : (s > 12 ? 12 : s)), reservedPx(0.0f) {}
    constexpr Col(int s, float reserved) : span(s < -1 ? -1 : (s > 12 ? 12 : s)), reservedPx(reserved) {}

    static constexpr Col Auto()                         { return Col{0, 0.0f};  } // Автоматическая контентная ширина (не привязана к сетке)
    static constexpr Col Fit()                          { return Col{0, 0.0f};  } // Синоним для Auto
    static constexpr Col Fill(float reservedRight = 0.0f){ return Col{-1, reservedRight}; } // Растягивание до конца строки (с опциональным резервом под кнопки справа)
    static constexpr Col Full()                         { return Col{12, 0.0f}; } // 100% строки
    static constexpr Col Half()                         { return Col{6, 0.0f};  } // 50% строки (2 колонки)
    static constexpr Col Third()                        { return Col{4, 0.0f};  } // 33.3% строки (3 колонки)
    static constexpr Col TwoThirds()                    { return Col{8, 0.0f};  } // 66.7% строки (2/3)
    static constexpr Col Quarter()                      { return Col{3, 0.0f};  } // 25% строки (4 колонки)
    static constexpr Col Sixth()                        { return Col{2, 0.0f};  } // 16.7% строки (6 колонок)
    static constexpr Col Custom(int s)                  { return Col{s, 0.0f};  }

    bool IsAuto() const { return span == 0; }
    bool IsFill() const { return span < 0; }
};

// Ширина колонки в span долей из 12 при доступной ширине avail и зазоре gap (все значения в px).
// Общая формула для ColumnLayout и Card.
inline float SpanWidth(int span, float avail, float gap) {
    span = std::clamp(span, 1, 12);
    if (span == 12) {
        return avail;
    }
    const float width = (span * avail - (12 - span) * gap) / 12.0f;
    return std::max(20.0f, std::floor(width));
}

// ============================================================================
// Адаптивная 12-колоночная раскладка с автопереносом строк (ColumnLayout).
// RAII-область: колонки открываются через Col(), строки переносятся сами.
//
//     ColumnLayout row({.columnGap = 12});
//     if (auto c = row.Col(Col::TwoThirds())) { ... }
//     if (auto c = row.Col(Col::Third()))     { ... }   // строка заполнена, дальше — новая строка
// ============================================================================
struct ColumnLayoutOptions {
    float widthPx   = 0.0f;    // итоговые px; 0 — вся доступная ширина
    float columnGap = -1.0f;   // базовые px; < 0 — UiTheme::SpacingLarge()
    float rowGap    = -1.0f;   // базовые px; < 0 — UiTheme::CardRowSpacing()
};

class ColumnLayout : public Scope {
public:
    // RAII-колонка: if (auto c = row.Col(Col::Half())) { ... }
    class ColumnScope : public Scope {
    public:
        ~ColumnScope();

        float Width() const { return m_width; }
        void AlignWithInput();
        void RightAlign(float itemWidth);

    private:
        friend class ColumnLayout;
        ColumnScope(ColumnLayout* parent, float width, bool active)
            : m_parent(parent), m_width(width) {
            m_open = active;
        }

        ColumnLayout* m_parent = nullptr;
        float m_width = 0.0f;
    };

    explicit ColumnLayout(const ColumnLayoutOptions& options = {});
    ~ColumnLayout();

    ColumnScope Col(::Col col = ::Col::Half());

    // Принудительный переход на новую строку
    void NextRow();

    // Геометрия
    float GetAvailableWidth() const { return m_rowAvailWidth; }
    float CalculateSpanWidth(int span) const;
    float GetColumnSpacing() const { return m_columnSpacing; }
    float GetRowSpacing() const { return m_rowSpacing; }
    int GetRemainingSpan() const { return 12 - m_rowSpanUsed; }

    // Активная ширина колонки в текущем потоке (опрашивается компонентами, например Card)
    static float CurrentColumnWidth();

private:
    bool BeginCol(::Col col);
    void EndCol();

    float m_totalAvailWidth = 0.0f;
    float m_rowAvailWidth = 0.0f;
    float m_columnSpacing = 16.0f;
    float m_rowSpacing = 16.0f;

    int m_rowSpanUsed = 0;
    int m_colIndexInRow = 0;
    float m_accumulatedWidth = 0.0f;
    int m_activeSpan = 0;
    float m_activeWidth = 0.0f;
    bool m_colOpen = false;
    bool m_needsNewRowSpacing = false;

    static thread_local float s_currentColumnWidth;
};