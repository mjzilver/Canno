#include "../include/Cell.hpp"

#include <algorithm>
#include <optional>

#include "../include/Formula.hpp"
#include "../include/Sheet.hpp"

Cell::Cell(Sheet& sheet, int row, int col) : row(row), col(col), sheet(sheet) {}

std::string Cell::get_value() {
    if (dirty && formula.has_value()) {
        value = compute_value();
        dirty = false;
    }
    return value;
}

std::optional<std::string> Cell::get_formula_val() {
    if (!formula.has_value()) {
        return std::nullopt;
    }
    return formula->get_text();
}

std::string Cell::get_formula_or_value() {
    return formula.has_value() ? formula->get_text() : value;
}

void Cell::set_value(const std::string& val) {
    if (!val.empty() && val[0] == '=') {
        formula = Formula(this, val);

        for (auto& parent : parents) {
            auto& siblings = parent->children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        }
        parents.clear();
        auto deps = formula->calc_deps(sheet);

        for (auto& dep : deps) {
            parents.push_back(dep);
        }

        for (auto& parent : parents) {
            parent->children.emplace_back(this);
        }

        dirty = true;
    } else {
        formula.reset();
        value = val;
        dirty = false;
    }

    for (auto& child : children) {
        child->mark_dirty();
    }
}

void Cell::mark_dirty() {
    if (!dirty) {
        dirty = true;
        for (auto& child : children) {
            child->mark_dirty();
        }
    }
}

void Cell::add_parent(Cell* parent) {
    parents.push_back(parent);
    parent->children.push_back(this);
}

std::string Cell::compute_value() {
    if (!formula.has_value()) {
        return value;
    }

    return formula->evaluate(sheet);
}
