#include "core/Assets.hpp"
#include <filesystem>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace Assets {

static std::filesystem::path GetExecutableDir() {
#ifdef _WIN32
    std::vector<wchar_t> buffer(MAX_PATH);
    DWORD len = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    while (len == buffer.size()) {
        buffer.resize(buffer.size() * 2);
        len = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    }
    if (len > 0) {
        return std::filesystem::path(buffer.data()).parent_path();
    }
#endif
    return std::filesystem::current_path();
}

std::string Resolve(const std::string& relPath) {
    if (relPath.empty()) return "";

    std::filesystem::path target = std::filesystem::u8path(relPath);
    if (target.is_absolute()) {
        std::error_code ec;
        if (std::filesystem::exists(target, ec)) {
            return target.string();
        }
        return "";
    }

    auto exeDir = GetExecutableDir();

    std::vector<std::filesystem::path> candidates = {
        target,
        std::filesystem::path("assets") / target,
        std::filesystem::path("..") / "assets" / target,
        exeDir / "assets" / target,
        exeDir / ".." / "assets" / target,
        exeDir / ".." / ".." / "assets" / target
    };

    for (const auto& cand : candidates) {
        std::error_code ec;
        if (std::filesystem::exists(cand, ec) && !std::filesystem::is_directory(cand, ec)) {
            return std::filesystem::canonical(cand, ec).string();
        }
    }

    return "";
}

bool Exists(const std::string& relPath) {
    return !Resolve(relPath).empty();
}

} // namespace Assets
