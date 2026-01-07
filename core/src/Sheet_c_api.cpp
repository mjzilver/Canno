#include "../include/Sheet_c_api.hpp"

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <string>

#include "../include/Cell.hpp"
#include "../include/Sheet.hpp"

extern "C" {

static std::string tmp;

std::unique_ptr<Sheet> sheet = std::make_unique<Sheet>();

SheetHandle sheet_create() { return sheet.get(); }

int sheet_set_cell(SheetHandle handle, int row, int col, const char* value) {
    return static_cast<Sheet*>(handle)->set_cell(row, col, value);
}

const char* sheet_get_cell_val(SheetHandle handle, int row, int col) {
    auto opt = static_cast<Sheet*>(handle)->get_cell_val(row, col);
    tmp = opt.value_or("");
    return tmp.c_str();
}

const char* sheet_get_cell_formula(SheetHandle handle, int row, int col) {
    auto opt = static_cast<Sheet*>(handle)->get_cell_formula(row, col);
    tmp = opt.value_or("");
    return tmp.c_str();
}

// Returns array of dependencies in format: row, col
// Caller must free() the returned array
int* sheet_get_cell_dependencies(SheetHandle handle, int row, int col, int* dep_count) {
    Cell* cell = static_cast<Sheet*>(handle)->get_cell(row, col);
    if (!cell) {
        return nullptr;
    }

    auto deps = cell->get_dependencies();
    *dep_count = static_cast<int>(deps.size());
    if (*dep_count == 0) return nullptr;

    int* dep_arr = (int*)malloc(*dep_count * sizeof(int) * 2);
    if (!dep_arr) {
        *dep_count = 0;
        return nullptr;
    }

    for (size_t i = 0; i < static_cast<size_t>(*dep_count); ++i) {
        auto& dep = deps[i];
        dep_arr[i * 2] = dep->get_row();
        dep_arr[i * 2 + 1] = dep->get_col();
    }

    return dep_arr;
}

int sheet_rows(SheetHandle handle) {
    Sheet* s = static_cast<Sheet*>(handle);
    return static_cast<int>(s->rows_size());
}

int sheet_cols(SheetHandle handle, int row) {
    Sheet* s = static_cast<Sheet*>(handle);
    return static_cast<int>(s->cols_size(row));
}
}