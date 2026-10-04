#pragma once

#include <imgui.h>
#include <string>
#include "omnikit/ThemePalette.hpp"

namespace OmniKit {

// Component style colors (col* fields) are not set as constants: they are assembled
// by UiTheme::ApplyPalette from the semantic palette of the current theme (ThemePalette)

// ============================================================================
// 1. Semantic component variants of the design system (Semantic Variants)
// ============================================================================
enum class UiVariant {
    Default,    // Neutral control / regular text
    Primary,    // Primary action / brand accent (blue)
    Secondary,  // Secondary neutral (gray-blue)
    Success,    // Success, normal state, START (green)
    Warning,    // Warning, attention (amber / orange)
    Danger,     // Error, fault, STOP (red)
    Info        // Informational / cyan accent
};

// ============================================================================
// 1.1. Standard sizing scale for interactive elements (T-Shirt Scale)
// ============================================================================
enum class UiSize {
    Large,   // 48px — Control panel / Touch (START/STOP/Traverse)
    Medium,  // 36px — Forms, cards, dialog windows (Default)
    Small,   // 30px — Toolbars, filters, search bar
    Mini     // 26px — Table rows, inline actions
};

// Width/height representing "all available space" (value of width/height options field).
// 0 — size to content / auto, Fill — stretch to container edge.
inline constexpr float Fill = -1.0f;

// Control metrics for a specific size with automatic DPI scaling
struct ControlMetrics {
    float height    = 36.0f; // Frame height (already scaled by scale factor)
    float paddingX  = 12.0f; // Horizontal padding
    float paddingY  = 6.0f;  // Vertical padding
    float rounding  = 6.0f;  // Border corner radius
    float iconSize  = 16.0f; // Vector icon size
    ImFont* font    = nullptr;// Recommended font
    float fontSize  = 18.0f; // Base font size in pt
};

// Style descriptor for a semantic variant (background, text, and border colors)
struct SemanticStyle {
    ImU32  colText;     // Text color (for labels, headers, icons)
    ImU32  colBtnText;  // Text color on filled button (typically white)
    ImU32  colBorder;   // Border color (for InputField, Tag, Card)
    ImVec4 colBg;       // Button / badge background
    ImVec4 colBgHover;  // Button background on hover
    ImVec4 colBgActive; // Button background when pressed
};

// ============================================================================
// 2. Component Style Descriptors (Component Styles)
// ============================================================================

// Tab bar style (TabBar)
struct TabBarStyle {
    ImFont* font           = nullptr; // Tab font
    float fontSize         = 20.0f;   // Base tab font size (20pt Medium)
    float itemSpacing      = 6.0f;    // Base horizontal spacing between tabs
    float separatorGapTop  = 4.0f;    // Gap from bottom of tabs to separator line
    float separatorGapBottom = 6.0f;  // Gap from separator line to content
    float cornerRadius     = 6.0f;    // Active tab corner radius
    float horizontalPadding = 14.0f;  // Horizontal padding inside tab

    ImU32 colActiveBg      = 0;
    ImU32 colActiveText    = 0;
    ImU32 colInactiveText  = 0;
    ImU32 colHoverBg       = 0;
    ImU32 colHoverText     = 0;
    ImU32 colSeparator     = 0;
};

// Card style (Card)
struct CardStyle {
    ImFont* titleFont      = nullptr; // Title font
    float titleFontSize    = 22.0f;   // Base title font size (22pt Bold)
    float titleSpacing     = 10.0f;   // Spacing below title to fields
    float paddingX         = 16.0f;   // Horizontal internal padding
    float paddingY         = 14.0f;   // Vertical internal padding
    float rowSpacing       = 8.0f;    // Spacing between field rows
    float columnSpacing    = 12.0f;   // Column spacing in 12-column grid
    float cornerRadius     = 8.0f;    // Card corner radius
    float borderSize       = 1.0f;    // Border thickness

    ImU32 colBg            = 0;
    ImU32 colBorder        = 0;
    ImU32 colTitle         = 0;
};

// Input field style (InputField)
struct InputFieldStyle {
    ImFont* labelFont      = nullptr; // Label font
    float labelFontSize    = 18.0f;   // Base label font size (18pt Regular)
    float labelSpacing     = 4.0f;    // Gap between label and input frame

    ImFont* valueFont      = nullptr; // Input value font
    float valueFontSize    = 20.0f;   // Base value font size (20pt Medium)

    float framePaddingX    = 10.0f;   // Internal horizontal padding
    float framePaddingY    = 6.0f;    // Internal vertical padding
    float frameRounding    = 6.0f;    // Input field corner radius
    float borderSize       = 1.0f;    // Border thickness

