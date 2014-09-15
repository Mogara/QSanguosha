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

#ifndef _PLAYER_CARD_DIALOG_H
#define _PLAYER_CARD_DIALOG_H

#include "clientplayer.h"

#include <QDialog>
#include <QMap>
#include <QGroupBox>

class MagatamaWidget : public QWidget {
    Q_OBJECT

public:
    explicit MagatamaWidget(int hp, Qt::Orientation orientation);

    static QPixmap GetMagatama(int index);
    static QPixmap GetSmallMagatama(int index);
};

class PlayerCardDialog : public QDialog {
    Q_OBJECT

public:
    explicit PlayerCardDialog(const ClientPlayer *player, const QString &flags = "hej",
        bool handcard_visible = false, Card::HandlingMethod method = Card::MethodNone,
        const QList<int> &disabled_ids = QList<int>());

private:
    QGroupBox *createButtonArea(const CardList &list, const QString &title, const QString &noCardText);
    QWidget *createAvatar();
    QWidget *createHandcardButton();
    QWidget *createEquipArea();
    QWidget *createJudgingArea();

    const ClientPlayer *player;
    bool handcard_visible;
    Card::HandlingMethod method;
    QList<int> disabled_ids;

signals:
    void idSelected(int card_id);
};

#endif

