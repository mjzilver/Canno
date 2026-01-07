#pragma once

#include "Sheet.hpp"
#include <filesystem>

class SheetSerializer {
public:
    static bool save_csv(const Sheet& sheet, const std::filesystem::path& path);
    static bool load_csv(Sheet& sheet, const std::filesystem::path& path);
};