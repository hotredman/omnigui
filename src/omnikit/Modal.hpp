#pragma once

#include <algorithm>
#include <string>

#include <imgui.h>

#include "omnikit/Button.hpp"
#include "omnikit/Icon.hpp"
#include "omnikit/Scope.hpp"
#include "omnikit/UiTheme.hpp"

namespace OmniKit {

// Modal content scrolling mode
enum class ModalScroll {
    Auto,  // Scrollbar appears automatically when content overflows
    None   // No outer scrolling: content is fixed (for tables with internal scrolling)
};

// Modal window options (designated initializers):
//
//     if (Modal modal(showSettings, {.title = "Settings", .width = 640})) { ... }
struct ModalOptions {
    const char* title = nullptr;       // Window title text; nullptr for no title
    float width = 720.0f;              // Base px (scaled internally)
    float height = 580.0f;             // Base px; <= 0: auto-height to fit content
    float footerHeight = 0.0f;         // Base px; 0: auto-computed from theme (button height + padding)
    bool cardBackground = true;        // Card background color for the content area
};

// Modal content area options
struct ModalContentOptions {
    ModalScroll scroll = ModalScroll::Auto;
    float footerHeight = 0.0f;         // Base px; 0 uses default footer height
};

// Options for standard "Cancel" / "Done" footer buttons
struct ModalActionsOptions {
    const char* confirmLabel = "Done";
    const char* cancelLabel = "Cancel";
    UiVariant variant = UiVariant::Primary;   // Confirm button style variant
    Icon icon = Icon::Check;                  // Confirm button icon
    bool confirmDisabled = false;
};

// ============================================================================
// Modal dialog window (RAII scope). The "open" state is stored by the application
// (bool): to open the window, simply set it to true; the window resets the
// flag automatically upon closing. The window identity is the address of this flag.
//
//     static bool s_open = false;
//     if (Button("Settings")) s_open = true;
//     if (Modal modal(s_open, {.title = "Settings"}); modal) {
//         if (auto content = modal.Content()) { ... }
//         if (auto footer = modal.Footer()) { if (footer.Actions()) Apply(); }
//     }
//
// Provides RAII management of the popup lifecycle, ImGui style stack,
// as well as fixed HeaderScope, ContentScope, and FooterScope regions.
// ============================================================================
class Modal : public Scope {
public:
    // ------------------------------------------------------------------------
    // 1. HeaderScope: top fixed region (title, fields, tabs)
    // ------------------------------------------------------------------------
    class HeaderScope : public Scope {
    public:
        explicit HeaderScope(Modal* parent) {
            if (parent && *parent) {
                m_open = true;
                m_width = ImGui::GetContentRegionAvail().x;
            }
        }

        ~HeaderScope() {
            if (m_open) {
                ImGui::Spacing();
            }
        }

        float Width() const { return m_width; }

        void Title(const std::string& title) {
            const UiTheme& theme = UiTheme::Get();
            theme.PushFont(theme.fontBold, 18.0f);
            ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "%s", title.c_str());
            theme.PopFont();
        }

        void Subtitle(const std::string& subtitle) {
            const UiTheme& theme = UiTheme::Get();
            theme.PushFont(theme.fontRegular, 13.0f);
            ImGui::TextColored(ImColor(theme.palette.textMuted).Value, "%s", subtitle.c_str());
            theme.PopFont();
        }

        void Separator() {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }

        void Spacing() {
            ImGui::Spacing();
        }

    private:
        float m_width = 0.0f;
    };

    // ------------------------------------------------------------------------
    // 2. ContentScope: central region (scrollable or fixed)
    // ------------------------------------------------------------------------
    class ContentScope : public Scope {
    public:
        ContentScope(Modal* parent, const ModalContentOptions& options) {
            if (!parent || !*parent) return;

            // If window has fixed height (> 0), create a ChildWindow scroll region
            if (parent->m_options.height > 0.0f) {
                const UiTheme& theme = UiTheme::Get();
                float footerH = (options.footerHeight > 0.0f) ? theme.Scale(options.footerHeight)
                                                              : parent->GetDefaultFooterHeight();

                if (parent->m_options.cardBackground) {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, theme.card.colBg);
                    m_colorPushed = true;
                }

                ImGuiWindowFlags flags = ImGuiWindowFlags_None;
                if (options.scroll == ModalScroll::None) {
                    flags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
                }

                m_isChild = true;
                m_open = ImGui::BeginChild("##ModalContent", ImVec2(0.0f, -footerH), false, flags);
                if (m_open) {
                    m_width = ImGui::GetContentRegionAvail().x;
                    m_height = ImGui::GetContentRegionAvail().y;
                }
            } else {
                // Auto-size: content renders directly into the popup window
                m_isChild = false;
                m_open = true;
                m_width = ImGui::GetContentRegionAvail().x;
                m_height = ImGui::GetContentRegionAvail().y;
            }
        }

        ~ContentScope() {
            if (m_isChild) {
                ImGui::EndChild();
            }
            if (m_colorPushed) {
                ImGui::PopStyleColor();
            }
        }

        float Width() const { return m_width; }
        float Height() const { return m_height; }

