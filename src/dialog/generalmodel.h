#ifndef GENERALMODEL_H
#define GENERALMODEL_H

#include <QAbstractTableModel>

#include "general.h"

class GeneralModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum ColumnType{
        NameColumn,
        KingdomColumn,
        GenderColumn,
        MaxHpColumn,
        PackageColumn,
        TitleColumn,

        ColumnTypesCount
    };

    explicit GeneralModel(const GeneralList &list);

    virtual int columnCount(const QModelIndex &parent) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    QModelIndex firstIndex();

private:
    GeneralList list;
};

#endif // GENERALMODEL_H
