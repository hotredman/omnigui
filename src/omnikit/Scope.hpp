#pragma once

// Base class for RAII scopes (Scope).
//
// Contract:
//   * The constructor opens the scope (ImGui::Begin / BeginGroup / ...),
//     and the destructor closes it — there are no public Begin()/End() calls;
//   * The object lives on the stack within a single frame; copying and moving
//     are deleted (C++17 guarantees copy elision for prvalues, so
//     `auto bar = Toolbar();` and returning scopes from methods work);
//   * `explicit operator bool` indicates whether the scope is open and can be drawn into:
//
//         if (auto bar = Toolbar()) {
//             if (auto left = bar.Left()) { left.Label("Session:"); }
//         }
//
// The destructor of a derived class MUST close the scope even when
// Begin returned false (ImGui::End() must always be called, as required by ImGui).

namespace OmniKit {

class Scope {
public:
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
    Scope(Scope&&) = delete;
    Scope& operator=(Scope&&) = delete;

    // Scope is open and visible: content can be rendered
    explicit operator bool() const noexcept { return m_open; }

protected:
    Scope() = default;
    ~Scope() = default;

    bool m_open = false;
};

} // namespace OmniKit
