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
    Stress,         // Напряжение и давление
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
    // Сила (база: Н)
    N, KN, Kgf, Lbf,
    // Напряжение / давление (база: МПа)
    MPa, NMm2, GPa, KPa, Bar, KgfMm2, KgfCm2, Psi,
    // Деформация (база: %)
    Percent, Ratio, Permille,
    // Длина / перемещение (база: мм)
    Mm, Um, Cm, M,
    // Площадь (база: мм²)
    Mm2, Cm2, M2,
    // Энергия / работа (база: Дж)
    J, KJ, NM,
    // Скорость перемещения (база: мм/мин)
    MmMin, MmS, MS,
    // Время (база: с)
    S, Min, Ms, H,
    // Температура (база: °C)
    C,
    // Безразмерная величина
    None
};

namespace Units {

const char* Key(PhysicalUnit unit);                         // "kN"
std::optional<PhysicalUnit> FromKey(std::string_view key);  // только канонический ключ
const char* Label(PhysicalUnit unit);                       // "кН"

UnitCategory CategoryOf(PhysicalUnit unit);
bool SameCategory(PhysicalUnit a, PhysicalUnit b);
PhysicalUnit BaseUnit(UnitCategory category);
const char* CategoryLabel(UnitCategory category);
const std::vector<UnitCategory>& Categories();
const std::vector<PhysicalUnit>& UnitsOf(UnitCategory category);

// Значение в единице unit -> в базовой единице её категории
double ToBase(double value, PhysicalUnit unit);
// Перевод между единицами одной категории; разные категории — nullopt
std::optional<double> Convert(double value, PhysicalUnit from, PhysicalUnit to);

#ifdef OMNIGUI_HAS_NLOHMANN_JSON
PhysicalUnit Get(const nlohmann::json& j, const char* field, PhysicalUnit fallback);
#endif

}  // namespace Units

#ifdef OMNIGUI_HAS_NLOHMANN_JSON
void to_json(nlohmann::json& j, PhysicalUnit unit);
void from_json(const nlohmann::json& j, PhysicalUnit& unit);
#endif
