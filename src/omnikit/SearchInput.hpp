#pragma once

#include "omnikit/UiTheme.hpp"
#include "omnikit/UiKey.hpp"
#include <string>
#include <source_location>

namespace OmniKit {

// Search input options (designated initializers):
//
//     SearchInput(query, {.hint = "Search datasets..."});
//     SearchInput(query, {.hint = "Filter...", .width = 220});
//     SearchInput(query, {.hint = "Search...", .width = Fill});   // fill full width
struct SearchInputOptions {
    const char* hint  = "Search...";       // Placeholder text
    UiSize      size  = UiSize::Small;
    float       width = 0.0f;              // Base px (scaled internally); 0 uses default width; Fill expands to available width
    UiKey       key   = {};                // Identity; default uses hint
};

// Design system search input supporting UiSize, placeholder, and quick clear button.
// Returns true if query string changed
bool SearchInput(std::string& query, const SearchInputOptions& options = {},
                 std::source_location loc = std::source_location::current());

} // namespace OmniKit
