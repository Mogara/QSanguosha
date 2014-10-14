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

#ifndef GENERALOVERVIEW_H
#define GENERALOVERVIEW_H

#include "flatdialog.h"
#include <QModelIndex>

class QCheckBox;
class QLineEdit;
class QButtonGroup;
class QSpinBox;
class General;
class QCommandLinkButton;
class Skill;
class GeneralOverview;

class GeneralSearch : public FlatDialog {
    Q_OBJECT

public:
    GeneralSearch(GeneralOverview *parent);

private:
    QCheckBox *include_hidden_checkbox;
    QLabel *nickname_label;
    QLineEdit *nickname_edit;
    QLabel *name_label;
    QLineEdit *name_edit;
    QButtonGroup *gender_buttons;
    QButtonGroup *kingdom_buttons;
    QLabel *maxhp_lower_label, *maxhp_upper_label;
    QSpinBox *maxhp_lower_spinbox;
    QSpinBox *maxhp_upper_spinbox;
    QPushButton *select_all_button, *unselect_all_button;
    QButtonGroup *package_buttons;

signals:
    void search(bool include_hidden, const QString &nickname, const QString &name, const QStringList &genders,
        const QStringList &kingdoms, int lower, int upper, const QStringList &packages);

protected:
    virtual void accept();

private:
    QWidget *createInfoTab();
    QLayout *createButtonLayout();

private slots:
    void clearAll();
    void selectAllPackages();
    void unselectAllPackages();
};

namespace Ui {
    class GeneralOverview;
}

class GeneralOverview : public FlatDialog {
    Q_OBJECT

public:
    GeneralOverview(QWidget *parent = 0);
    ~GeneralOverview();
    void fillGenerals(const QList<const General *> &generals, bool init = true);

    static GeneralOverview *getInstance(QWidget *main_window);

private:
    Ui::GeneralOverview *ui;
    QVBoxLayout *button_layout;
    GeneralSearch *general_search;

    QString origin_window_title;

    QMap<const General *, int> *all_generals;

    void resetButtons();
    void addLines(const General *general, const Skill *skill);
    void addDeathLine(const General *general);
    void addWinLineOfCaoCao();
    void addCopyAction(QCommandLinkButton *button);
    bool hasSkin(const General *general) const;
    QString getCvInfo(const QString &generalName);
    QString getIllustratorInfo(const QString &generalName);

public slots:
    void startSearch(bool include_hidden, const QString &nickname, const QString &name, const QStringList &genders,
        const QStringList &kingdoms, int lower, int upper, const QStringList &packages);

private slots:
    void playAudioEffect();
    void copyLines();
    void showNextSkin();
    void fillAllGenerals();
    void on_tableView_clicked(const QModelIndex &index);
    void playDeathAudio();
};

#endif // GENERALOVERVIEW_H

