#pragma once
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <utility>

class Cell;

class Sheet : public std::enable_shared_from_this<Sheet> {
public:
    static constexpr int CELL_WIDTH = 50;
    static constexpr int CELL_HEIGHT = 50;

    Sheet();
    void init_cells();

    bool set_cell(int col, int row, const std::string& value);
    bool set_cell(const std::string& cell_ref, const std::string& value);

    std::shared_ptr<Cell> get_cell(int col, int row);
    std::shared_ptr<Cell> get_cell(const std::string& cell_ref);
    std::optional<std::string> get_cell_val(int col, int row);
    std::optional<std::string> get_cell_val(const std::string& cell_ref);

private:
    std::array<std::array<std::shared_ptr<Cell>, CELL_HEIGHT>, CELL_WIDTH> cells;

    bool parse_int(const std::string& str, int& out);
    std::optional<std::pair<int, int>> cell_ref_to_indices(const std::string& cell_ref);
};
