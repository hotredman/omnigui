#include "ui/components/Toolbar.hpp"

Toolbar::Toolbar(float posY, float height, const ToolbarStyle* customStyle)
    : Bar(MakeBarMetrics(customStyle ? *customStyle : UiTheme::Get().toolbar),
          "##ToolbarWindow", "##ToolbarZones",
          (posY >= 0.0f) ? posY : UiTheme::Get().HeaderHeight(), height)
{
}
