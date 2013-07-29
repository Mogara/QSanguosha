#ifndef CLIENTPLAYERLISTMODEL_H
#define CLIENTPLAYERLISTMODEL_H

#include <QAbstractListModel>

class ClientPlayer;

class ClientPlayerListModel: public QAbstractListModel{
    Q_OBJECT

public:
    ClientPlayerListModel(QObject *parent, bool includeNone);
    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

private:
    QList<const ClientPlayer *> list;
};

#endif // CLIENTPLAYERLISTMODEL_H
