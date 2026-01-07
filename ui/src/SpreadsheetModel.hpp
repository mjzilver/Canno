#pragma once

#include <QAbstractTableModel>
#include <QString>

#include "Sheet.hpp"

class SpreadsheetModel : public QAbstractTableModel {
    Q_OBJECT

private:
    Sheet& sheet;

public:
    explicit SpreadsheetModel(Sheet& s, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
};
