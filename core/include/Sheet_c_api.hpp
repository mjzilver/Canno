#pragma once

extern "C" {

typedef void* SheetHandle;

SheetHandle sheet_create();

int sheet_set_cell(SheetHandle sheet, int row, int col, const char* value);

const char* sheet_get_cell_val(SheetHandle sheet, int row, int col);

const char* sheet_get_cell_formula(SheetHandle sheet, int row, int col);

int* sheet_get_cell_dependencies(SheetHandle, int row, int col, int* dep_count);

int sheet_cols(SheetHandle sheet, int row);
int sheet_rows(SheetHandle sheet);
}
