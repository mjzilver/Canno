#include "../include/Sheet.hpp"

#include <memory>
#include <optional>
#include <string>

#include "../include/Cell.hpp"
#include "../include/Utils.hpp"

bool Sheet::set_cell(int row, int col, const std::string& value) {
    if (col >= 0 && row >= 0 ) {
        ensure_cell(row, col);
        rows[row][col]->set_value(value);
        return true;
    }
    return false;
}

bool Sheet::set_cell(const std::string& cell_ref, const std::string& value) {
    auto indices = cell_ref_to_indices(cell_ref);
    if (!indices.has_value()) return false;
    return set_cell(indices->first, indices->second, value);
}

Cell* Sheet::get_cell(int row, int col) const {

    if (row >= 0 && row < rows_size() && col >= 0 && col < cols_size(row)) {
        return rows[row][col].get();
    }
    return nullptr;
}

Cell* Sheet::get_cell(const std::string& cell_ref) const {
    auto indices = cell_ref_to_indices(cell_ref);
    if (!indices.has_value()) return nullptr;
    return get_cell(indices->first, indices->second);
}

std::optional<std::string> Sheet::get_cell_val(int row, int col) const {
    if (row >= 0 && row < rows_size() && col >= 0 && col < cols_size(row)) {
        if(!rows[row][col]) return std::nullopt;
        return rows[row][col]->get_value();
    }
    return std::nullopt;
}

std::optional<std::string> Sheet::get_cell_val(const std::string& cell_ref) const {
    auto indices = cell_ref_to_indices(cell_ref);
    if (!indices.has_value()) return std::nullopt;
    return get_cell_val(indices->first, indices->second);
}

std::optional<std::string> Sheet::get_cell_formula(int row, int col) const {
    if (row >= 0 && row < rows_size() && col >= 0 && col < cols_size(row)) {
        if(!rows[row][col]) return std::nullopt;
        return rows[row][col]->get_formula_val();
    }
    return std::nullopt;
}

std::optional<std::string> Sheet::get_cell_formula(const std::string& cell_ref) const {
    auto indices = cell_ref_to_indices(cell_ref);
    if (!indices.has_value()) return std::nullopt;
    return get_cell_formula(indices->first, indices->second);
}

void Sheet::ensure_cell(int row, int col) {
    if (row >= static_cast<int>(rows.size())) {
        rows.resize(row + 1);
    }

    if (col >= static_cast<int>(rows[row].size())) {
        rows[row].resize(col + 1);
    }

    if (!rows[row][col]) {
        rows[row][col] = std::make_unique<Cell>(*this, row, col);
    }
}
