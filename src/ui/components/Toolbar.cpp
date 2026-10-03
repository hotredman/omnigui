#include "ui/components/Toolbar.hpp"

Toolbar::Toolbar(const ToolbarOptions& options)
    : Bar(MakeBarMetrics(options.style ? *options.style : UiTheme::Get().toolbar),
          "##ToolbarWindow", "##ToolbarZones",
          (options.posYPx >= 0.0f) ? options.posYPx : UiTheme::Get().HeaderHeight(), options.heightPx)
{
}
