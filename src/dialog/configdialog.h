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

#ifndef _CONFIG_DIALOG_H
#define _CONFIG_DIALOG_H

#include "flatdialog.h"

class QLineEdit;

namespace Ui {
    class ConfigDialog;
}

class ConfigDialog : public FlatDialog {
    Q_OBJECT
public:
    ConfigDialog(QWidget *parent = 0);
    ~ConfigDialog();

private:
    Ui::ConfigDialog *ui;
    void showFont(QLineEdit *lineedit, const QFont &font);

private slots:
    void on_setTextEditColorButton_clicked();
    void on_setTextEditFontButton_clicked();
    void on_changeAppFontButton_clicked();
    void on_resetBgMusicButton_clicked();
    void on_browseBgMusicButton_clicked();
    void on_resetBgButton_clicked();
    void on_browseBgButton_clicked();
    void on_resetTableBgButton_clicked();
    void on_browseTableBgButton_clicked();
    void on_resetRecordPathsButton_clicked();
    void on_browseRecordPathsButton_clicked();
    void saveConfig();

    void on_toolTipFontColorButton_clicked();

    void on_overviewFontColorButton_clicked();

    void on_toolTipBackgroundColorButton_clicked();

signals:
    void bg_changed();
    void tableBg_changed();
};

#endif

