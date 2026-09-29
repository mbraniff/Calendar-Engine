#include "paths.h"
#include "platform/platform.h"

namespace paths {
    const std::filesystem::path& root() {
        static const std::filesystem::path dir = exeDir();
        return dir;
    }

    std::filesystem::path asset(std::string_view name) { return root() / "assets" / name; }
    std::filesystem::path shader(std::string_view name) { return root() / "shaders" / name; }
    std::filesystem::path font(std::string_view name) { return root() / "assets" / "fonts" / name; }
}