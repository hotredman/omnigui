#include "ui/components/Header.hpp"

Header::Header(float height, const HeaderStyle* customStyle)
    : Bar(MakeBarMetrics(customStyle ? *customStyle : UiTheme::Get().header),
          "##HeaderWindow", "##HeaderZones", 0.0f, height)
{
}
