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

#include "avatarmodel.h"
#include "skinbank.h"

AvatarModel::AvatarModel(const GeneralList &list)
    : list(list)
{
}

int AvatarModel::rowCount(const QModelIndex &) const
{
    return list.size();
}

QVariant AvatarModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= list.length())
        return QVariant();

    const General *general = list.at(row);

    switch(role) {
    case Qt::UserRole: return general->objectName();
    case Qt::DisplayRole: return Sanguosha->translate(general->objectName());
    case Qt::DecorationRole: {
        QIcon icon(G_ROOM_SKIN.getGeneralPixmap(general->objectName(),
                                                QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE));
        return icon;
    }
    }

    return QVariant();
}
