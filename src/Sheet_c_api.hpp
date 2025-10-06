#pragma once

extern "C" {

typedef void* SheetHandle;

SheetHandle sheet_create();

int sheet_set_cell(SheetHandle sheet, int col, int row, const char* value);
int sheet_set_cell_ref(SheetHandle sheet, const char* cell_ref, const char* value);

const char* sheet_get_cell_val(SheetHandle sheet, int col, int row);
const char* sheet_get_cell_val_ref(SheetHandle sheet, const char* cell_ref);

const char* sheet_get_cell_formula(SheetHandle sheet, int col, int row);
const char* sheet_get_cell_formula_ref(SheetHandle sheet, const char* cell_ref);

int sheet_cols(SheetHandle sheet);
int sheet_rows(SheetHandle sheet);

}
