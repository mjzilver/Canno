import ctypes

class CannoFFI:
    def __init__(self, lib_path="bin/libcanno.so"):
        self.lib = ctypes.CDLL(lib_path)
        self._bind_functions()
        self.sheet = self.lib.sheet_create()

    def _bind_functions(self):
        self.lib.sheet_create.restype = ctypes.c_void_p

        self.lib.sheet_set_cell.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int, ctypes.c_char_p]
        self.lib.sheet_set_cell.restype = ctypes.c_int

        self.lib.sheet_get_cell_val.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        self.lib.sheet_get_cell_val.restype = ctypes.c_char_p

        self.lib.sheet_get_cell_formula.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        self.lib.sheet_get_cell_formula.restype = ctypes.c_char_p

        self.lib.sheet_cols.argtypes = [ctypes.c_void_p]
        self.lib.sheet_cols.restype = ctypes.c_int

        self.lib.sheet_rows.argtypes = [ctypes.c_void_p]
        self.lib.sheet_rows.restype = ctypes.c_int

    def set_cell(self, col, row, value):
        return self.lib.sheet_set_cell(self.sheet, col, row, value.encode())

    def get_cell_val(self, col, row):
        val = self.lib.sheet_get_cell_val(self.sheet, col, row)
        return val.decode() if val else ""

    def get_cell_formula(self, col, row):
        form = self.lib.sheet_get_cell_formula(self.sheet, col, row)
        return form.decode() if form else ""

    def cols(self):
        return self.lib.sheet_cols(self.sheet)

    def rows(self):
        return self.lib.sheet_rows(self.sheet)