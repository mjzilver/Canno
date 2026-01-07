import ctypes

from config import LIBCANNO_PATH

class CannoFFI:
    def __init__(self, lib_path=LIBCANNO_PATH):
        self.lib = ctypes.CDLL(lib_path)
        self._bind_functions()
        self.sheet = self.lib.sheet_create()

    def _bind_functions(self):
        self.lib.sheet_create.restype = ctypes.c_void_p

        self.lib.sheet_set_cell.argtypes = [
            ctypes.c_void_p,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_char_p,
        ]
        self.lib.sheet_set_cell.restype = ctypes.c_int

        self.lib.sheet_get_cell_val.argtypes = [
            ctypes.c_void_p,
            ctypes.c_int,
            ctypes.c_int,
        ]
        self.lib.sheet_get_cell_val.restype = ctypes.c_char_p

        self.lib.sheet_get_cell_formula.argtypes = [
            ctypes.c_void_p,
            ctypes.c_int,
            ctypes.c_int,
        ]
        self.lib.sheet_get_cell_formula.restype = ctypes.c_char_p

        self.lib.sheet_get_cell_dependencies.argtypes = [
            ctypes.c_void_p,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.POINTER(ctypes.c_int),
        ]
        self.lib.sheet_get_cell_dependencies.restype = ctypes.POINTER(ctypes.c_int)

        self.lib.sheet_cols.argtypes = [ctypes.c_void_p, ctypes.c_int]
        self.lib.sheet_cols.restype = ctypes.c_int

        self.lib.sheet_rows.argtypes = [ctypes.c_void_p]
        self.lib.sheet_rows.restype = ctypes.c_int

    def set_cell(self, row, col, value):
        return self.lib.sheet_set_cell(self.sheet, row, col, value.encode())

    def get_cell_val(self, row, col):
        val = self.lib.sheet_get_cell_val(self.sheet, row, col)
        return val.decode() if val else ""

    def get_cell_formula(self, row, col):
        form = self.lib.sheet_get_cell_formula(self.sheet, row, col)
        return form.decode() if form else ""

    def get_cell_deps(self, row, col):
        count = ctypes.c_int()
        ptr = self.lib.sheet_get_cell_dependencies(
            self.sheet, row, col, ctypes.byref(count)
        )
        if not ptr:
            return []

        deps = [(ptr[i * 2], ptr[i * 2 + 1]) for i in range(count.value)]
        self.lib.free(ptr)
        return deps

    def cols(self, row):
        return self.lib.sheet_cols(self.sheet, row)

    def rows(self):
        return self.lib.sheet_rows(self.sheet)
