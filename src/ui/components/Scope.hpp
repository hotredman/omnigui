#pragma once

// Базовый класс RAII-областей (Scope).
//
// Контракт:
//   * конструктор открывает область (ImGui::Begin / BeginGroup / ...),
//     деструктор её закрывает — публичных Begin()/End() нет;
//   * объект живёт на стеке в рамках одного кадра, копировать и перемещать
//     его нельзя (C++17 гарантирует исключение копии для prvalue, поэтому
//     `auto bar = Toolbar();` и возврат областей из методов работают);
//   * `explicit operator bool` — область открыта и в неё можно рисовать:
//
//         if (auto bar = Toolbar()) {
//             if (auto left = bar.Left()) { left.Label("Session:"); }
//         }
//
// Деструктор производного класса ОБЯЗАН закрывать область даже тогда, когда
// Begin вернул false (ImGui::End() вызывается всегда, этого требует ImGui).
class Scope {
public:
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
    Scope(Scope&&) = delete;
    Scope& operator=(Scope&&) = delete;

    // Область открыта и видима: содержимое можно рисовать
    explicit operator bool() const noexcept { return m_open; }

protected:
    Scope() = default;
    ~Scope() = default;

    bool m_open = false;
};
