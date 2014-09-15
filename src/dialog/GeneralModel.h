/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#ifndef GENERALMODEL_H
#define GENERALMODEL_H

#include <QAbstractTableModel>

#include "general.h"

class GeneralModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum ColumnType {
        TitleColumn,
        NameColumn,
        KingdomColumn,
        GenderColumn,
        MaxHpColumn,
        PackageColumn,

        ColumnTypesCount
    };

    explicit GeneralModel(const QMap<const General *, int> &list, QList<const General *> &keepOrderList);

    virtual int columnCount(const QModelIndex &parent) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    inline QMap<const General *, int> *generalMap() { return &all_generals; }

    QModelIndex firstIndex();

private:
    QMap<const General *, int> all_generals;
    QList<const General*> keep_order_list;
};

#endif // GENERALMODEL_H
