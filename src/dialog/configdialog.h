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

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

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

    typedef void (ConfigDialog::*Callback) (const QVariant &);
    QList<Callback> resetCallbacks;
    QVariantList callbackArgs;

    void doCallback(Callback callback, const QVariant &oldValue, const QVariant &newValue);

    //callbacks
    void setBackground(const QVariant &path);
    void setTableBg(const QVariant &path);
    void setTooltipBackgroundColor(const QVariant &color);
    void setAppFont(const QVariant &font);
    void setTextEditFont(const QVariant &font);
    void setTextEditColor(const QVariant &color);
    void setTooltipFontColor(const QVariant &color);
    void setOverviewFontColor(const QVariant &color);
    void setBgMusic(const QVariant &path);
    void setEffectsEnabled(const QVariant &enabled);
    void setLastWordEnabled(const QVariant &enabled);
    void setBGMEnabled(const QVariant &enabled);
    void setBGMVolume(const QVariant &volume);
    void setEffectVolume(const QVariant &volume);
    void setRecordsSavePath(const QVariant &path);

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
    void on_resetRecordPathButton_clicked();
    void on_browseRecordPathButton_clicked();

    void on_toolTipFontColorButton_clicked();
    void on_overviewFontColorButton_clicked();
    void on_toolTipBackgroundColorButton_clicked();

    void onEffectsEnabledChanged(bool enabled);
    void onLastWordEnabledChanged(bool enabled);
    void onBGMVolumeChanged(int volume);
    void onEffectVolumeChanged(int volume);
    void saveConfig();
    void discardSettings();

signals:
    void bg_changed();
    void tableBg_changed();
};

#endif // CONFIGDIALOG_H

