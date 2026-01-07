#pragma once

#include <array>
#include <memory>
#include <optional>
#include <string>

#include "Cell.hpp"

class Sheet {
public:
    static constexpr int SHEET_COLS = 50;
    static constexpr int SHEET_ROWS = 50;

    Sheet();

    bool set_cell(int col, int row, const std::string& value);
    bool set_cell(const std::string& cell_ref, const std::string& value);

    Cell* get_cell(int col, int row);
    Cell* get_cell(const std::string& cell_ref);
    std::optional<std::string> get_cell_val(int col, int row);
    std::optional<std::string> get_cell_val(const std::string& cell_ref);
    std::optional<std::string> get_cell_formula(int col, int row);
    std::optional<std::string> get_cell_formula(const std::string& cell_ref);

private:
    std::array<std::array<std::unique_ptr<Cell>, SHEET_ROWS>, SHEET_COLS> cells;
};
