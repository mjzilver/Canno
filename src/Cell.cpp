#include "Cell.hpp"

#include <algorithm>
#include <memory>

#include "Formula.hpp"
#include "Sheet.hpp"

Cell::Cell(std::shared_ptr<Sheet> sheet) : sheet(sheet) {}

std::string Cell::get_value() {
    if (dirty && formula.has_value()) {
        value = compute_value();
        dirty = false;
    }
    return value;
}

std::optional<std::string> Cell::get_formula() { return formula->get_text(); }

void Cell::set_value(const std::string& val) {
    if (!val.empty() && val[0] == '=') {
        formula = Formula(shared_from_this(), val);

        for (auto& parent : parents) {
            auto& siblings = parent->children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), shared_from_this()), siblings.end());
        }
        parents.clear();
        auto deps = formula->calc_deps(sheet);

        for (auto& dep : deps) {
            parents.push_back(dep);
        }

        for (auto& parent : parents) {
            parent->children.emplace_back(shared_from_this());
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

void Cell::add_parent(const std::shared_ptr<Cell>& parent) {
    parents.push_back(parent);
    parent->children.push_back(shared_from_this());
}

std::string Cell::compute_value() {
    if (!formula) {
        return value;
    }

    return formula->evaluate(sheet);
}
