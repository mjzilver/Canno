#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Formula.hpp"

class Cell {
public:
    explicit Cell(Sheet& parent_sheet, int row, int col);

    std::string get_value();
    void set_value(const std::string& val);

    Formula* get_formula() { return formula ? &*formula : nullptr; }
    std::optional<std::string> get_formula_val();

    std::string get_formula_or_value();

    void mark_dirty();
    void add_parent(Cell* parent);

    const std::vector<Cell*>& get_dependencies() const { return children; }

    int get_row() { return row; };
    int get_col() { return col; }

    bool has_formula() { return formula.has_value(); };

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