    ImFont* unitFont       = nullptr; // Measurement unit font
    float unitFontSize     = 18.0f;   // Base unit font size (18pt Regular)

    // Suggestion dropdown below field (TextSuggest)
    float suggestPaddingY   = 4.0f;    // Vertical list padding
    float suggestHeadHeight = 26.0f;   // Suggestion list header row height
    float suggestGap        = 2.0f;    // Gap between input field and list

    ImU32 colLabel         = 0;
    ImU32 colBg            = 0;
    ImU32 colBorder        = 0;
    ImU32 colText          = 0;
    ImU32 colUnit          = 0;
};

// Dropdown style (Combo / Select)
struct ComboStyle {
    ImFont* labelFont          = nullptr; // Label font
    float labelFontSize        = 18.0f;   // Base label font size (18pt Regular)
    float labelSpacing         = 4.0f;    // Gap between label and field frame

    ImFont* font               = nullptr; // Selected value font
    float fontSize             = 20.0f;   // Base value font size (20pt Medium)

    float framePaddingX        = 10.0f;   // Internal horizontal padding
    float framePaddingY        = 6.0f;    // Internal vertical padding
    float frameRounding        = 6.0f;    // Field corner radius
    float borderSize           = 1.0f;    // Border thickness

    // Arrow chevron
    float arrowWidth           = 7.0f;    // Base arrow width
    float arrowHeight          = 4.5f;    // Base arrow height
    float arrowPaddingRight    = 12.0f;   // Arrow padding from right edge

    // Dropdown list (Popup)
    float popupRounding        = 6.0f;    // Popup window corner radius
    float popupBorderSize      = 1.0f;    // Popup window border thickness

    // Color palette
    ImU32 colLabel             = 0;
    ImU32 colBg                = 0;
    ImU32 colBgHovered         = 0;
    ImU32 colBgActive          = 0;
    ImU32 colBorder            = 0;
    ImU32 colBorderHovered     = 0;
    ImU32 colText              = 0;
    ImU32 colArrow             = 0; // Chevron
    ImU32 colArrowHovered      = 0; // Brighter on hover
    ImU32 colPopupBg           = 0;
    ImU32 colPopupBorder       = 0;
};

// Toggle switch style (Toggle)
struct ToggleStyle {
    float switchWidth          = 42.0f;   // Oval toggle switch width
    float switchHeight         = 22.0f;   // Oval toggle switch height
    float knobPadding          = 2.0f;    // Knob clearance from toggle edges
    float minHeight            = 42.0f;   // Minimum height of full control row
    float labelSpacing         = 10.0f;   // Spacing between switch and text
    float textLineSpacing      = 2.0f;    // Spacing between label and sublabel

    ImFont* fontLabel          = nullptr; // Main label font
    ImFont* fontSublabel       = nullptr; // Sublabel font
    float fontSizeLabel        = 20.0f;   // Main label font size (20pt Medium)
    float fontSizeSublabel     = 18.0f;   // Sublabel font size (18pt Regular)

    // Colors
    ImU32 colTrackOff          = 0;    // Inactive track background
    ImU32 colTrackOffHover     = 0;    // Inactive track background on hover
    ImU32 colTrackOffBorder    = 0;    // Inactive track border
    ImU32 colTrackOn           = 0;  // Active track background
    ImU32 colTrackOnHover      = 0;  // Active track background on hover
    ImU32 colTrackOnBorder     = 0;  // Active track border
    ImU32 colKnobOff           = 0; // Inactive knob
    ImU32 colKnobOn            = 0; // Active knob

    ImU32 colTextMain          = 0; // Main label text
    ImU32 colTextSub           = 0; // Sublabel / description text
};

// Sidebar navigation style (Sidebar)
struct SidebarStyle {
    float width            = 230.0f;  // Base sidebar width
    float itemHeight = 40.0f;         // Base item height
    float itemSpacing = 2.0f;         // Spacing between items
    float cornerRadius     = 7.0f;    // Item pill corner radius
    float activeBarWidth   = 4.0f;    // Width of blue vertical active indicator bar
    float iconSize         = 20.0f;   // Vector icon size

    ImFont* font           = nullptr; // Regular font
    ImFont* activeFont     = nullptr; // Active font
    float fontSize         = 20.0f;   // Base sidebar font size (20pt Medium)

