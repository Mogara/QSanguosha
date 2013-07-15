#include "generalmodel.h"
#include "engine.h"

GeneralModel::GeneralModel(const GeneralList &list)
    :list(list)
{
}

int GeneralModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;

    return ColumnTypesCount;
}

int GeneralModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;

    return list.count();
}

QVariant GeneralModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if(row < 0 || row >= list.length())
        return QVariant();

    const General *general = list.at(row);
    switch(role){
    case Qt::UserRole:
    case Qt::EditRole: return general->objectName();
    case Qt::DisplayRole:{
        switch(index.column()){
        case NameColumn: return Sanguosha->translate(general->objectName());
        case KingdomColumn: return Sanguosha->translate(general->getKingdom());
        case GenderColumn: return Sanguosha->translate(general->getGenderString());
        case MaxHpColumn: return general->getMaxHp();
        case PackageColumn: return Sanguosha->translate(general->getPackage());
        case TitleColumn: return Sanguosha->translate("#" + general->objectName(), tr("No Title"));
        }
    }
    case Qt::BackgroundRole:{
        if(general->isHidden())
            return QBrush(Qt::gray);
    }
    }

    return QVariant();
}

QVariant GeneralModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)
        return QVariant();

    if(orientation == Qt::Horizontal){
        switch(section){
        case NameColumn: return tr("Name");
        case KingdomColumn: return tr("Kingdom");
        case GenderColumn: return tr("Gender");
        case MaxHpColumn: return tr("Max HP");
        case PackageColumn: return tr("Package");
        case TitleColumn: return tr("Title");
        }
    }else if(orientation == Qt::Vertical){
        return QString::number(1 + section);
    }

    return QVariant();
}

QModelIndex GeneralModel::firstIndex()
{
    if(list.isEmpty())
        return QModelIndex();
    else
        return createIndex(0, 0);

}


