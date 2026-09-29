#pragma once

#include <imgui.h>

// ============================================================================
// Дескриптор стиля графиков (ChartStyle)
// ============================================================================
struct ChartStyle {
    // Цвета холста и рамки
    ImU32 colBg             = 0;
    ImU32 colBorder         = 0;
    float cornerRadius      = 6.0f;
    float borderSize        = 1.0f;

    // Кривая по умолчанию (серия / линия / маркер без своего цвета)
    ImU32 colLine           = 0;

    // Сетка
    ImU32 colMajorGrid      = 0;
    ImU32 colMinorGrid      = 0;
    float majorGridWidth    = 1.0f;
    float minorGridWidth    = 1.0f;

    // Оси и числовые метки
    ImU32 colAxis           = 0;
    ImU32 colTickText       = 0;
    ImU32 colAxisTitle      = 0;
    float tickFontSize      = 18.0f;
    float axisTitleFontSize = 20.0f;

    // Зонд / перекрестие
    ImU32 colCrosshair      = 0;
    float crosshairWidth    = 1.0f;

    // Проекции маркеров
    ImU32 colProjection     = 0;

    // Всплывающая подсказка (Tooltip)
    ImU32 colTooltipBg      = 0;
    ImU32 colTooltipBorder  = 0;
    ImU32 colTooltipText    = 0;
};