    ImU32 colBg            = 0;
    ImU32 colActiveBg      = 0;
    ImU32 colHoverBg       = 0;
    ImU32 colActiveText    = 0;
    ImU32 colInactiveText  = 0;
    ImU32 colHoverText     = 0;
    ImU32 colActiveBar     = 0;
    ImU32 colSeparator     = 0;
    ImU32 colScrollRegionBg = 0; // Scrollable region background (test run list)
};

// Indicator widget style (Indicator)
struct IndicatorStyle {
    bool autoWidth         = true;    // Automatic width sized to content
    float minWidth         = 110.0f;  // Minimum badge width
    float width            = 180.0f;  // Fixed/base width (if autoWidth == false)
    int defaultDigits      = 5;       // Default number of digits (e.g., "99.99")

    float height           = 56.0f;   // Base badge height
    float cornerRadius     = 6.0f;    // Corner radius
    float padX             = 10.0f;   // Internal padding X
    float padY             = 3.0f;    // Internal padding Y

    ImFont* titleFont      = nullptr; // Title font
    float titleFontSize    = 16.0f;

    ImFont* valueFont      = nullptr; // Value font
    float valueFontSize    = 40.0f;

    ImFont* unitFont       = nullptr; // Unit font
    float unitFontSize     = 20.0f;

    // Tare square button styles (Tare button)
    float tareBtnSize        = 26.0f;   // Square button side length
    float tareBtnRadius      = 4.0f;    // Button corner radius
    std::string tareLabel    = "";      // Button label (if empty, draws vector tare reticle target)
    ImU32 colTareBtn         = 0;       // Button background
    ImU32 colTareBtnHover    = 0;      // Background on hover
    ImU32 colTareBtnActive   = 0;       // Background when pressed
    ImU32 colTareBorder      = 0;      // Button border
    ImU32 colTareBorderHover = 0;     // Border on hover
    ImU32 colTareIcon        = 0;    // Symbol/icon
    ImU32 colTareIconHover   = 0;    // Symbol/icon on hover

    ImU32 colBg            = 0;
    ImU32 colBorder        = 0;
    ImU32 colTitle         = 0;
    ImU32 colValue         = 0;
    ImU32 colUnit          = 0;
};

// Value readout banner / display style (ValueDisplay)
struct ValueDisplayStyle {
    ImFont* valueFont          = nullptr; // Large value font
    float valueFontSize        = 38.0f;   // Base large value font size (>= 60px)
    float valueCompactFontSize = 24.0f;   // Compact value font size (< 60px)

    ImFont* unitFont           = nullptr; // Measurement unit font
    float unitFontSize         = 18.0f;   // Base unit font size (>= 60px)
    float unitCompactFontSize  = 16.0f;   // Compact unit font size (< 60px)

    float cornerRadius         = 8.0f;    // Background corner radius
    float borderSize           = 1.0f;    // Border thickness

    ImU32 colBg                = 0; // Dark inset background
    ImU32 colBorder            = 0; // Border
    ImU32 colValue             = 0; // Vibrant value text
    ImU32 colUnit              = 0; // Muted unit text
};

// Application header style (Header)
struct HeaderStyle {
    float height            = 80.0f;   // Base header height
    float paddingX          = 16.0f;   // Horizontal internal padding
    float paddingY          = 12.0f;   // Vertical internal padding
    float zoneSpacing       = 10.0f;   // Spacing between zones (Left, Center, Right)
    float separatorSize     = 1.0f;    // Bottom separator line thickness
    float defaultLeftWidth  = 310.0f;  // Initial left zone width before first measurement
    float defaultRightWidth = 84.0f;   // Initial right zone width before first measurement

    ImU32 colBg             = 0; // Header window background
    ImU32 colSeparator      = 0; // Separator
};

// Bottom status bar style (StatusBar)
struct StatusBarStyle {
    float height            = 30.0f;   // Base bar height
    float paddingX          = 12.0f;   // Horizontal internal padding
    float itemSpacing       = 12.0f;   // Spacing between items and separator
    float separatorSize     = 1.0f;    // Top line and separator thickness

    ImU32 colBg             = 0;    // Bar background (matches header)
    ImU32 colTopLine        = 0;    // Top separator line
    ImU32 colSeparator      = 0;    // Vertical item separator
    ImU32 colText           = 0; // Regular (secondary) text
};

// Device / connection status widget style (DeviceStatus)
struct DeviceStatusStyle {
    float width               = 220.0f;  // Base badge width
    float height              = 56.0f;   // Base height (aligned with Indicator::height)
    float cornerRadius        = 6.0f;    // Frame corner radius
    float ledRadius           = 5.0f;    // LED indicator radius
    float borderSize          = 1.0f;    // Border thickness
    float actionBtnSize       = 36.0f;   // Action button [P] size (increased from 28.0f)

