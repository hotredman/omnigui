#pragma once

#include <optional>
#include <string_view>
#include <vector>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#define OMNIGUI_HAS_NLOHMANN_JSON 1
#endif

enum class UnitCategory {
    Force,
    Stress,         // Stress and pressure
    Strain,
    Length,
    Area,
    Energy,
    Speed,
    Time,
    Temperature,
    Dimensionless
};

enum class PhysicalUnit {
    // Force (base: N)
    N, KN, Kgf, Lbf,
    // Stress / pressure (base: MPa)
    MPa, NMm2, GPa, KPa, Bar, KgfMm2, KgfCm2, Psi,
    // Strain (base: %)
    Percent, Ratio, Permille,
    // Length / displacement (base: mm)
    Mm, Um, Cm, M,
    // Area (base: mm²)
    Mm2, Cm2, M2,
    // Energy / work (base: J)
    J, KJ, NM,
    // Speed (base: mm/min)
    MmMin, MmS, MS,
    // Time (base: s)
    S, Min, Ms, H,
    // Temperature (base: °C)
    C,
    // Dimensionless quantity
    None
};

namespace Units {

const char* Key(PhysicalUnit unit);                         // "kN"
std::optional<PhysicalUnit> FromKey(std::string_view key);  // canonical key only
const char* Label(PhysicalUnit unit);                       // "kN"

UnitCategory CategoryOf(PhysicalUnit unit);
bool SameCategory(PhysicalUnit a, PhysicalUnit b);
PhysicalUnit BaseUnit(UnitCategory category);
const char* CategoryLabel(UnitCategory category);
const std::vector<UnitCategory>& Categories();
const std::vector<PhysicalUnit>& UnitsOf(UnitCategory category);

// Value in unit -> in base unit of its category
double ToBase(double value, PhysicalUnit unit);
// Convert between units of the same category; different categories return nullopt
std::optional<double> Convert(double value, PhysicalUnit from, PhysicalUnit to);

#ifdef OMNIGUI_HAS_NLOHMANN_JSON
PhysicalUnit Get(const nlohmann::json& j, const char* field, PhysicalUnit fallback);
#endif

}  // namespace Units

#ifdef OMNIGUI_HAS_NLOHMANN_JSON
void to_json(nlohmann::json& j, PhysicalUnit unit);
void from_json(const nlohmann::json& j, PhysicalUnit& unit);
#endif
