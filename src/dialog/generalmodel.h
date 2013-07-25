#ifndef GENERALMODEL_H
#define GENERALMODEL_H

#include "general.h"

#include <QAbstractListModel>
#include <QMap>

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
    GeneralListModel();

    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;

    void doSearch(const QMap<QString, QString> &options);

private:
    GeneralList list;
};


#endif // GENERALMODEL_H