    ImFont* titleFont         = nullptr; // Device name font (16.0f)
    float titleFontSize       = 16.0f;
    ImFont* statusFont        = nullptr; // Status font (20.0f)
    float statusFontSize      = 20.0f;

    ImU32 colBg               = 0; // Badge background
    ImU32 colBorder           = 0; // Badge border
    ImU32 colTitle            = 0; // Device name color
    ImU32 colStatusText       = 0; // Status text color

    // Semantic LED colors per IEC/SCADA standards:
    ImU32 colLedDisconnected  = 0; // Disconnected / powered off
    ImU32 colLedConnecting    = 0;  // Yellow — connecting / handshake
    ImU32 colLedIdle          = 0;   // Green — ready, waiting for command
    ImU32 colLedRunning       = 0;  // Blue — active process / running
    ImU32 colLedPaused        = 0;   // Amber — paused / hold
    ImU32 colLedFault         = 0;   // Red — fault / alarm
};

// Horizontal carousel container style (Carousel)
struct CarouselStyle {
    float arrowWidth        = 20.0f;   // Compact width for < and > arrow buttons
    float itemSpacing       = 8.0f;    // Spacing between items in carousel
    float scrollSpeed       = 15.0f;   // Smooth scroll interpolation speed (Lerp)
    float scrollStep        = 180.0f;  // Base scroll step when clicking arrow buttons
    float paddingY          = 0.0f;    // Additional vertical clearance
    bool hideDisabledArrows = true;    // Hide inactive arrow button (at start or end of feed)

    ImU32 colBg             = 0; // Scroll track background
    ImU32 colArrowBg        = 0; // Soft chip background for arrows
    ImU32 colArrowText      = 0; // Arrow icon / text color
};

// Context menu style (ContextMenu)
struct ContextMenuStyle {
    // Typography (enlarged readable fonts)
    ImFont* headerFont       = nullptr; // Section header font (Roboto-Bold)
    float headerFontSize     = 18.0f;   // Large accent header (18px Bold)

    ImFont* itemFont         = nullptr; // Menu item font (Roboto-Medium)
    float itemFontSize       = 20.0f;   // Large readable item font (20px Medium)

    // Geometry
    float cornerRadius       = 10.0f;   // Popup window corner radius
    float itemRounding       = 5.0f;    // Item hover highlight corner radius
    float itemHeight         = 36.0f;   // Menu item row height
    float checkColWidth      = 28.0f;   // Checkmark column width
    float minWidth           = 210.0f;  // Minimum menu width
    float windowPaddingX     = 14.0f;   // Popup horizontal padding
    float windowPaddingY     = 12.0f;   // Popup vertical padding
    float itemSpacingY       = 4.0f;    // Spacing between items

    // Color palette
    ImU32 colBg              = 0;       // Popup window background
    ImU32 colBorder          = 0;       // Window border
    ImU32 colHeader          = 0;    // Header color (muted caps)
    ImU32 colText            = 0;    // Regular item text
    ImU32 colTextActive      = 0;     // Active item text
    ImU32 colHoverBg         = 0;      // Background on hover
    ImU32 colActiveBg        = 0;       // Background on click
    ImU32 colCheckmark       = 0;     // Checkmark color
    ImU32 colSeparator       = 0;       // Separator
};

// Preset selector style with integrated readout (PresetGrid)
struct PresetGridStyle {
    // Geometry and layout
    float displaySpacing      = 0.0f;   // Additional spacing from display to button matrix (default 0, uses unified rowSpacing)
    float displayRounding     = 8.0f;   // Readout corner radius (harmonizes with Card)

    float btnRounding         = 6.0f;   // Preset button corner radius
    float colSpacing          = 6.0f;   // Horizontal spacing between buttons in a row
    float rowSpacing          = 6.0f;   // Vertical spacing between button rows

    // Typography
    ImFont* displayValFont    = nullptr;// Large display digit font
    float displayValFontSize  = 24.0f;  // Balanced accent font size for display value
    ImFont* displayUnitFont   = nullptr;// Display measurement unit font
    float displayUnitFontSize = 16.0f;

    ImFont* btnFont           = nullptr;// Font for preset buttons
    float btnFontSize = 20.0f;          // Crisp, neat digit font for presets (16pt Medium)

    // Readout display color palette
    ImU32 colDisplayBg        = 0;   // Dark display background
    ImU32 colDisplayBorder    = 0;   // Display border
    ImU32 colDisplayValue     = 0;// Vibrant value digits
    ImU32 colDisplayUnit      = 0;// Muted unit color

    // Preset buttons color palette
    ImU32 colBtnBg            = 0;   // Normal button background
    ImU32 colBtnBorder        = 0;   // Normal button border
    ImU32 colBtnText          = 0;// Normal button text

