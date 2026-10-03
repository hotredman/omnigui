#include "CalcAreaPage.hpp"
#include "omnikit.hpp"

#include <cmath>
#include <cstdint>

namespace OmniKitShowcase {

namespace {

enum class SpecimenShape : uint8_t {
    Flat = 0,    // Flat (b x a)
    Round,       // Round (Ø d)
    Tube,        // Tubular (D x s)
    Wire,        // Wire (Ø d)
    Custom,      // Custom section (S₀)
    Count
};

inline constexpr const char* const kSpecimenShapeLabels[] = {
    "Flat (b x a)",
    "Round (Ø d)",
    "Tubular (D x s)",
    "Wire (Ø d)",
    "Custom Section (S₀)"
};

inline constexpr double kPi = 3.14159265358979323846;

inline double CalculateInitialArea(SpecimenShape shape, double width, double thickness,
                                   double diameter, double outerDiameter, double wallThickness,
                                   double customArea) {
    switch (shape) {
        case SpecimenShape::Flat:
            return (width > 0.0 && thickness > 0.0) ? (width * thickness) : 0.0;
        case SpecimenShape::Round:
        case SpecimenShape::Wire:
            return diameter > 0.0 ? (kPi * diameter * diameter / 4.0) : 0.0;
        case SpecimenShape::Tube:
            return (outerDiameter > 0.0 && wallThickness > 0.0 && wallThickness * 2.0 < outerDiameter)
                       ? (kPi * wallThickness * (outerDiameter - wallThickness))
                       : 0.0;
        case SpecimenShape::Custom:
            return customArea > 0.0 ? customArea : 0.0;
        default:
            return 0.0;
    }
}

struct SpecimenData {
    SpecimenShape shape          = SpecimenShape::Flat;
    float         width          = 15.0f;   // b0 (mm)
    float         thickness      = 3.0f;    // a0 (mm)
    float         diameter       = 10.0f;   // d0 (mm)
    float         outerDiameter  = 20.0f;   // D (mm)
    float         wallThickness  = 2.0f;    // s (mm)
    float         customArea     = 45.0f;   // S0 (mm²)
    float         l0             = 50.0f;   // L0 (mm)
    float         lc             = 70.0f;   // Lc (mm)
    float         calculatedArea = 45.0f;   // S0 (mm²)

    void UpdateCalculatedArea() {
        calculatedArea = static_cast<float>(CalculateInitialArea(
            shape, width, thickness, diameter, outerDiameter, wallThickness, customArea));
    }
};

SpecimenData s_specimen;

bool RenderCrossSectionCard(SpecimenData& s) {
    bool changed = false;
    const auto edited = [&](bool fieldChanged) {
        if (fieldChanged) {
            s.UpdateCalculatedArea();
            changed = true;
        }
    };

    if (Card card(CardOptions{.title = "Cross-Section Shape & Dimensions", .key = "SecDimensions"}); card) {
        edited(card.Combo(s.shape, kSpecimenShapeLabels, {.label = "Cross-section shape", .col = Col::Full()}));

        switch (s.shape) {
            case SpecimenShape::Flat:
                edited(card.Float(s.width,     {.label = "Initial width (b₀)",     .unit = "mm", .format = "%.2f", .col = Col::Half()}));
                edited(card.Float(s.thickness, {.label = "Initial thickness (a₀)", .unit = "mm", .format = "%.2f", .col = Col::Half()}));
                break;

            case SpecimenShape::Round:
                edited(card.Float(s.diameter, {.label = "Initial diameter (d₀)", .unit = "mm", .format = "%.2f", .col = Col::Half()}));
                break;

            case SpecimenShape::Wire:
                edited(card.Float(s.diameter, {.label = "Wire diameter (d₀)", .unit = "mm", .format = "%.2f", .col = Col::Half()}));
                break;

            case SpecimenShape::Tube:
                edited(card.Float(s.outerDiameter, {.label = "Outer diameter (D)", .unit = "mm", .format = "%.2f", .col = Col::Half()}));
                edited(card.Float(s.wallThickness, {.label = "Wall thickness (s)", .unit = "mm", .format = "%.2f", .col = Col::Half()}));
                break;

            case SpecimenShape::Custom:
                edited(card.Float(s.customArea, {.label = "Initial area (S₀)", .unit = "mm²", .format = "%.2f", .col = Col::Half()}));
                break;

            default:
                break;
        }
    }
    return changed;
}

bool RenderLengthsCard(SpecimenData& s) {
    bool changed = false;
    if (Card card(CardOptions{.title = "Basic Gauge Lengths", .key = "SecLengths"}); card) {
        changed |= card.Float(s.l0, {.label = "Initial gauge length (L₀)", .unit = "mm", .format = "%.1f", .col = Col::Half()});
        changed |= card.Float(s.lc, {.label = "Parallel length (Lc)",      .unit = "mm", .format = "%.1f", .col = Col::Half()});
    }
    return changed;
}

void RenderCalculatedAreaCard(SpecimenData& s) {
    if (Card card(CardOptions{.title = "Initial Cross-Section Area (S₀)", .key = "SecAreaCard"}); card) {
        s.UpdateCalculatedArea();
        card.Value(s.calculatedArea, {.unit = "mm²", .format = "%.2f", .col = Col::Full()});
    }
}

}  // namespace

void RenderCalcAreaPage() {
    ColumnLayout flow;

    // Left column (50%): Cross-section dimensions and gauge lengths
    if (auto col = flow.Col(Col::Half())) {
        RenderCrossSectionCard(s_specimen);
        Spacer(12);
        RenderLengthsCard(s_specimen);
    }

    // Right column (50%): Automatically calculated cross-section area S₀
    if (auto col = flow.Col(Col::Half())) {
        RenderCalculatedAreaCard(s_specimen);
    }
}

}  // namespace OmniKitShowcase
