#pragma once

#include <algorithm>
#include <string>

#include <imgui.h>

#include "ui/components/Button.hpp"
#include "ui/components/Icon.hpp"
#include "ui/components/UiTheme.hpp"

// Режим прокрутки контентной области модального окна
enum class ModalScroll {
    Auto,  // Скроллбар появляется автоматически при переполнении контента
    None   // Без внешнего скролла: контент фиксирован (для таблиц с собственным скроллом)
};

// ============================================================================
// Компонент первого класса: Модальное диалоговое окно (Modal)
// Предоставляет RAII-управление жизненным циклом попапа, стеком стилей ImGui,
// а также фиксированными областями HeaderScope, ContentScope и FooterScope.
// ============================================================================
class Modal {
public:
    struct Config {
        float width = 720.0f;
        float height = 580.0f;       // <= 0: автоматическая высота по содержимому
        float footerHeight = 0.0f;   // 0: авто-расчет по теме (высота кнопок + отступы)
        bool cardBackground = true;  // Подложка цвета карточки для контентной области

        constexpr Config(float w = 720.0f, float h = 580.0f, float footerH = 0.0f, bool cardBg = true)
            : width(w), height(h), footerHeight(footerH), cardBackground(cardBg) {}
    };

    // ------------------------------------------------------------------------
    // 1. HeaderScope: верхняя фиксированная область (заголовок, поля, табы)
    // ------------------------------------------------------------------------
    class HeaderScope {
    public:
        explicit HeaderScope(Modal* parent) : m_parent(parent) {
            if (m_parent && m_parent->IsOpen()) {
                m_open = true;
                m_width = ImGui::GetContentRegionAvail().x;
            }
        }

        ~HeaderScope() {
            if (m_open) {
                ImGui::Spacing();
            }
        }

        HeaderScope(const HeaderScope&) = delete;
        HeaderScope& operator=(const HeaderScope&) = delete;

        HeaderScope(HeaderScope&& other) noexcept
            : m_parent(other.m_parent), m_open(other.m_open), m_width(other.m_width) {
            other.m_parent = nullptr;
            other.m_open = false;
        }

        explicit operator bool() const { return m_open; }
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
        Modal* m_parent = nullptr;
        bool m_open = false;
        float m_width = 0.0f;
    };

    // ------------------------------------------------------------------------
    // 2. ContentScope: центральная область (скроллируемая или фиксированная)
    // ------------------------------------------------------------------------
    class ContentScope {
    public:
        ContentScope(Modal* parent, ModalScroll scrollMode = ModalScroll::Auto,
                     float customFooterH = 0.0f)
            : m_parent(parent) {
            if (!m_parent || !m_parent->IsOpen()) return;

            // Если окно фиксированной высоты (> 0) — создаём скролл-регион ChildWindow
            if (m_parent->m_config.height > 0.0f) {
                const UiTheme& theme = UiTheme::Get();
                float footerH = (customFooterH > 0.0f) ? theme.Scale(customFooterH)
                                                       : m_parent->GetDefaultFooterHeight();

                if (m_parent->m_config.cardBackground) {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, theme.card.colBg);
                    m_colorPushed = true;
                }

                ImGuiWindowFlags flags = ImGuiWindowFlags_None;
                if (scrollMode == ModalScroll::None) {
                    flags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
                }

                m_isChild = true;
                m_open = ImGui::BeginChild("##ModalContent", ImVec2(0.0f, -footerH), false, flags);
                if (m_open) {
                    m_width = ImGui::GetContentRegionAvail().x;
                    m_height = ImGui::GetContentRegionAvail().y;
                }
            } else {
                // Авто-размер: контент рендерится напрямую в окно попапа
                m_isChild = false;
                m_open = true;
                m_width = ImGui::GetContentRegionAvail().x;
                m_height = ImGui::GetContentRegionAvail().y;
            }
        }

        ~ContentScope() {
            if (m_isChild) {
                ImGui::EndChild();
                m_isChild = false;
            }
            if (m_colorPushed) {
                ImGui::PopStyleColor();
                m_colorPushed = false;
            }
        }

        ContentScope(const ContentScope&) = delete;
        ContentScope& operator=(const ContentScope&) = delete;

        ContentScope(ContentScope&& other) noexcept
            : m_parent(other.m_parent),
              m_open(other.m_open),
              m_isChild(other.m_isChild),
              m_colorPushed(other.m_colorPushed),
              m_width(other.m_width),
              m_height(other.m_height) {
            other.m_parent = nullptr;
            other.m_open = false;
            other.m_isChild = false;
            other.m_colorPushed = false;
        }

        explicit operator bool() const { return m_open; }
        float Width() const { return m_width; }
        float Height() const { return m_height; }