    private:
        bool m_isChild = false;
        bool m_colorPushed = false;
        float m_width = 0.0f;
        float m_height = 0.0f;
    };

    // ------------------------------------------------------------------------
    // 3. FooterScope: bottom fixed action area
    // ------------------------------------------------------------------------
    class FooterScope : public Scope {
    public:
        explicit FooterScope(Modal* parent) : m_parent(parent) {
            if (m_parent && *m_parent) {
                m_open = true;
                ImGui::Separator();
                ImGui::Spacing();
                m_width = ImGui::GetContentRegionAvail().x;
            }
        }

        float Width() const { return m_width; }

        // Align buttons to the right edge
        void RightAlign(float totalWidth) {
            float avail = ImGui::GetContentRegionAvail().x;
            if (avail > totalWidth) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - totalWidth));
            }
        }

        // Continue on the same line within the footer
        void SameLine(float offsetFromStartX = 0.0f, float spacing = -1.0f) {
            ImGui::SameLine(offsetFromStartX, spacing);
        }

        // Align right group of buttons when an element has already been drawn on the left
        void AlignRight(float totalWidth) {
            ImGui::SameLine();
            float avail = ImGui::GetContentRegionAvail().x;
            if (avail > totalWidth) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - totalWidth));
            }
        }

        // Standard "Cancel" and "Done" buttons: both close the window,
        // returns true if confirmed
        bool Actions(const ModalActionsOptions& options = {}) {
            if (!m_open || !m_parent) return false;

            const UiTheme& theme = UiTheme::Get();
            const float scale = theme.GetScale();
            const float btnW = 120.0f * scale;
            const float spacingX = theme.SpacingSmall();
            const float totalW = btnW * 2.0f + spacingX;

            RightAlign(totalW);

            bool cancelled = Button({.label = options.cancelLabel, .variant = UiVariant::Secondary, .width = btnW / scale});
            ImGui::SameLine(0.0f, spacingX);
            bool confirmed = Button({.label = options.confirmLabel, .variant = options.variant, .icon = options.icon,
                                     .width = btnW / scale, .disabled = options.confirmDisabled});

            if (cancelled) {
                m_parent->Close();
                return false;
            }
            if (confirmed && !options.confirmDisabled) {
                m_parent->Close();
                return true;
            }
            return false;
        }

    private:
        Modal* m_parent = nullptr;
        float m_width = 0.0f;
    };

    // ------------------------------------------------------------------------
    // Modal constructor and destructor (RAII)
    // ------------------------------------------------------------------------
    Modal(bool& isOpen, const ModalOptions& options = {})
        : m_isOpenRef(&isOpen), m_options(options) {
        // Window identity is the address of the "open" flag
        ImGui::PushID(&isOpen);

        if (!*m_isOpenRef) return;

        // Window name: "title text##internal-suffix"; identical across all ImGui calls
        m_windowName = std::string(m_options.title ? m_options.title : "") + kPopupName;

        if (!ImGui::IsPopupOpen(m_windowName.c_str())) {
            ImGui::OpenPopup(m_windowName.c_str());
        }

        const UiTheme& theme = UiTheme::Get();
        const float scale = theme.GetScale();

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        if (m_options.height <= 0.0f) {
            flags |= ImGuiWindowFlags_AlwaysAutoResize;
            ImGui::SetNextWindowSize(ImVec2(theme.Scale(m_options.width), 0.0f), ImGuiCond_Always);
        } else {
            ImGui::SetNextWindowSize(
                ImVec2(theme.Scale(m_options.width), theme.Scale(m_options.height)),
                ImGuiCond_Appearing);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f * scale, 16.0f * scale));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, theme.CornerRadius());
        m_stylesPushed = 2;

        if (ImGui::BeginPopupModal(m_windowName.c_str(), nullptr, flags)) {
            m_open = true;
            m_begun = true;
            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                Close();
            }
        } else {
            // Closed externally (ESC before rendering or focus lost)
            *m_isOpenRef = false;
            m_open = false;
            m_begun = false;
        }
    }

    ~Modal() {
        if (m_begun) {
            ImGui::EndPopup();
            m_begun = false;
        }
        if (m_stylesPushed > 0) {
            ImGui::PopStyleVar(m_stylesPushed);
            m_stylesPushed = 0;
        }
        ImGui::PopID();
    }

    HeaderScope Header() { return HeaderScope(this); }

    ContentScope Content(const ModalContentOptions& options = {}) {
        return ContentScope(this, options);
    }

    FooterScope Footer() { return FooterScope(this); }

    // Close window (e.g. from a custom footer button)
    void Close() {
        if (m_isOpenRef) *m_isOpenRef = false;
        m_open = false;
        ImGui::CloseCurrentPopup();
    }

    float GetDefaultFooterHeight() const {
        const UiTheme& theme = UiTheme::Get();
        return theme.GetMetrics(UiSize::Medium).height + theme.Scale(32.0f);
    }

private:
    // Internal window name suffix: not displayed, part of ImGui ID
    static constexpr const char* kPopupName = "##modal";

    bool* m_isOpenRef = nullptr;
    ModalOptions m_options;
    std::string m_windowName;
    bool m_begun = false;
    int m_stylesPushed = 0;
};

} // namespace OmniKit
