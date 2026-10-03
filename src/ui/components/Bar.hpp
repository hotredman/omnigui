#pragma once

#include "ui/components/Scope.hpp"
#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include "ui/components/Button.hpp"
#include <imgui.h>

// Общая основа горизонтальных полос окна (Header, Toolbar): окно во всю ширину
// экрана с тремя зонами — Left, Center и Right. Ширины Left/Right измеряются
// после отрисовки и запоминаются в ImGuiStorage, поэтому Center получает
// остаток и сам объект полосы не хранит состояния между кадрами.

// Базовые (немасштабированные) метрики полосы — общее подмножество HeaderStyle и ToolbarStyle
struct BarMetrics {
    float height;
    float paddingX;
    float paddingY;
    float zoneSpacing;
    float separatorSize;
    float defaultLeftWidth;
    float defaultRightWidth;
    ImU32 colBg;
    ImU32 colSeparator;
};

template <typename Style>
BarMetrics MakeBarMetrics(const Style& s) {
    return { s.height, s.paddingX, s.paddingY, s.zoneSpacing, s.separatorSize,
             s.defaultLeftWidth, s.defaultRightWidth, s.colBg, s.colSeparator };
}

// Геометрия полосы в пикселях (после масштабирования), общая для зон
struct BarLayout {
    float paddingX = 0.0f;
    float paddingY = 0.0f;
    float zoneSpacing = 0.0f;
    float contentHeight = 0.0f;      // высота рабочей области зоны, px
    float baseContentHeight = 0.0f;  // она же в базовых единицах (для компонентов, масштабирующих сами)
    float leftWidth = 0.0f;
    float rightWidth = 0.0f;
    ImGuiID storageId = 0;
};

class Bar;

// Зона полосы. Не создаётся напрямую: Bar::Left()/Center()/Right()
class BarZone : public Scope {
public:
    // Высота рабочей области зоны, px
    float Height() const { return m_layout ? m_layout->contentHeight : 0.0f; }

    // Подпись, выровненная по вертикали по центру полосы
    void Label(const char* text, UiVariant variant = UiVariant::Secondary);

    // Кнопка высотой с рабочую область полосы (options.size игнорируется)
    bool Button(const char* label, const ButtonOptions& options = {});

protected:
    BarZone(BarLayout* layout, bool active);
    ~BarZone() = default;

    // Закрывает группу зоны и возвращает измеренную ширину
    float EndZone();

    BarLayout* m_layout = nullptr;
};

class LeftZone : public BarZone {
public:
    ~LeftZone();
private:
    friend class Bar;
    LeftZone(BarLayout* layout, bool active);
};

class CenterZone : public BarZone {
public:
    ~CenterZone();
    // Ширина, оставшаяся между левой и правой зонами, px
    float Width() const { return m_width; }
private:
    friend class Bar;
    CenterZone(BarLayout* layout, bool active);
    float m_width = 0.0f;
};

class RightZone : public BarZone {
public:
    ~RightZone();
private:
    friend class Bar;
    RightZone(BarLayout* layout, bool active);
};

class Bar : public Scope {
public:
    ~Bar();

    LeftZone Left();
    CenterZone Center();
    RightZone Right();

    float GetHeight() const { return m_height; }
    float GetContentHeight() const { return m_layout.contentHeight; }

protected:
    // windowName / zonesId — идентификаторы окна и хранилища ширин зон;
    // posY — верх полосы (px), height <= 0 — высота из метрик
    Bar(const BarMetrics& metrics, const char* windowName, const char* zonesId,
        float posY, float height);

private:
    BarLayout m_layout;
    float m_height = 0.0f;
};
