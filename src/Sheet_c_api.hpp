#pragma once

extern "C" {

typedef void* SheetHandle;

SheetHandle sheet_create();

int sheet_set_cell(SheetHandle sheet, int col, int row, const char* value);

const char* sheet_get_cell_val(SheetHandle sheet, int col, int row);

const char* sheet_get_cell_formula(SheetHandle sheet, int col, int row);

int* sheet_get_cell_dependencies(SheetHandle, int c_col, int c_row, int* dep_count);

int sheet_cols(SheetHandle sheet);
int sheet_rows(SheetHandle sheet);
}
