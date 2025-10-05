#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Formula.hpp"

class Cell : public std::enable_shared_from_this<Cell> {
public:
    explicit Cell(std::shared_ptr<Sheet> parentSheet);

    std::string get_value();
    void set_value(const std::string& val);
    std::optional<std::string> get_formula();

    void mark_dirty();
    void add_parent(const std::shared_ptr<Cell>& parent);

private:
    std::shared_ptr<Sheet> sheet;
    std::string compute_value();
    std::string value;
    std::optional<Formula> formula = std::nullopt;
    std::vector<std::shared_ptr<Cell>> parents;
    std::vector<std::shared_ptr<Cell>> children;
    bool dirty = false;
};
