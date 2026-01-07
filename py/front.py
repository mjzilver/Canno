import tkinter as tk

from canno import CannoFFI

canno = CannoFFI()

cols = 50
rows = 50

root = tk.Tk()
root.title("Canno Spreadsheet")
root.geometry("800x600")

frame = tk.Frame(root)
frame.pack(expand=True, fill="both")

canvas = tk.Canvas(frame)
scrollbar_y = tk.Scrollbar(frame, orient="vertical", command=canvas.yview)
scrollbar_x = tk.Scrollbar(frame, orient="horizontal", command=canvas.xview)
scrollable_frame = tk.Frame(canvas)

scrollable_frame.bind(
    "<Configure>", lambda e: canvas.configure(scrollregion=canvas.bbox("all"))
)

canvas.create_window((0, 0), window=scrollable_frame, anchor="nw")
canvas.configure(yscrollcommand=scrollbar_y.set, xscrollcommand=scrollbar_x.set)

canvas.grid(row=0, column=0, sticky="nsew")
scrollbar_y.grid(row=0, column=1, sticky="ns")
scrollbar_x.grid(row=1, column=0, sticky="ew")

frame.grid_rowconfigure(0, weight=1)
frame.grid_columnconfigure(0, weight=1)

entries = {}


def col_label(index):
    label = ""
    while index >= 0:
        label = chr(ord("A") + index % 26) + label
        index = index // 26 - 1
    return label


for c in range(cols):
    label = tk.Label(
        scrollable_frame, text=col_label(c), borderwidth=1, relief="raised", width=10
    )
    label.grid(row=0, column=c + 1, sticky="nsew")

for r in range(rows):
    label = tk.Label(
        scrollable_frame, text=str(r + 1), borderwidth=1, relief="raised", width=4
    )
    label.grid(row=r + 1, column=0, sticky="nsew")


def draw_sheet():
    for r in range(rows):
        for c in range(cols):
            val = canno.get_cell_val(r, c)
            e = tk.Entry(scrollable_frame, width=15)
            e.grid(row=r + 1, column=c + 1)
            e.insert(0, val)
            entries[(r, c)] = e
            e.bind("<FocusOut>", lambda event, row=r, col=c: save_cell(event, row, col))
            e.bind("<FocusIn>", lambda event, row=r, col=c: enter_cell(event, row, col))


def update_deps(deps):
    [update_cell(r, c) for r, c in deps]


def update_cell(row, col):
    val = canno.get_cell_val(row, col)
    entries[(row, col)].delete(0, tk.END)
    entries[(row, col)].insert(0, val)


def enter_cell(_, row, col):
    formula = canno.get_cell_formula(row, col)
    if formula:
        entries[(row, col)].delete(0, tk.END)
        entries[(row, col)].insert(0, formula)


def save_cell(_, row, col):
    value = entries[(row, col)].get()
    canno.set_cell(row, col, value)

    update_cell(row, col)
    update_deps(canno.get_cell_deps(row, col))


draw_sheet()
root.mainloop()