    private:
        Modal* m_parent = nullptr;
        bool m_open = false;
        bool m_isChild = false;
        bool m_colorPushed = false;
        float m_width = 0.0f;
        float m_height = 0.0f;
    };

    // ------------------------------------------------------------------------
    // 3. FooterScope: нижняя фиксированная область действий
    // ------------------------------------------------------------------------
    class FooterScope {
    public:
        explicit FooterScope(Modal* parent) : m_parent(parent) {
            if (m_parent && m_parent->IsOpen()) {
                m_open = true;
                ImGui::Separator();
                ImGui::Spacing();
                m_width = ImGui::GetContentRegionAvail().x;
            }
        }

        ~FooterScope() = default;

        FooterScope(const FooterScope&) = delete;
        FooterScope& operator=(const FooterScope&) = delete;

        FooterScope(FooterScope&& other) noexcept
            : m_parent(other.m_parent), m_open(other.m_open), m_width(other.m_width) {
            other.m_parent = nullptr;
            other.m_open = false;
        }

        explicit operator bool() const { return m_open; }
        float Width() const { return m_width; }

        // Выравнивание кнопок по правому краю
        void RightAlign(float totalWidth) {
            float avail = ImGui::GetContentRegionAvail().x;
            if (avail > totalWidth) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - totalWidth));
            }
        }

        // Переход на ту же строку внутри футера
        void SameLine(float offsetFromStartX = 0.0f, float spacing = -1.0f) {
            ImGui::SameLine(offsetFromStartX, spacing);
        }

        // Выравнивание правой группы кнопок, когда слева уже отрисован элемент
        void AlignRight(float totalWidth) {
            ImGui::SameLine();
            float avail = ImGui::GetContentRegionAvail().x;
            if (avail > totalWidth) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - totalWidth));
            }
        }

        // Стандартные кнопки «Отмена» и «Готово»
        bool Actions(const char* confirmText = "Готово", const char* cancelText = "Отмена",
                     bool confirmEnabled = true, UiVariant confirmVariant = UiVariant::Primary,
                     Icon::Id confirmIcon = Icon::Check) {
            if (!m_open || !m_parent) return false;

            const UiTheme& theme = UiTheme::Get();
            const float scale = theme.GetScale();
            const float btnW = 120.0f * scale;
            const float spacingX = theme.SpacingSmall();
            const float totalW = btnW * 2.0f + spacingX;

            RightAlign(totalW);

            bool cancelled = Button::Secondary(cancelText, Icon::None, UiSize::Medium, btnW / scale);
            ImGui::SameLine(0.0f, spacingX);
            bool confirmed = Button::Render(confirmText, confirmVariant, confirmIcon, UiSize::Medium,
                                            btnW / scale, !confirmEnabled);

            if (cancelled) {
                m_parent->Close();
                return false;
            }
            if (confirmed && confirmEnabled) {
                m_parent->Close();
                return true;
            }
            return false;
        }

    private:
        Modal* m_parent = nullptr;
        bool m_open = false;
        float m_width = 0.0f;
    };

    // ------------------------------------------------------------------------
    // Конструктор и деструктор Modal (RAII)
    // ------------------------------------------------------------------------
    Modal(const char* id, bool& isOpen, float width, float height = 0.0f)
        : Modal(id, isOpen, Config(width, height)) {}

    Modal(const char* id, bool& isOpen, const Config& config = {})
        : m_id(id), m_isOpenRef(&isOpen), m_config(config) {
        if (!*m_isOpenRef) return;

        if (!ImGui::IsPopupOpen(m_id)) {
            ImGui::OpenPopup(m_id);
        }

        const UiTheme& theme = UiTheme::Get();
        const float scale = theme.GetScale();

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        if (m_config.height <= 0.0f) {
            flags |= ImGuiWindowFlags_AlwaysAutoResize;
            ImGui::SetNextWindowSize(ImVec2(theme.Scale(m_config.width), 0.0f), ImGuiCond_Always);
        } else {
            ImGui::SetNextWindowSize(
                ImVec2(theme.Scale(m_config.width), theme.Scale(m_config.height)),
                ImGuiCond_Appearing);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f * scale, 16.0f * scale));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, theme.CornerRadius());
        m_stylesPushed = 2;

        if (ImGui::BeginPopupModal(m_id, nullptr, flags)) {
            m_open = true;
            m_begun = true;
            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                Close();
            }
        } else {
            // Закрыта снаружи (ESC до отрисовки или потеря фокуса)
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
    }

    Modal(const Modal&) = delete;
    Modal& operator=(const Modal&) = delete;

    explicit operator bool() const { return m_open; }
    bool IsOpen() const { return m_open; }

    HeaderScope Header() { return HeaderScope(this); }

    ContentScope Content(ModalScroll scrollMode = ModalScroll::Auto, float customFooterH = 0.0f) {
        return ContentScope(this, scrollMode, customFooterH);
    }

    FooterScope Footer() { return FooterScope(this); }

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
    const char* m_id = nullptr;
    bool* m_isOpenRef = nullptr;
    Config m_config;
    bool m_open = false;
    bool m_begun = false;
    int m_stylesPushed = 0;
};
