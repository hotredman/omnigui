#include "core/units/Units.hpp"

#ifdef OMNIGUI_HAS_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

#include <array>
#include <stdexcept>
#include <string>

namespace {

struct UnitInfo {
    PhysicalUnit unit;
    const char* key;
    const char* label;
    UnitCategory category;
    double scaleToBase;  // value_base = value * scaleToBase
};

// Порядок строк — порядок PhysicalUnit
constexpr UnitInfo kUnits[] = {
    {PhysicalUnit::N, "N", "Н", UnitCategory::Force, 1.0},
    {PhysicalUnit::KN, "kN", "кН", UnitCategory::Force, 1000.0},
    {PhysicalUnit::Kgf, "kgf", "кгс", UnitCategory::Force, 9.80665},
    {PhysicalUnit::Lbf, "lbf", "lbf", UnitCategory::Force, 4.44822},

    {PhysicalUnit::MPa, "MPa", "МПа", UnitCategory::Stress, 1.0},
    {PhysicalUnit::NMm2, "N_mm2", "Н/мм²", UnitCategory::Stress, 1.0},
    {PhysicalUnit::GPa, "GPa", "ГПа", UnitCategory::Stress, 1000.0},
    {PhysicalUnit::KPa, "kPa", "кПа", UnitCategory::Stress, 0.001},
    {PhysicalUnit::Bar, "bar", "бар", UnitCategory::Stress, 0.1},
    {PhysicalUnit::KgfMm2, "kgf_mm2", "кгс/мм²", UnitCategory::Stress, 9.80665},
    {PhysicalUnit::KgfCm2, "kgf_cm2", "кгс/см²", UnitCategory::Stress, 0.0980665},
    {PhysicalUnit::Psi, "psi", "psi", UnitCategory::Stress, 0.00689476},

    {PhysicalUnit::Percent, "percent", "%", UnitCategory::Strain, 1.0},
    {PhysicalUnit::Ratio, "ratio", "доли", UnitCategory::Strain, 100.0},
    {PhysicalUnit::Permille, "permille", "‰", UnitCategory::Strain, 0.1},

    {PhysicalUnit::Mm, "mm", "мм", UnitCategory::Length, 1.0},
    {PhysicalUnit::Um, "um", "мкм", UnitCategory::Length, 0.001},
    {PhysicalUnit::Cm, "cm", "см", UnitCategory::Length, 10.0},
    {PhysicalUnit::M, "m", "м", UnitCategory::Length, 1000.0},

    {PhysicalUnit::Mm2, "mm2", "мм²", UnitCategory::Area, 1.0},
    {PhysicalUnit::Cm2, "cm2", "см²", UnitCategory::Area, 100.0},
    {PhysicalUnit::M2, "m2", "м²", UnitCategory::Area, 1000000.0},

    {PhysicalUnit::J, "J", "Дж", UnitCategory::Energy, 1.0},
    {PhysicalUnit::KJ, "kJ", "кДж", UnitCategory::Energy, 1000.0},
    {PhysicalUnit::NM, "N_m", "Н·м", UnitCategory::Energy, 1.0},

    {PhysicalUnit::MmMin, "mm_min", "мм/мин", UnitCategory::Speed, 1.0},
    {PhysicalUnit::MmS, "mm_s", "мм/с", UnitCategory::Speed, 60.0},
    {PhysicalUnit::MS, "m_s", "м/с", UnitCategory::Speed, 60000.0},

    {PhysicalUnit::S, "s", "с", UnitCategory::Time, 1.0},
    {PhysicalUnit::Min, "min", "мин", UnitCategory::Time, 60.0},
    {PhysicalUnit::Ms, "ms", "мс", UnitCategory::Time, 0.001},
    {PhysicalUnit::H, "h", "ч", UnitCategory::Time, 3600.0},

    {PhysicalUnit::C, "C", "°C", UnitCategory::Temperature, 1.0},

    {PhysicalUnit::None, "none", "—", UnitCategory::Dimensionless, 1.0},
};
constexpr size_t kUnitCount = sizeof(kUnits) / sizeof(kUnits[0]);
static_assert(kUnitCount == static_cast<size_t>(PhysicalUnit::None) + 1,
              "kUnits: размер не равен числу элементов PhysicalUnit");

struct CategoryInfo {
    UnitCategory category;
    const char* label;
    PhysicalUnit base;
};

constexpr CategoryInfo kCategories[] = {
    {UnitCategory::Force, "Сила / Нагрузка", PhysicalUnit::N},
    {UnitCategory::Stress, "Напряжение / Давление", PhysicalUnit::MPa},
    {UnitCategory::Strain, "Деформация / Удлинение", PhysicalUnit::Percent},
    {UnitCategory::Length, "Длина / Перемещение", PhysicalUnit::Mm},
    {UnitCategory::Area, "Площадь сечения", PhysicalUnit::Mm2},
    {UnitCategory::Energy, "Энергия / Работа", PhysicalUnit::J},
    {UnitCategory::Speed, "Скорость перемещения", PhysicalUnit::MmMin},
    {UnitCategory::Time, "Время", PhysicalUnit::S},
    {UnitCategory::Temperature, "Температура", PhysicalUnit::C},
    {UnitCategory::Dimensionless, "Безразмерная величина", PhysicalUnit::None},
};
constexpr size_t kCategoryCount = sizeof(kCategories) / sizeof(kCategories[0]);
static_assert(kCategoryCount == static_cast<size_t>(UnitCategory::Dimensionless) + 1,
              "kCategories: размер не равен числу элементов UnitCategory");

constexpr bool TablesInEnumOrder() {
    for (size_t i = 0; i < kUnitCount; ++i)
        if (static_cast<size_t>(kUnits[i].unit) != i) return false;
    for (size_t i = 0; i < kCategoryCount; ++i)
        if (static_cast<size_t>(kCategories[i].category) != i) return false;
    return true;
}
static_assert(TablesInEnumOrder(), "Таблицы единиц и категорий должны следовать в порядке перечислений");

const UnitInfo& Info(PhysicalUnit unit) { return kUnits[static_cast<size_t>(unit)]; }
const CategoryInfo& Info(UnitCategory category) { return kCategories[static_cast<size_t>(category)]; }

}  // namespace

