#include "ui/components/Header.hpp"

Header::Header(const HeaderOptions& options)
    : Bar(MakeBarMetrics(options.style ? *options.style : UiTheme::Get().header),
          "##HeaderWindow", "##HeaderZones", 0.0f, options.heightPx)
{
}
