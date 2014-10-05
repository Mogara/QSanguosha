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

#ifndef _CONNECTION_DIALOG_H
#define _CONNECTION_DIALOG_H

#include <QListWidget>
#include <QComboBox>
#include <QButtonGroup>

#include "flatdialog.h"

namespace Ui {
    class ConnectionDialog;
}

class ConnectionDialog : public FlatDialog {
    Q_OBJECT

public:
    ConnectionDialog(QWidget *parent);
    ~ConnectionDialog();
    void hideAvatarList();
    void showAvatarList();

private:
    Ui::ConnectionDialog *ui;

private slots:
    void on_detectLANButton_clicked();
    void on_clearHistoryButton_clicked();
    void on_avatarList_doubleClicked(const QModelIndex &index);
    void on_changeAvatarButton_clicked();
    void on_connectButton_clicked();
};

#endif

