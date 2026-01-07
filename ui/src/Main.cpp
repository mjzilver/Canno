#include <QApplication>
#include <QTableView>

#include "Sheet.hpp"
#include "SpreadsheetModel.hpp"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationName("Canno Spreadsheet");

    Sheet sheet;

    SpreadsheetModel model(sheet);

    QTableView view;
    view.resize(500, 500);
    view.setMinimumSize(500, 500);

    view.setModel(&model);

    view.setSelectionBehavior(QAbstractItemView::SelectItems);
    view.setSelectionMode(QAbstractItemView::SingleSelection);

    view.setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);

    view.show();

    return app.exec();
}
