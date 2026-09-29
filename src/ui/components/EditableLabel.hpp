#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include <imgui.h>
#include <string>
#include <functional>

class EditableLabel {
public:
    EditableLabel(const EditableLabelStyle* customStyle = nullptr);

    // Отрисовка редактируемой метки.
    // Возвращает true, если значение было изменено и зафиксировано (Enter, кнопка или потеря фокуса).
    // Если передан onDefaultValue, клик по кнопке с карандашом в режиме редактирования подставляет дефолтное имя.
    // Если editable == false, отображается как статичный заголовок без иконки карандаша и без реакции на клик.
    bool Render(const char* id, std::string& value, float width = 0.0f, ImFont* font = nullptr,
                const std::function<std::string()>& onDefaultValue = nullptr, bool editable = true);

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
