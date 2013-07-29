#include "clientplayerlistmodel.h"

#include "client.h"
#include "engine.h"

ClientPlayerListModel::ClientPlayerListModel(QObject *parent, bool includeNone)
    :QAbstractListModel(parent)
{
    if(includeNone)
        list << NULL;
    list << ClientInstance->getPlayers();
}

int ClientPlayerListModel::rowCount(const QModelIndex &parent) const{
    return parent.isValid() ? 0 : list.length();
}

QVariant ClientPlayerListModel::data(const QModelIndex &index, int role) const{
    int i = index.row();
    if(i < 0 || i >= list.length())
        return QVariant();

    const ClientPlayer *p = list[i];
    const General *g = p ? p->getGeneral() : NULL;

    switch(role){
    case Qt::DisplayRole: {
        if(p){
            return QString("%1 [%2]")
                    .arg(Sanguosha->translate(g->objectName()))
                    .arg(p->screenName());
        }else
            return tr("None");
    }

    case Qt::DecorationRole:{
        if(p)
            return QIcon(g->getTinyIconPath());
        else{
            QPixmap pixmap(General::TinyIconSize);
            pixmap.fill();
            return QIcon(pixmap);
        }
    }

    case Qt::UserRole:
        if(p)
            return p->objectName();
        else
            return ".";
    }

    return QVariant();
}
