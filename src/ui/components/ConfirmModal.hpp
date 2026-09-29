#pragma once

#include "ui/components/UiTheme.hpp"

#include <string>

// ============================================================================
// Модальное окно подтверждения: заголовок, поясняющий текст, кнопки «Отмена» и
// подтверждающая (цвет — по варианту). id — уникальная часть имени окна ImGui.
// ============================================================================
class ConfirmModal {
public:
    ConfirmModal(std::string id, std::string title, std::string text, std::string confirmLabel,
                 UiVariant variant = UiVariant::Danger);

    void Open() { m_open = true; }
    bool IsOpen() const { return m_open; }

    // Раз в кадр; true — оператор подтвердил
    bool Render();

private:
    std::string m_windowName;  // «заголовок##id»
    std::string m_title;
    std::string m_text;
    std::string m_confirmLabel;
    UiVariant m_variant;
    bool m_open = false;
};
