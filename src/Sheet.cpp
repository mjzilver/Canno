#include "Sheet.hpp"

#include <cctype>
#include <charconv>
#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>

#include "Cell.hpp"

Sheet::Sheet() {}

void Sheet::init_cells() {
    std::shared_ptr<Sheet> self = shared_from_this();
    for (int x = 0; x < SHEET_COLS; ++x) {
        for (int y = 0; y < SHEET_ROWS; ++y) {
            cells[x][y] = std::make_unique<Cell>(self);
        }
    }
}

bool Sheet::parse_int(const std::string& str, int& out) {
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), out);
    if (ec != std::errc()) {
        std::cerr << "Could not parse int: '" << str << "'\n";
        return false;
    }
    return true;
}

bool Sheet::set_cell(int col, int row, const std::string& value) {
    if (col >= 0 && col < SHEET_COLS && row >= 0 && row < SHEET_ROWS) {
        cells[col][row]->set_value(value);
        return true;
    }
    return false;
}

bool Sheet::set_cell(const std::string& cell_ref, const std::string& value) {
    auto indices = cell_ref_to_indices(cell_ref);
    if (!indices.has_value()) return false;
    return set_cell(indices->first, indices->second, value);
}

std::shared_ptr<Cell> Sheet::get_cell(int col, int row) {
    if (col >= 0 && col < SHEET_COLS && row >= 0 && row < SHEET_ROWS) {
        return cells[col][row];
    }
    return nullptr;
}

std::shared_ptr<Cell> Sheet::get_cell(const std::string& cell_ref) {
    auto indices = cell_ref_to_indices(cell_ref);
    if (!indices.has_value()) return nullptr;
    return get_cell(indices->first, indices->second);
}

std::optional<std::string> Sheet::get_cell_val(int col, int row) {
    if (col >= 0 && col < SHEET_COLS && row >= 0 && row < SHEET_ROWS) {
        return cells[col][row]->get_value();
    }
    return std::nullopt;
}

std::optional<std::string> Sheet::get_cell_val(const std::string& cell_ref) {
    auto indices = cell_ref_to_indices(cell_ref);
    if (!indices.has_value()) return std::nullopt;
    return get_cell_val(indices->first, indices->second);
}

std::optional<std::string> Sheet::get_cell_formula(int col, int row) {
    if (col >= 0 && col < SHEET_COLS && row >= 0 && row < SHEET_ROWS) {
        return cells[col][row]->get_formula();
    }
    return std::nullopt;
}
std::optional<std::string> Sheet::get_cell_formula(const std::string& cell_ref) {
    auto indices = cell_ref_to_indices(cell_ref);
    if (!indices.has_value()) return std::nullopt;
    return get_cell_formula(indices->first, indices->second);
}

std::optional<std::pair<int, int>> Sheet::cell_ref_to_indices(const std::string& cell_ref) {
    int col = 0;
    int row = 0;
    size_t i = 0;

    while (i < cell_ref.size() && std::isalpha(cell_ref[i])) {
        char c = std::toupper(cell_ref[i]);
        col = col * 26 + (c - 'A' + 1);
        ++i;
    }

    if (!parse_int(cell_ref.substr(i), row)) {
        std::cerr << "Invalid row in cell reference: " << cell_ref << "\n";
        return std::nullopt;
    }

    if (col == 0 || row == 0) {
        std::cerr << "Invalid cell reference format: " << cell_ref << "\n";
        return std::nullopt;
    }

    return std::pair{col - 1, row - 1};
}
