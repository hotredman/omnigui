#pragma once

#include "ui/components/UiTheme.hpp"
#include <string>
#include <functional>

class ContextMenu;

class Indicator {
public:
    using MenuCallback = std::function<void(ContextMenu& menu)>;

    Indicator(std::string title, std::string value, std::string unit, int digitsCount = 0, bool showTare = false);
    Indicator(std::string title, std::string value, std::string unit, bool showTare);
    Indicator(std::string title, double value, int precision, std::string unit, int digitsCount = 0, bool showTare = false);

    // Заголовок плашки выводится капсом (оформление компонента)
    void SetTitle(std::string title);
    void SetValue(const std::string& value);
    void SetValue(double value);
    void SetUnit(const std::string& unit);
    void SetDigitsCount(int digits);
    void SetPrecision(int precision);
    void SetShowTare(bool show);
    void SetAllowPrecisionChange(bool allow) { m_allowPrecisionChange = allow; }
    void SetOnTare(std::function<void()> callback);
    // Выбор числа знаков в меню значения (программный SetPrecision не сообщает)
    void SetOnPrecisionChanged(std::function<void(int)> callback);
    // Содержимое меню по клику на заголовок и на единицу измерения; без
    // обработчика зона не кликабельна
    void SetOnRenderTitleMenu(MenuCallback callback);
    void SetOnRenderUnitMenu(MenuCallback callback);
    void SetTareLabel(std::string label) { m_tareLabel = std::move(label); }

    const std::string& GetTitle() const { return m_title; }
    const std::string& GetValue() const { return m_value; }
    double GetNumericValue() const { return m_numericValue; }
    const std::string& GetUnit() const { return m_unit; }
    int GetDigitsCount() const { return m_digitsCount; }
    int GetPrecision() const { return m_precision; }
    bool GetAllowPrecisionChange() const { return m_allowPrecisionChange; }
    const std::string& GetTareLabel() const { return m_tareLabel; }
    bool HasTare() const { return m_showTare; }

    // Расчет ширины плашки на основе содержимого, числа разрядов и шрифтов стиля
    float CalculateWidth() const;
    float CalculateWidth(const IndicatorStyle& style) const;

    // Канонический рендер: 0 параметров, берёт настройки из UiTheme::Get()
    void Render();

    // Опциональный оверрайд стиля для единичного виджета
    void Render(const IndicatorStyle& style);

private:
    std::string m_title;
    std::string m_value;
    std::string m_unit;
    std::string m_tareLabel;
    double m_numericValue = 0.0;
    int m_precision = 2;
    int m_integerDigits = 2;
    int m_digitsCount = 0;
    bool m_showTare = false;
    bool m_allowPrecisionChange = true;
    std::function<void()> m_onTare;
    std::function<void(int)> m_onPrecisionChanged;
    MenuCallback m_onRenderTitleMenu;
    MenuCallback m_onRenderUnitMenu;

    struct MenuZone {
        ImVec2 pos;
        ImVec2 size;
        float fontSize;
        ImU32 color;
        float underlineThickness;
    };
    void RenderMenuZone(const char* name, const MenuZone& zone, const char* tooltip,
                        const MenuCallback& fillMenu);

    void InitFromValueString();

    mutable float m_cachedWidth = 0.0f;
    mutable float m_lastScale = 0.0f;
    mutable int m_lastPrecision = -1;
    mutable int m_lastDigitsCount = -1;
    mutable size_t m_lastValueLen = 0;
    mutable bool m_lastShowTare = false;
    mutable std::string m_lastTitle;
    mutable std::string m_lastUnit;
    mutable bool m_widthDirty = true;
};