    ImU32 colBtnHoverBg       = 0;   // Button background on hover
    ImU32 colBtnHoverBorder   = 0;  // Border on hover
    ImU32 colBtnHoverText     = 0;// Text on hover

    ImU32 colBtnActiveBg      = 0; // Selected button background
    ImU32 colBtnActiveBorder  = 0; // Selected button border
    ImU32 colBtnActiveText    = 0;// Selected button text
};

// Side panel style (SidePanel)
struct SidePanelStyle {
    float splitterWidth     = 6.0f;                       // Interactive splitter width for mouse grabbing
    float splitterLineWidth = 1.0f;                       // Separator line thickness
    ImU32 colBg             = 0;  // Panel background (transparent — no background)
    ImU32 colBorder         = 0;       // Panel border (disabled by default)
    float borderSize        = 0.0f;
    ImVec2 padding          = ImVec2(0.0f, 0.0f);         // Internal panel padding

    ImU32 colSplitter       = 0;  // Splitter line at rest
    ImU32 colSplitterHover  = 0;// Highlight on hover
    ImU32 colSplitterActive = 0;// Highlight when dragging (Accent Blue)
};

// Toolbar style (Toolbar)
struct ToolbarStyle {
    float height            = 46.0f;                       // Base toolbar height
    float paddingX          = 12.0f;                       // Horizontal padding
    float paddingY          = 7.0f;                        // Vertical padding (centers 32px elements: (46-32)/2 = 7)
    float zoneSpacing       = 10.0f;                       // Spacing between zones (Left, Center, Right)
    float defaultLeftWidth  = 200.0f;                      // Initial left zone width
    float defaultRightWidth = 250.0f;                      // Initial right zone width
    float separatorSize     = 1.0f;                        // Bottom separator line thickness

    float ContentHeight() const { return height - paddingY * 2.0f; }

    ImU32 colBg             = 0;   // Panel background
    ImU32 colSeparator      = 0;   // Bottom separator line
};

// Editable text label style (EditableLabel)
struct EditableLabelStyle {
    float boxHeight         = 32.0f;                       // Base button and input field height
    float iconSize          = 16.0f;                       // Pencil icon size
    float spacing           = 8.0f;                        // Spacing between icon and text
    float frameRounding     = 6.0f;                        // Input field border rounding
    float minInputWidth     = 200.0f;                      // Minimum input field width
    float paddingX          = 10.0f;                       // Internal input field horizontal padding

    ImU32 colIcon           = 0; // Icon in normal state
    ImU32 colIconHover      = 0; // Icon on hover
    ImU32 colIconActive     = 0;  // Icon in editing mode
    ImU32 colBtnBg          = 0;    // Pencil button background
    ImU32 colBtnBorder      = 0;    // Pencil button border
    ImU32 colInputBg        = 0;    // Input field background
    ImU32 colInputBorder    = 0;  // Input field border (accent)
};

// Toolbar button style (ToolButton)
struct ToolButtonStyle {
    float size              = 28.0f;                       // Base side length of square button
    float cornerRadius      = 6.0f;                        // Border corner radius
    float borderSize        = 1.0f;                        // Border thickness
    float iconScale         = 0.55f;                       // Icon scale within button

    ImFont* font            = nullptr;                     // Font for text glyphs
    float fontSize          = 16.0f;

    ImU32 colBg             = 0;    // Button background
    ImU32 colBgHover        = 0;    // Background on hover
    ImU32 colBgActive       = 0;    // Background on click
    ImU32 colBorder         = 0;    // Thin border
    ImU32 colBorderHover    = 0;   // Border on hover
    ImU32 colIcon           = 0;  // Icon / text color
    ImU32 colIconHover      = 0;  // Icon color on hover

    ImU32 colBgSelected     = 0;  // Selected toggle button background
    ImU32 colBorderSelected = 0;  // Selected toggle button border
};

// Table style (Table)
struct TableStyle {
    // 1. Table header (Header)
    ImU32 colHeaderBg          = 0;   // Header background
    ImU32 colHeaderHovered     = 0;   // Hover highlight on sortable column header
    ImU32 colHeaderActive      = 0;   // Column header when pressed
    ImU32 colHeaderText        = 0;// Secondary header text color
    float headerHeight         = 40.0f;                       // Base header height
    float headerFontSize       = 18.0f;                       // Base header font size (18pt Medium)
    float cellFontSize         = 20.0f;                       // Base cell font size (20pt Medium)
    float subFontSize          = 18.0f;                       // Base sublabel font size (18pt Regular)

    ImU32 colBodyBg            = 0;   // Table body and empty state background

