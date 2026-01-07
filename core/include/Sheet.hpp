#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Cell.hpp"

class Sheet {
public:
    Sheet() = default;

    bool set_cell(int row, int col, const std::string& value);
    bool set_cell(const std::string& cell_ref, const std::string& value);

    Cell* get_cell(int row, int col) const;
    Cell* get_cell(const std::string& cell_ref) const;

    std::optional<std::string> get_cell_val(int row, int col) const;
    std::optional<std::string> get_cell_val(const std::string& cell_ref) const;

    std::optional<std::string> get_cell_formula(int row, int col) const;
    std::optional<std::string> get_cell_formula(const std::string& cell_ref) const;

    void clear() {
        rows.clear();
    }

    const std::vector<std::vector<std::unique_ptr<Cell>>>& get_rows() const { return rows; }

    int rows_size() const { return rows.size(); }
    int cols_size(int row) const {
        if (row >= 0 && row <= rows_size()) {
            return rows[row].size();
        }
        return 0;
    }

private:
    void ensure_cell(int row, int col);

    std::vector<std::vector<std::unique_ptr<Cell>>> rows;
};
