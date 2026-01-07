#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Formula.hpp"

class Cell {
public:
    explicit Cell(Sheet& parent_sheet, int col, int row);

    std::string get_value();
    void set_value(const std::string& val);

    Formula* get_formula() { return formula ? &*formula : nullptr; }
    std::optional<std::string> get_formula_val();

    void mark_dirty();
    void add_parent(Cell* parent);

    const std::vector<Cell*>& get_dependencies() const { return children; }

    int get_row() { return row; };
    int get_col() { return col; }

private:
    int row, col;

    Sheet& sheet;
    std::string compute_value();
    std::string value;
    std::optional<Formula> formula = std::nullopt;
    std::vector<Cell*> parents;
    std::vector<Cell*> children;
    bool dirty = false;
};
