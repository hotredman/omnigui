#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>

// ============================================================================
// Текст UTF-8 для оформления интерфейса
// ============================================================================

// Верхний регистр: латиница и кириллица (включая ё); прочие символы — как есть
inline std::string Utf8ToUpper(std::string_view text) {
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        const unsigned char lead = static_cast<unsigned char>(text[i]);
        if (lead >= 'a' && lead <= 'z') {
            out += static_cast<char>(lead - 'a' + 'A');
            continue;
        }
        // Кириллица U+0400..U+047F: 0xD0/0xD1 + байт продолжения
        if ((lead == 0xD0 || lead == 0xD1) && i + 1 < text.size()) {
            unsigned code = ((lead & 0x1Fu) << 6) | (static_cast<unsigned char>(text[i + 1]) & 0x3Fu);
            if (code >= 0x430 && code <= 0x44F) code -= 0x20;       // а..я -> А..Я
            else if (code >= 0x450 && code <= 0x45F) code -= 0x50;  // ѐ..џ (ё) -> Ѐ..Џ (Ё)
            out += static_cast<char>(0xC0 | (code >> 6));
            out += static_cast<char>(0x80 | (code & 0x3F));
            ++i;
            continue;
        }
        out += static_cast<char>(lead);
    }
    return out;
}

// Копирует строку в char-буфер фиксированного размера: обрезает по границе символа UTF-8
// и всегда ставит завершающий нуль (переносимая замена strncpy_s)
inline void CopyTruncated(char* dst, std::size_t capacity, const std::string& src) {
    if (!dst || capacity == 0) return;
    std::size_t n = std::min(src.size(), capacity - 1);
    while (n > 0 && n < src.size() && (static_cast<unsigned char>(src[n]) & 0xC0) == 0x80) --n;
    std::memcpy(dst, src.data(), n);
    dst[n] = '\0';
}
