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

#ifndef _CARD_OVERVIEW_H
#define _CARD_OVERVIEW_H

#include "card.h"

#include <QDialog>
#include <QTableWidgetItem>

class MainWindow;
namespace Ui {
    class CardOverview;
}

class CardOverview : public QDialog {
    Q_OBJECT

public:
    static CardOverview *getInstance(QWidget *main_window);

    CardOverview(QWidget *parent = 0);
    void loadFromAll();
    void loadFromList(const QList<const Card *> &list);

    ~CardOverview();

private:
    Ui::CardOverview *ui;

    void addCard(int i, const Card *card);

private slots:
    void on_femalePlayButton_clicked();
    void on_malePlayButton_clicked();
    void on_playAudioEffectButton_clicked();
    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);
    void on_tableWidget_itemSelectionChanged();
    void askCard();

protected:
    void showEvent(QShowEvent *);
};

#endif

