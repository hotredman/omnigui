#pragma once

#include "omnikit/UiTheme.hpp"
#include <string>
#include <functional>

namespace OmniKit {

class ContextMenu;

// Indicator parameters (designated initializers):
//
//     Indicator speed({.title = "THROUGHPUT", .precision = 1, .unit = "MB/s", .digits = 5, .tare = true});
struct IndicatorOptions {
    std::string title;
    double      value     = 0.0;
    int         precision = 2;       // decimal places, 0..5
    std::string unit;
    int         digits    = 0;       // number of digits for panel width calculation; 0 — based on value
    bool        tare      = false;   // show tare/zero button
};

class Indicator {
public:
    using MenuCallback = std::function<void(ContextMenu& menu)>;

    explicit Indicator(const IndicatorOptions& options);

    // The panel title is rendered in uppercase (component styling)
    void SetTitle(std::string title);
    void SetValue(const std::string& value);
    void SetValue(double value);
    void SetUnit(const std::string& unit);
    void SetDigitsCount(int digits);
    void SetPrecision(int precision);
    void SetShowTare(bool show);
    void SetAllowPrecisionChange(bool allow) { m_allowPrecisionChange = allow; }
    void SetOnTare(std::function<void()> callback);
    // Precision selection in the value menu (programmatic SetPrecision does not notify)
    void SetOnPrecisionChanged(std::function<void(int)> callback);
    // Menu content on clicking the title or the unit of measurement; without
    // a handler, the zone is not clickable
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

    // Calculates panel width based on content, number of digits, and style fonts
    float CalculateWidth() const;
    float CalculateWidth(const IndicatorStyle& style) const;

    // Canonical render: 0 parameters, uses settings from UiTheme::Get()
    void Render();

    // Optional style override for a single widget
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
    // "Menu open" states for clickable zones: title, precision, units
    bool m_titleMenuOpen = false;
    bool m_precisionMenuOpen = false;
    bool m_unitMenuOpen = false;
    void RenderMenuZone(const char* name, bool& menuOpen, const MenuZone& zone, const char* tooltip,
                        const MenuCallback& fillMenu);


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

} // namespace OmniKit
