#pragma once
#include <filesystem>
#include <string_view>

namespace paths {
    const std::filesystem::path& root();
    std::filesystem::path asset(std::string_view name);
    std::filesystem::path shader(std::string_view name);
    std::filesystem::path font(std::string_view name);
}