#ifndef GENERALMODEL_H
#define GENERALMODEL_H

#include <QAbstractListModel>

#include "general.h"

class GeneralModel : public QAbstractListModel{
    Q_OBJECT
public:

    explicit GeneralModel(const GeneralList &list);

    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

private:
    GeneralList list;
};

#endif // GENERALMODEL_H
