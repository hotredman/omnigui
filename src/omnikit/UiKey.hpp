#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <source_location>
#include <cstdint>
#include <string_view>

namespace OmniKit {

// ============================================================================
// UiKey — universal UI element identification key.
// Supports:
//  - string identifier: .key = "start_btn"
//  - integer index:     .key = i (in loops, without std::to_string allocations)
//  - data pointer:      .key = &my_entity
//  - empty key:         UiKey{} (default)
// ============================================================================
struct UiKey {
    enum class Type : uint8_t { None, String, Int, Ptr };

    Type        type = Type::None;
    const char* str = nullptr;
    int         index = 0;
    const void* ptr = nullptr;

    constexpr UiKey() noexcept = default;
    constexpr UiKey(const char* s) noexcept : type((s && s[0] != '\0') ? Type::String : Type::None), str(s) {}
    constexpr UiKey(int i) noexcept         : type(Type::Int), index(i) {}
    constexpr UiKey(const void* p) noexcept : type(p ? Type::Ptr : Type::None), ptr(p) {}

    constexpr bool IsEmpty() const noexcept { return type == Type::None; }
    constexpr explicit operator bool() const noexcept { return !IsEmpty(); }

    void Push() const {
        switch (type) {
            case Type::String:
                if (str && str[0] != '\0') ImGui::PushID(str);
                break;
            case Type::Int:
                ImGui::PushID(index);
                break;
            case Type::Ptr:
                if (ptr) ImGui::PushID(ptr);
                break;
            case Type::None:
                break;
        }
    }
};

// ============================================================================
// HashLocation — constexpr FNV-1a hash of call site in source code.
// Evaluated at compile time during inlining; guarantees unique seed for
// each widget call site in source code.
// ============================================================================
constexpr ImGuiID HashLocation(std::source_location loc) noexcept {
    const char* file = loc.file_name();
    uint32_t line = loc.line();
    uint32_t col = loc.column();

    uint32_t h = 2166136261u;
    for (const char* p = file; *p; ++p)
        h = (h ^ static_cast<uint8_t>(*p)) * 16777619u;
    h = (h ^ static_cast<uint8_t>(line & 0xFF)) * 16777619u;
    h = (h ^ static_cast<uint8_t>((line >> 8) & 0xFF)) * 16777619u;
    h = (h ^ static_cast<uint8_t>((line >> 16) & 0xFF)) * 16777619u;
    h = (h ^ static_cast<uint8_t>(col & 0xFF)) * 16777619u;
    return static_cast<ImGuiID>(h);
}

// ============================================================================
// AutoIdScope — RAII widget identity scope:
//  1. If user explicitly specified options.key, pushes this key.
//  2. Otherwise forms unique ID from call site (loc) and label/icon.
//  3. Pops ID from ImGui stack upon exiting scope.
// ============================================================================
class AutoIdScope {
public:
    AutoIdScope(const UiKey& key, const char* label, std::source_location loc, int salt = 0) {
        if (key) {
            key.Push();
        } else {
            ImGuiID id = HashLocation(loc);
            if (label && label[0] != '\0') {
                id = ImHashStr(label, 0, id);
            } else if (salt != 0) {
                id = ImHashData(&salt, sizeof(int), id);
            }
            ImGui::PushID(static_cast<int>(id));
        }
    }

    ~AutoIdScope() {
        ImGui::PopID();
    }

    AutoIdScope(const AutoIdScope&) = delete;
    AutoIdScope& operator=(const AutoIdScope&) = delete;
};

} // namespace OmniKit
