#pragma once

#include "omnikit/Scope.hpp"
#include <imgui.h>

namespace OmniKit {

// Widget identity scope (wrapper around the ImGui ID stack). Needed when
// identical widgets repeat — in a loop, in table rows, in a list:
//
//     for (auto& run : runs) {
//         if (IdScope id(run.id.c_str())) { Button("Delete"); }
//     }
//
// The constructor pushes an identifier onto the stack, the destructor pops it. The scope is always
// "open" (operator bool == true) so that it can be used in an if-initializer.
class IdScope : public Scope {
public:
    explicit IdScope(const char* key) { ImGui::PushID(key); m_open = true; }
    explicit IdScope(int index)       { ImGui::PushID(index); m_open = true; }
    explicit IdScope(const void* ptr) { ImGui::PushID(ptr); m_open = true; }
    ~IdScope() { ImGui::PopID(); }
};

} // namespace OmniKit
