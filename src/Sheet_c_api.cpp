#include "Sheet_c_api.hpp"

#include <memory>
#include <string>

#include "Sheet.hpp"

extern "C" {

static std::string tmp;

std::shared_ptr<Sheet> sheet = std::make_shared<Sheet>();

SheetHandle sheet_create() {
    sheet->init_cells();
    return sheet.get();
}

int sheet_set_cell(SheetHandle handle, int col, int row, const char* value) {
    return static_cast<Sheet*>(handle)->set_cell(col, row, value);
}

int sheet_set_cell_ref(SheetHandle handle, const char* cell_ref, const char* value) {
    return static_cast<Sheet*>(handle)->set_cell(cell_ref, value);
}

const char* sheet_get_cell_val(SheetHandle handle, int col, int row) {
    auto opt = static_cast<Sheet*>(handle)->get_cell_val(col, row);
    tmp = opt.value_or("");
    return tmp.c_str();
}

const char* sheet_get_cell_val_ref(SheetHandle handle, const char* cell_ref) {
    auto opt = static_cast<Sheet*>(handle)->get_cell_val(cell_ref);
    tmp = opt.value_or("");
    return tmp.c_str();
}

const char* sheet_get_cell_formula(SheetHandle handle, int col, int row) {
    auto opt = static_cast<Sheet*>(handle)->get_cell_formula(col, row);
    tmp = opt.value_or("");
    return tmp.c_str();
}

const char* sheet_get_cell_formula_ref(SheetHandle handle, const char* cell_ref) {
    auto opt = static_cast<Sheet*>(handle)->get_cell_formula(cell_ref);
    tmp = opt.value_or("");
    return tmp.c_str();
}

int sheet_cols(SheetHandle handle) { return static_cast<Sheet*>(handle)->SHEET_COLS; }
int sheet_rows(SheetHandle handle) { return static_cast<Sheet*>(handle)->SHEET_ROWS; }
}