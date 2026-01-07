#include <QApplication>
#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QHBoxLayout>
#include <QFileDialog>
#include "Sheet.hpp"
#include "SheetSerializer.hpp"
#include "SpreadsheetModel.hpp"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationName("Canno Spreadsheet");

    Sheet sheet;
    SpreadsheetModel model(sheet);

    QWidget window;

    QVBoxLayout* main_layout = new QVBoxLayout(&window);

    QTableView* view = new QTableView;
    view->setModel(&model);
    view->setSelectionBehavior(QAbstractItemView::SelectItems);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);
    view->setMinimumSize(500, 500);

    main_layout->addWidget(view);

    QWidget* bottom_bar = new QWidget;
    QHBoxLayout* bar_layout = new QHBoxLayout(bottom_bar);

    QPushButton* open_button = new QPushButton("Open CSV");
    QPushButton* save_button = new QPushButton("Save CSV");

    bar_layout->addWidget(open_button);
    bar_layout->addWidget(save_button);

    main_layout->addWidget(bottom_bar); 

    QObject::connect(open_button, &QPushButton::clicked, [&]() {
        QString path = QFileDialog::getOpenFileName(&window, "Open CSV File", "", "CSV Files (*.csv)");
        if (!path.isEmpty()) {
            SheetSerializer::load_csv(sheet, path.toStdString());
            model.layoutChanged(); 
        }
    });

    QObject::connect(save_button, &QPushButton::clicked, [&]() {
        QString path = QFileDialog::getSaveFileName(&window, "Save CSV File", "", "CSV Files (*.csv)");
        if (!path.isEmpty()) {
            SheetSerializer::save_csv(sheet, path.toStdString());
        }
    });

    window.setLayout(main_layout);
    window.resize(600, 600);
    window.show();

    return app.exec();
}
