#include "generalmodel.h"
#include "engine.h"

GeneralModel::GeneralModel(const GeneralList &list)
    :list(list)
{

}

int GeneralModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;
    else
        return list.length();
}

QVariant GeneralModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if(row < 0 || row >= list.length())
        return QVariant();

    const General *g = list[row];

    switch(role){
    case Qt::EditRole: return g->objectName();
    case Qt::DisplayRole: return QString("%1 (%2)").arg(g->objectName()).arg(Sanguosha->translate(g->objectName()));
    case Qt::BackgroundRole: if(g->isHidden()) return QBrush(Qt::gray);
    case Qt::DecorationRole: return QIcon(g->getPixmapPath("tiny"));
    }

    return QVariant();
}


