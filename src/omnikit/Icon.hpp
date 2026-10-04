#pragma once

#include <imgui.h>
#include <string>
#include <string_view>
#include <cstdint>

namespace OmniKit {

// Universal pictogram class (Value Object).
// Combines a built-in catalog of vector icons from the design system
// with support for external icons from files and assets without duplicating declarations.
class Icon {
public:
    // Catalog of built-in design system pictograms
    enum Id : uint8_t {
        None = 0,

        // Sidebar and general (named by visual shape)
        Database,        // Database cylinder disk stack (Archive)
        List,            // Bulleted item list (Event Log)
        Cog,             // Spoked machinery gear (Machine Parameters)
        Target,          // Crosshair target (Sensor Calibration)
        Gamepad,         // Gamepad body (Control Panel)
        Gear,            // 6-toothed engineering gear (Settings / Setup)
        LineChart,       // Document with polyline chart (Result Processing)
        Clipboard,       // Clipboard with top clip (Sample Sheet)
        BarChart,        // Column bar chart (Test Series)

        // Remote control and actuator drive (Remote Control)
        Play,            // Filled right-pointing triangle ("START")
        Square,          // Filled regular square ("STOP")
        Pause,           // Two parallel vertical bars ("Pause")
        RotateCcw,       // Curved counter-clockwise return arrow ("Return")
        ArrowDownToLine, // Downward arrow into tray/platform ("Unload")
        RefreshCw,       // Circular refresh arrow ("Fault Reset")
        ArrowUp,         // Triangular upward arrow ("UP")
        ArrowDown,       // Triangular downward arrow ("DOWN")
        ChevronsUp,      // Double upward chevrons (Fast Up)
        ChevronsDown,    // Double downward chevrons (Fast Down)

        // Navigation and scrolling (Navigation / Carousel / Scrolling)
        ChevronLeft,     // Left chevron (<) for horizontal scrolling of carousels/tabs
        ChevronRight,    // Right chevron (>) for horizontal scrolling of carousels/tabs
        ArrowLeft,       // Left arrow (alias for ChevronLeft)
        ArrowRight,      // Right arrow (alias for ChevronRight)

        // System and header (Header / Controls)
        Power,           // Circle with top vertical line (Power switch)
        Maximize,        // 4 expanding corners (Fullscreen)
        Minimize,        // 4 contracting corners (Windowed mode)
        Sun,             // Sun with rays (Light theme)
        Moon,            // Crescent moon (Dark theme)
        Zero,            // Tare zero target with center dot (Tare)
        Check,           // Checkmark selection
        Pencil,          // Pencil / Edit in place
        Plus,            // Plus / Create / Add / Zoom in
        Minus,           // Minus / Remove / Zoom out
        Close,           // Cross / Clear / Reset (X)
        Aa,              // Dual letters Aa (Interface scale / Fonts)
        Image,           // Image frame with landscape picture

        // Data actions and PLC (Machine Actions)
        Upload,          // Upward arrow ("Write" to PLC / Upload)
        Download,        // Downward arrow ("Read" from PLC / Download)
        Refresh,         // Circular arrow ("Factory Defaults" / Fault Reset / Refresh)

        // Archive and files (Archive / File Actions)
        Folder,          // Folder (Open data directory)
        Trash,           // Trash can (Delete project)
        Copy,            // Two documents (Duplicate project)
        ExternalLink,    // Outward arrow (Open project)
    };

    enum class SourceType : uint8_t {
        None = 0,
        Builtin,
        Asset,
        File
    };

    // Constructors
    Icon() noexcept = default;
    Icon(Id id) noexcept
        : m_type(id == None ? SourceType::None : SourceType::Builtin)
        , m_id(id)
        , m_tintable(true)
    {}

    // Factory methods for external files and assets
    static Icon File(std::string_view filePath, bool tintable = true) {
        if (filePath.empty()) return Icon();
        return Icon(SourceType::File, filePath, tintable);
    }
    static Icon file(std::string_view filePath, bool tintable = true) {
        return File(filePath, tintable);
    }

    static Icon Asset(std::string_view assetPath, bool tintable = true) {
        if (assetPath.empty()) return Icon();
        return Icon(SourceType::Asset, assetPath, tintable);
    }
    static Icon asset(std::string_view assetPath, bool tintable = true) {
        return Asset(assetPath, tintable);
    }

    // State and type queries
    bool IsEmpty() const noexcept { return m_type == SourceType::None; }
    bool IsValid() const noexcept { return m_type != SourceType::None; }
    explicit operator bool() const noexcept { return IsValid(); }

    SourceType GetType() const noexcept { return m_type; }
    Id GetId() const noexcept { return m_id; }
    const std::string& GetPath() const noexcept { return m_path; }
    bool IsTintable() const noexcept { return m_tintable; }

    // Comparison operators with Id (e.g., icon == Icon::None or icon != Icon::Play)
    bool operator==(Id id) const noexcept {
        if (id == None) return m_type == SourceType::None;
        return m_type == SourceType::Builtin && m_id == id;
    }
    bool operator!=(Id id) const noexcept {
        return !(*this == id);
    }
    friend bool operator==(Id id, const Icon& icon) noexcept {
        return icon == id;
    }
    friend bool operator!=(Id id, const Icon& icon) noexcept {
        return icon != id;
    }

    // Comparison operators between Icon instances
    bool operator==(const Icon& other) const noexcept {
        if (m_type != other.m_type) return false;
        if (m_type == SourceType::None) return true;
        if (m_type == SourceType::Builtin) return m_id == other.m_id;
        return m_path == other.m_path && m_tintable == other.m_tintable;
    }
    bool operator!=(const Icon& other) const noexcept {
        return !(*this == other);
    }

    // Instance rendering methods
    void Draw(ImDrawList* dl, ImVec2 center, float size, ImU32 color) const;
    void DrawAt(ImDrawList* dl, ImVec2 pos, float size, ImU32 color) const;
    void Render(float size, ImU32 color) const;

    // Static rendering utilities
    static void Draw(const Icon& icon, ImDrawList* dl, ImVec2 center, float size, ImU32 color) {
        icon.Draw(dl, center, size, color);
    }
    static void DrawAt(const Icon& icon, ImDrawList* dl, ImVec2 pos, float size, ImU32 color) {
        icon.DrawAt(dl, pos, size, color);
    }
    static void Render(const Icon& icon, float size, ImU32 color) {
        icon.Render(size, color);
    }

private:
    Icon(SourceType type, std::string_view path, bool tintable)
        : m_type(type)
        , m_id(None)
        , m_tintable(tintable)
        , m_path(path)
    {}

    SourceType m_type = SourceType::None;
    Id m_id = None;
    bool m_tintable = true;
    std::string m_path;
};

// Type alias for convenience
using IconId = Icon::Id;

} // namespace OmniKit