    // 2. Data rows (Rows)
    ImU32 colRowBg             = 0;   // Primary row background
    ImU32 colRowBgAlt          = 0;   // Alternating zebra stripe
    ImU32 colRowHovered        = 0;   // Highlight on cursor hover
    ImU32 colRowSelected       = 0;  // Selected row
    float rowHeight            = 64.0f;                       // Base row height

    // 3. Grid lines and cell borders (Borders & Dividers)
    ImU32 colBorderOuter       = 0;   // Outer border
    ImU32 colBorderInner       = 0;   // Inner fine dividers
    float cellPaddingX         = 12.0f;                       // Cell horizontal padding
    float cellPaddingY         = 8.0f;                        // Cell vertical padding

    // 4. Empty state (Empty state)
    ImU32 colEmptyIcon         = 0;
    ImU32 colEmptyText         = 0;
};

// In-page search and filter bar style (FilterBar)
struct FilterBarStyle {
    float height            = 34.0f;                       // Base filter bar height
    float itemSpacing       = 8.0f;                        // Spacing between similar buttons in a group
    float groupSpacing      = 14.0f;                       // Spacing between distinct functional groups (actions -> filters)
    float labelSpacing      = 6.0f;                        // Spacing from label to its control (Label -> Combo/Input)
    float stretchGap        = 16.0f;                       // Gap between expanding search input and right group
    float minSearchWidth    = 180.0f;                      // Minimum search input width
    ImU32 colLabel          = 0;// Label color (muted)
};

// Scrollbar style (Scrollbar)
struct ScrollbarStyle {
    float size              = 8.0f;                        // Clean 8px thickness (replaces default 16px)
    float cornerRadius      = 4.0f;                        // Fully rounded thumb pill/capsule
    float minGrabSize       = 20.0f;                       // Minimum thumb size for long lists

    ImU32 colBg             = 0;        // Transparent track (does not cut across table/content background)
    ImU32 colBgHovered      = 0;   // Subtle track highlight on hover
    ImU32 colGrab           = 0;  // Scrollbar thumb
    ImU32 colGrabHovered    = 0; // Brighter on mouse hover
    ImU32 colGrabActive     = 0; // Thumb when dragging
};

// Vertical item list style (List / ListView)
struct ListStyle {
    // Typography (strictly even font sizes)
    ImFont* headerFont         = nullptr; // List section header font
    float headerFontSize       = 16.0f;   // Section header (16pt Bold)
    ImFont* itemFont           = nullptr; // Item name font
    float itemFontSize         = 18.0f;   // Item font (18pt Medium)
    ImFont* subFont            = nullptr; // Label / details font
    float subFontSize          = 14.0f;   // Details font (14pt Regular)

    // Geometry
    float itemHeight           = 38.0f;   // Base row height
    float itemSpacing          = 2.0f;    // Spacing between rows
    float cornerRadius         = 6.0f;    // Row corner radius on hover/selection
    float paddingX             = 10.0f;   // Internal horizontal padding
    float activeBarWidth       = 3.0f;    // Accent vertical indicator bar width
    float iconSize             = 14.0f;   // Vector icon / status dot size

    // Palette
    ImU32 colBg                = 0;         // List background (transparent)
    ImU32 colItemBg            = 0;         // Normal item background at rest
    ImU32 colItemHoverBg       = 0;    // Item background on hover
    ImU32 colItemSelectedBg    = 0;    // Selected item background
    ImU32 colActiveBar         = 0;  // Accent indicator bar

    ImU32 colHeader            = 0; // List header text
    ImU32 colText              = 0; // Normal item text
    ImU32 colTextSelected      = 0; // Selected item text
    ImU32 colTextMuted         = 0; // Secondary text / subtitle
    ImU32 colBorder            = 0;    // List border / divider
};

// Main content area style (ContentArea)
// The workspace is the application canvas: it does not draw its own background,
// resting on top of the frame cleared with UiTheme::colWindowBg
struct ContentAreaStyle {
    float paddingX             = 16.0f;                        // Internal horizontal padding
    float paddingY             = 10.0f;                        // Internal vertical padding
    bool enableScrollX         = false;                        // Horizontal scrolling
    bool enableScrollY         = false;                        // Vertical scrolling
};

// Interactive chip tag style (Tag)
struct TagStyle {
    ImFont* font               = nullptr; // Tag font
    float fontSize             = 16.0f;   // Base font size (16pt Medium)

    float paddingX             = 6.0f;    // Internal horizontal padding
    float paddingY             = 1.5f;    // Internal vertical padding
    float spacingX             = 5.0f;    // Horizontal spacing between tags
    float spacingY             = 2.5f;    // Vertical spacing between wrapped tag lines
    float cornerRadius         = 4.0f;    // Chip corner radius
    float borderSize           = 1.0f;    // Border thickness

