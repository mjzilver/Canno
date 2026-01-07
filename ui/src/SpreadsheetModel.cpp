#include "SpreadsheetModel.hpp"

#include "Cell.hpp"
#include "qnamespace.h"

SpreadsheetModel::SpreadsheetModel(Sheet& s, QObject* parent) : QAbstractTableModel(parent), sheet(s) {}

int SpreadsheetModel::rowCount(const QModelIndex&) const { return sheet.SHEET_ROWS; }
int SpreadsheetModel::columnCount(const QModelIndex&) const { return sheet.SHEET_COLS; }

QVariant SpreadsheetModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        QString header;
        int col = section;
        while (col >= 0) {
            header.prepend(QChar('A' + (col % 26)));
            col = col / 26 - 1;
        }
        return header;
    } else {
        return section + 1;
    }
}

QVariant SpreadsheetModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();

    if (role == Qt::DisplayRole) {
        auto cell_val = sheet.get_cell_val(index.column(), index.row());
        if (cell_val) return QString::fromStdString(cell_val.value());
    } else if (role == Qt::EditRole) {
        auto formula = sheet.get_cell_formula(index.column(), index.row());
        if (formula) return QString::fromStdString(formula.value());
    }
    return QVariant();
}

bool SpreadsheetModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || role != Qt::EditRole) return false;

    if (sheet.set_cell(index.column(), index.row(), value.toString().toStdString())) {
        emit dataChanged(index, index);

        Cell* cell = sheet.get_cell(index.column(), index.row());
        auto deps = cell->get_dependencies();

        for (auto dep : deps) {
            QModelIndex dep_index = this->index(dep->get_row(), dep->get_col());
            emit dataChanged(dep_index, dep_index);
        }
        return true;
    }
    return false;
}

Qt::ItemFlags SpreadsheetModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}
