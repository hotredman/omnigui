#pragma once

#include "ui/components/UiTheme.hpp"

#include <string>

// Параметры окна подтверждения (designated initializers):
//
//     ConfirmModal deleteRun({.title = "Delete run?", .message = "This cannot be undone.",
//                             .confirmLabel = "Delete"});
struct ConfirmModalOptions {
    const char* title        = "";
    const char* message      = "";
    const char* confirmLabel = "Confirm";
    UiVariant   variant      = UiVariant::Danger;   // цвет подтверждающей кнопки
};

// ============================================================================
// Модальное окно подтверждения: заголовок, поясняющий текст, кнопки «Отмена» и
// подтверждающая (цвет — по варианту). Объект хранит собственное состояние
// «открыто», поэтому идентификаторы окну не нужны.
// ============================================================================
class ConfirmModal {
public:
    explicit ConfirmModal(const ConfirmModalOptions& options = {});

    void Open() { m_open = true; }
    // Текст, зависящий от введённых значений: задаётся перед Open
    void SetMessage(std::string message) { m_message = std::move(message); }
    bool IsOpen() const { return m_open; }

    // Раз в кадр; true — оператор подтвердил
    bool Render();

private:
    std::string m_title;
    std::string m_message;
    std::string m_confirmLabel;
    UiVariant m_variant;
    bool m_open = false;
};
