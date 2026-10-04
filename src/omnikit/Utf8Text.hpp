#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>

namespace OmniKit {

// ============================================================================
// UTF-8 text formatting utilities
// ============================================================================

// Uppercase: Latin and Cyrillic (including yo); other characters untouched
inline std::string Utf8ToUpper(std::string_view text) {
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        const unsigned char lead = static_cast<unsigned char>(text[i]);
        if (lead >= 'a' && lead <= 'z') {
            out += static_cast<char>(lead - 'a' + 'A');
            continue;
        }
        // Cyrillic U+0400..U+047F: 0xD0/0xD1 + continuation byte
        if ((lead == 0xD0 || lead == 0xD1) && i + 1 < text.size()) {
            unsigned code = ((lead & 0x1Fu) << 6) | (static_cast<unsigned char>(text[i + 1]) & 0x3Fu);
            if (code >= 0x430 && code <= 0x44F) code -= 0x20;       // lowercase cyrillic -> uppercase cyrillic
            else if (code >= 0x450 && code <= 0x45F) code -= 0x50;  // cyrillic io/extended -> uppercase
            out += static_cast<char>(0xC0 | (code >> 6));
            out += static_cast<char>(0x80 | (code & 0x3F));
            ++i;
            continue;
        }
        out += static_cast<char>(lead);
    }
    return out;
}

// Copies string into fixed-size char buffer: truncates at UTF-8 character boundary
// and always null-terminates (portable alternative to strncpy_s)
inline void CopyTruncated(char* dst, std::size_t capacity, const std::string& src) {
    if (!dst || capacity == 0) return;
    std::size_t n = std::min(src.size(), capacity - 1);
    while (n > 0 && n < src.size() && (static_cast<unsigned char>(src[n]) & 0xC0) == 0x80) --n;
    std::memcpy(dst, src.data(), n);
    dst[n] = '\0';
}

} // namespace OmniKit
