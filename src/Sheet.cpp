#include "Sheet.hpp"

#include <memory>
#include <optional>

#include "Cell.hpp"
#include "Utils.hpp"

Sheet::Sheet() {}

void Sheet::init_cells() {
    std::shared_ptr<Sheet> self = shared_from_this();
    for (int col = 0; col < SHEET_COLS; col++) {
        for (int row = 0; row < SHEET_ROWS; row++) {
            cells[col][row] = std::make_unique<Cell>(self, col, row);
        }
    }
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
        return cells[col][row]->get_formula_val();
    }
    return std::nullopt;
}

std::optional<std::string> Sheet::get_cell_formula(const std::string& cell_ref) {
    auto indices = cell_ref_to_indices(cell_ref);
    if (!indices.has_value()) return std::nullopt;
    return get_cell_formula(indices->first, indices->second);
}