    ImU32 colBg                = 0;    // Background at rest
    ImU32 colBgHover           = 0;    // Background on hover
    ImU32 colBgActive          = 0;    // Background when pressed
    ImU32 colBorder            = 0;    // Border at rest
    ImU32 colBorderHover       = 0;  // Border on hover
    ImU32 colPrefix            = 0; // '#' prefix symbol color
    ImU32 colPrefixHover       = 0;
    ImU32 colText              = 0; // Tag text color
    ImU32 colTextHover         = 0;
};

// Interactive card row style (ItemRow)
struct ItemRowStyle {
    // Typography (strictly even font sizes)
    ImFont* titleFont          = nullptr; // Title font
    float titleFontSize        = 20.0f;   // 20pt Bold
    ImFont* symbolFont         = nullptr; // Symbol font
    float symbolFontSize       = 18.0f;   // 18pt Bold
    ImFont* unitFont           = nullptr; // Unit font
    float unitFontSize         = 18.0f;   // 18pt Medium
    ImFont* descriptionFont    = nullptr; // Description font
    float descriptionFontSize  = 16.0f;   // 16pt Regular
    ImFont* formulaFont        = nullptr; // Formula font
    float formulaFontSize      = 16.0f;   // 16pt Medium
    float badgeFontSize        = 14.0f;   // 14pt Medium

    // Geometry and internal padding (compact, without excessive whitespace)
    float paddingX             = 10.0f;   // Horizontal padding inside card
    float paddingYTop = 4.0f;             // Top padding inside card
    float paddingYBottom = 4.0f;          // Bottom padding inside card
    float spacingTitleDesc     = 1.0f;    // Spacing between title and description
    float spacingDescTags      = 2.0f;    // Spacing between description and tags
    float itemSpacingY = 8.0f;            // Spacing between cards in list
    float cornerRadius         = 6.0f;    // Card corner radius
    float borderSize           = 1.0f;    // Card border thickness

    // Right-side action button
    float actionBtnWidth       = 34.0f;   // Action button width
    float actionBtnHeight      = 34.0f;   // Action button height
    float actionSpacing        = 5.0f;    // Spacing between action buttons

    // Palette
    ImU32 colBg                = 0;    // Card background
    ImU32 colBgHover           = 0;    // Background on hover
    ImU32 colBorder            = 0;    // Card border
    ImU32 colBorderHover       = 0;   // Border on hover
    ImU32 colTextDescription   = 0; // Description text color
    ImU32 colTextFormula       = 0; // Formula text color
};

// Color triad for semantic badge: tint background, border, variant text
struct BadgeVariantColors {
    ImU32 colBg;       // Badge background color
    ImU32 colBorder;   // Badge border color
    ImU32 colText;     // Text color (high-contrast, easily readable)
};

// Semantic badge style (Badge)
struct BadgeStyle {
    ImFont* font               = nullptr; // Badge font (defaults to fontMedium)
    float fontSize             = 14.0f;   // Base font size (14pt Medium)

    float paddingX             = 6.0f;    // Internal horizontal padding
    float paddingY             = 1.5f;    // Internal vertical padding
    float cornerRadius         = 4.0f;    // Badge corner radius
    float borderSize           = 1.0f;    // Border thickness

    // Colors by UiVariant (Default=0, Primary=1, Secondary=2, Success=3, Warning=4, Danger=5, Info=6)
    BadgeVariantColors colors[7] = {};

    const BadgeVariantColors& GetColors(UiVariant variant) const {
        size_t idx = static_cast<size_t>(variant);
        if (idx < sizeof(colors) / sizeof(colors[0])) {
            return colors[idx];
        }
        return colors[0];
    }
};

// ============================================================================
// 4. Main Design System Class (UiTheme)
// ============================================================================
class UiTheme {
public:
    static UiTheme& Get();

    UiTheme();

    // Disabled state (DisabledScope): everything within a disabled region
    // is dimmed using this opacity. Components render colors through
    // ImGui::GetColorU32(color) so they are dimmed together with ImGui colors;
    // colors passed to PushStyleColor are dimmed by ImGui automatically (do not
    // double-wrap)
    static constexpr float kDisabledAlpha = 0.45f;

    // 1. Scaling
    static constexpr float kMinScale = 0.75f;
    static constexpr float kMaxScale = 2.5f;
    float scale = 1.0f;
    float GetScale() const { return scale; }
    void SetScale(float s);
    float Scale(float baselinePx) const { return baselinePx * scale; }

