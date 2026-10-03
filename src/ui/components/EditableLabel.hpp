#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include "ui/components/Text.hpp"
#include <imgui.h>
#include <string>
#include <functional>

// Параметры редактируемой метки (designated initializers):
//
//     title.Render(value, {.widthPx = zone.Width(), .defaultValue = [] { return std::string("Session"); }});
struct EditableLabelOptions {
    float                        widthPx      = 0.0f;              // итоговая ширина, px; 0 — по содержимому, < 0 — всё свободное место
    TextRole                     role         = TextRole::Title;   // Title — жирный заголовок, Body — обычный текст
    std::function<std::string()> defaultValue;                     // если задан, кнопка в режиме правки подставляет дефолтное имя
    bool                         editable     = true;              // false — статичный заголовок без карандаша и реакции на клик
};

class EditableLabel {
public:
    EditableLabel(const EditableLabelStyle* customStyle = nullptr);

    // Отрисовка редактируемой метки.
    // Возвращает true, если значение было изменено и зафиксировано (Enter, кнопка или потеря фокуса).
    // Если задан options.defaultValue, клик по кнопке с карандашом в режиме редактирования подставляет дефолтное имя.
    // Если options.editable == false, отображается как статичный заголовок без иконки карандаша и без реакции на клик.
    // Идентичность — сам объект (метка должна жить между кадрами).
    bool Render(std::string& value, const EditableLabelOptions& options = {});

    bool IsEditing() const { return m_editing; }
    void StartEdit(const std::string& currentValue);
    void CancelEdit();
    void CommitEdit(std::string& targetValue);

private:
    // Явно переданный стиль; nullptr — стиль текущей темы (смена темы видна сразу)
    const EditableLabelStyle* m_customStyle = nullptr;
    const EditableLabelStyle& Style() const { return m_customStyle ? *m_customStyle : UiTheme::Get().editableLabel; }
    bool m_editing = false;
    char m_buffer[256] = {};
    std::string m_originalValue;
    bool m_focusRequested = false;
    bool m_justOpened = false;
};
