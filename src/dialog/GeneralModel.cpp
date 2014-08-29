/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3.0 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Hegemony Team
    *********************************************************************/

#include "GeneralModel.h"
#include "engine.h"
#include "settings.h"

#include <QBrush>

GeneralModel::GeneralModel(const QMap<const General *, int> &list, QList<const General *> &keepOrderList)
    : all_generals(list), keep_order_list(keepOrderList)
{
}

int GeneralModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return ColumnTypesCount;
}

int GeneralModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return all_generals.count();
}

QVariant GeneralModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    if (row < 0 || row >= all_generals.size())
        return QVariant();

    const General *general = keep_order_list.at(row);
    switch(role) {
    case Qt::UserRole: return general->objectName();
    case Qt::DisplayRole: {
        switch(index.column()) {
        case TitleColumn: return general->getTitle(all_generals.value(general));
        case NameColumn: return Sanguosha->translate(general->objectName());
        case KingdomColumn: return Sanguosha->translate(general->getKingdom());
        case GenderColumn: return general->isMale() ? tr("Male") : (general->isFemale() ? tr("Female") : tr("NoGender"));
        case MaxHpColumn: {
            QString maxHp;
            if (general->objectName().startsWith("jg_")) {
                maxHp = QString::number(general->getMaxHpHead());
            } else if (general->getMaxHpHead() == general->getMaxHpDeputy()) {
                maxHp = QString::number((float)general->getMaxHpHead() / 2);
            } else {
                maxHp = QString::number((float)general->getMaxHpHead() / 2);
                if (general->getMaxHpHead() != general->getDoubleMaxHp()) {
                    maxHp.prepend("(");
                    maxHp.append(")");
                }
                maxHp.append("/");
                QString deputy_max_hp = QString::number((float)general->getMaxHpDeputy() / 2);
                if (general->getMaxHpDeputy() != general->getDoubleMaxHp()) {
                    deputy_max_hp.prepend("(");
                    deputy_max_hp.append(")");
                }
                maxHp.append(deputy_max_hp);
            }
            return maxHp;
        }
        case PackageColumn: return Sanguosha->translate(general->getPackage());
        }
    }
    case Qt::DecorationRole: {
        if (index.column() == NameColumn && general->isLord()) {
            QIcon icon("image/system/roles/lord.png");
            return icon;
        } else {
            break;
        }
    }
    case Qt::ToolTipRole: {
        switch(index.column()) {
        case TitleColumn:
        case NameColumn: {
            if (Sanguosha->isGeneralHidden(general->objectName())) {
                return tr("<font color=%1>This general is hidden</font>")
                        .arg(Config.SkillDescriptionInToolTipColor.name());
            } else {
                return QString();
            }
        }
        case PackageColumn: {
            if (Config.value("LuaPackages", QString())
                    .toString().split("+").contains(general->getPackage())) {
                return tr("<font color=%1>This is an Lua extension</font>")
                        .arg(Config.SkillDescriptionInToolTipColor.name());
            } else {
                return QString();
            }
        }
        default: return QString();
        }
    }
    case Qt::TextAlignmentRole: return Qt::AlignCenter;
    case Qt::BackgroundRole: {
        switch(index.column()) {
        case TitleColumn:
        case NameColumn: {
            if (Sanguosha->isGeneralHidden(general->objectName()))
                return QBrush(Qt::gray);
            else
                break;
        }
        case PackageColumn: {
            if (Config.value("LuaPackages", QString())
                    .toString().split("+").contains(general->getPackage())) {
                return QBrush(QColor(0x66, 0xCC, 0xFF));
            } else {
                break;
            }
        }
        default: break;
        }
    }
    }

    return QVariant();
}

QVariant GeneralModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case TitleColumn: return tr("Title");
        case NameColumn: return tr("Name");
        case KingdomColumn: return tr("Kingdom");
        case GenderColumn: return tr("Gender");
        case MaxHpColumn: return tr("Max HP");
        case PackageColumn: return tr("Package");
        }
    } else if (orientation == Qt::Vertical) {
        return QString::number(1 + section);
    }

    return QVariant();
}

QModelIndex GeneralModel::firstIndex()
{
    if (all_generals.isEmpty())
        return QModelIndex();
    else
        return createIndex(0, 0);
}