namespace Units {

const char* Key(PhysicalUnit unit) { return Info(unit).key; }

std::optional<PhysicalUnit> FromKey(std::string_view key) {
    for (const UnitInfo& u : kUnits)
        if (key == u.key) return u.unit;
    return std::nullopt;
}

const char* Label(PhysicalUnit unit) { return Info(unit).label; }

UnitCategory CategoryOf(PhysicalUnit unit) { return Info(unit).category; }

bool SameCategory(PhysicalUnit a, PhysicalUnit b) { return CategoryOf(a) == CategoryOf(b); }

PhysicalUnit BaseUnit(UnitCategory category) { return Info(category).base; }

const char* CategoryLabel(UnitCategory category) { return Info(category).label; }

const std::vector<UnitCategory>& Categories() {
    static const std::vector<UnitCategory> kAll = [] {
        std::vector<UnitCategory> out;
        for (const CategoryInfo& c : kCategories) out.push_back(c.category);
        return out;
    }();
    return kAll;
}

const std::vector<PhysicalUnit>& UnitsOf(UnitCategory category) {
    static const std::array<std::vector<PhysicalUnit>, kCategoryCount> kByCategory = [] {
        std::array<std::vector<PhysicalUnit>, kCategoryCount> out;
        for (const UnitInfo& u : kUnits) out[static_cast<size_t>(u.category)].push_back(u.unit);
        return out;
    }();
    return kByCategory[static_cast<size_t>(category)];
}

double ToBase(double value, PhysicalUnit unit) { return value * Info(unit).scaleToBase; }

std::optional<double> Convert(double value, PhysicalUnit from, PhysicalUnit to) {
    if (!SameCategory(from, to)) return std::nullopt;
    if (from == to) return value;
    return value * Info(from).scaleToBase / Info(to).scaleToBase;
}

#ifdef OMNIGUI_HAS_NLOHMANN_JSON
PhysicalUnit Get(const nlohmann::json& j, const char* field, PhysicalUnit fallback) {
    const auto it = j.find(field);
    if (it == j.end() || !it->is_string()) return fallback;
    return FromKey(it->get_ref<const std::string&>()).value_or(fallback);
}
#endif

}  // namespace Units

#ifdef OMNIGUI_HAS_NLOHMANN_JSON
void to_json(nlohmann::json& j, PhysicalUnit unit) { j = Units::Key(unit); }

void from_json(const nlohmann::json& j, PhysicalUnit& unit) {
    const std::optional<PhysicalUnit> parsed =
        j.is_string() ? Units::FromKey(j.get_ref<const std::string&>()) : std::nullopt;
    if (!parsed) throw std::invalid_argument("Неизвестная единица измерения: " + j.dump());
    unit = *parsed;
}
#endif
