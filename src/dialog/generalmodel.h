#ifndef GENERALMODEL_H
#define GENERALMODEL_H

#include "general.h"

#include <QAbstractListModel>

class GeneralCompleterModel : public QAbstractListModel{
    Q_OBJECT

public:
    explicit GeneralCompleterModel();

    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;

private:
    QList<const QObject *> list;
};

class GeneralListModel: public QAbstractListModel{
    Q_OBJECT

public:
    struct SearchOptions{
        QString kingdom;
        QString gender;
        QString package;
    };

    GeneralListModel();

    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;

    void doSearch(const SearchOptions &options);

private:
    GeneralList list;
};


#endif // GENERALMODEL_H