    // 2. Shell Layout Reference Dimensions
    float headerHeight     = 80.0f;
    float topBarHeight     = 46.0f;
    float sidebarWidth     = 230.0f;

    float HeaderHeight() const { return Scale(header.height); }
    float StatusBarHeight() const { return Scale(statusBar.height); }
    float ToolbarHeight() const { return Scale(toolbar.height); }
    float TopBarHeight() const { return ToolbarHeight(); }
    float SidebarWidth() const { return Scale(sidebarWidth); }

    // 3. Standard Grid Spacing Tokens
    float spacingSmall     = 4.0f;
    float spacingMedium    = 8.0f;
    float spacingLarge     = 16.0f;

    float SpacingSmall() const { return Scale(spacingSmall); }
    float SpacingMedium() const { return Scale(spacingMedium); }
    float SpacingLarge() const { return Scale(spacingLarge); }

    // Compatibility helper delegates
    float CornerRadius() const { return Scale(card.cornerRadius); }
    float IndicatorWidth() const { return Scale(indicator.width); }
    float IndicatorHeight() const { return Scale(indicator.height); }
    float CardRowSpacing() const { return Scale(card.rowSpacing); }
    void SetCardRowSpacing(float s) { card.rowSpacing = s; }
    float FieldLabelSpacing() const { return Scale(input.labelSpacing); }
    void SetFieldLabelSpacing(float s) { input.labelSpacing = s; }

    // 4. Common interface fonts (dynamic FreeType atlas)
    ImFont* fontRegular      = nullptr; // Roboto-Regular: base text, cells, labels
    ImFont* fontMedium       = nullptr; // Roboto-Medium: buttons, tabs, input values
    ImFont* fontBold         = nullptr; // Roboto-Bold: card headers, control panel, readout displays

    // Aliases for backward compatibility
    ImFont* defaultFont      = nullptr; // -> fontRegular
    ImFont* smallFont        = nullptr; // -> fontRegular
    ImFont* buttonFont       = nullptr; // -> fontMedium
    ImFont* projectTitleFont = nullptr; // -> fontBold
    ImFont* displayBigFont   = nullptr; // -> fontBold

    // Helper methods for ImGui 1.92+ dynamic font stack
    void PushFont(ImFont* font, float basePt) const {
        ImFont* f = font ? font : (fontRegular ? fontRegular : defaultFont);
        ImGui::PushFont(f, Scale(basePt));
    }

    void PushFont(float basePt) const {
        PushFont(nullptr, basePt);
    }

    void PopFont() const {
        ImGui::PopFont();
    }

    // 5. Theme: color semantic palette for current mode
    ThemeMode mode = ThemeMode::Dark;
    ThemePalette palette = ThemePalette::Dark();

    // Theme switching: palette -> component styles -> ImGuiStyle
    void SetMode(ThemeMode m);

    // 6. Component-specific styles
    TabBarStyle tab;
    CardStyle card;
    InputFieldStyle input;
    ComboStyle combo;
    ToggleStyle toggle;
    SidebarStyle sidebar;
    IndicatorStyle indicator;
    ValueDisplayStyle valueDisplay;
    HeaderStyle header;
    StatusBarStyle statusBar;
    DeviceStatusStyle deviceStatus;
    CarouselStyle carousel;
    ContextMenuStyle contextMenu;
    PresetGridStyle presetGrid;
    SidePanelStyle sidePanel;
    ToolbarStyle toolbar;
    EditableLabelStyle editableLabel;
    ToolButtonStyle toolButton;
    TableStyle table;
    FilterBarStyle filterBar;
    ScrollbarStyle scrollbar;
    ListStyle list;
    ContentAreaStyle contentArea;
    ItemRowStyle itemRow;
    TagStyle tag;
    BadgeStyle badge;

    // Get scaled control metrics for UiSize
    ControlMetrics GetMetrics(UiSize size) const;

    // Size factor for chips and labels (Badge, Tag, Toggle): ratio of selected
    // UiSize font size to Medium. Equal to 1 for Medium; appearance remains unchanged.
    float SizeFactor(UiSize size) const;

    // Get semantic variant style (background, text, and border colors)
    const SemanticStyle& GetVariantStyle(UiVariant variant) const;

    // Load interface and component fonts
    void LoadFonts();

    // Update DPI scaling
    void UpdateScaling(float s) { SetScale(s); }

    // Apply base parameters and colors to ImGuiStyle
    void ApplyToImGui();

private:
    // Colors of all component styles and semantic variants derived from palette
    void ApplyPalette();

    SemanticStyle m_variants[7] = {};
};

} // namespace OmniKit
