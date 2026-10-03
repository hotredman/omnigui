#include "core/Assets.hpp"
#include "core/PathUtil.hpp"
#include <filesystem>
#include <vector>

#include <SDL3/SDL_filesystem.h>

namespace Assets {

static std::filesystem::path GetExecutableDir() {
    // SDL_GetBasePath: каталог приложения на Windows, Linux и macOS (строка принадлежит SDL)
    if (const char* base = SDL_GetBasePath()) {
        return PathFromUtf8(base);
    }
    return std::filesystem::current_path();
}

std::string Resolve(const std::string& relPath) {
    if (relPath.empty()) return "";

    std::filesystem::path target = PathFromUtf8(relPath);
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

std::string DefaultFontPath() {
    return Resolve("fonts/Roboto-Regular.ttf");
}

} // namespace Assets
