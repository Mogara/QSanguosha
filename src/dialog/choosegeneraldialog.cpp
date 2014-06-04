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

#include "choosegeneraldialog.h"
#include "general.h"
#include "engine.h"
#include "client.h"
#include "settings.h"
#include "protocol.h"
#include "SkinBank.h"

#include <QSignalMapper>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QTimerEvent>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QTabWidget>
#include <QMessageBox>

using namespace QSanProtocol;

OptionButton::OptionButton(QString icon_path, const QString &caption, QWidget *parent)
: QToolButton(parent)
{
    QPixmap pixmap(icon_path);
    QIcon icon(pixmap);

    setIcon(icon);
    setIconSize(pixmap.size());

    if (!caption.isEmpty()) {
        setText(caption);
        setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        QFont font = Config.SmallFont;
        font.setPixelSize(Config.SmallFont.pixelSize() - 8);
        setFont(font);
    }
}

void OptionButton::mouseDoubleClickEvent(QMouseEvent *) {
    emit double_clicked();
}

ChooseGeneralDialog::ChooseGeneralDialog(const QStringList &general_names, QWidget *parent, bool view_only, const QString &title)
: QDialog(parent)
{
    m_freeChooseDialog = NULL;
    if (title.isEmpty())
        setWindowTitle(tr("Choose general"));
    else
        setWindowTitle(title);

    QString lord_name;

    QList<const General *> generals;
    foreach(QString general_name, general_names) {
        if (general_name.contains("(lord)")) {
            general_name.chop(6);
            lord_name = general_name;
            continue;
        }
        const General *general = Sanguosha->getGeneral(general_name);
        generals << general;
    }

    QSignalMapper *mapper = new QSignalMapper(this);
    QList<OptionButton *> buttons;
    bool tooManyManyGenerals = (generals.length() > G_COMMON_LAYOUT.m_chooseGeneralBoxNoIconThreshold);
    bool tooManyGenerals = (generals.length() > G_COMMON_LAYOUT.m_chooseGeneralBoxSwitchIconSizeThreshold);
    bool no_icon = false;
    QSize icon_size;
    QSanRoomSkin::GeneralIconSize icon_type;
    if (tooManyManyGenerals) {
        no_icon = true;
    }
    else {
        if (tooManyGenerals) {
            icon_type = QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE;
            icon_size = G_COMMON_LAYOUT.m_chooseGeneralBoxDenseIconSize;
        }
        else {
            icon_type = QSanRoomSkin::S_GENERAL_ICON_SIZE_CARD;
            icon_size = G_COMMON_LAYOUT.m_chooseGeneralBoxSparseIconSize;
        }
    }
    foreach(const General *general, generals) {
        QString caption = Sanguosha->translate(general->objectName());
        OptionButton *button = new OptionButton(QString(), caption);
        if (no_icon) {
            button->setIcon(QIcon("image/system/no-general-icon.png"));
            button->setIconSize(QSize(G_COMMON_LAYOUT.m_chooseGeneralBoxDenseIconSize.width(), 1));
        }
        else {
            button->setIcon(QIcon(G_ROOM_SKIN.getGeneralPixmap(general->objectName(), icon_type)));
            button->setIconSize(icon_size);
        }
        button->setToolTip(general->getSkillDescription(true));
        buttons << button;

        if (!view_only) {
            mapper->setMapping(button, general->objectName());
            connect(button, SIGNAL(double_clicked()), mapper, SLOT(map()));
            connect(button, SIGNAL(double_clicked()), this, SLOT(accept()));
        }
    }

    if (!view_only && generals.length() > 2) {
        int index = 0;
        foreach(const General *general, generals) {
            int party = 0;
            bool has_lord = false;
            foreach(const General *other, generals) {
                if (other->getKingdom() == general->getKingdom()) {
                    party++;
                    if (other != general && other->isLord())
                        has_lord = true;
                }
            }
            if (party < 2 || (!Self->getGeneral() && has_lord && party == 2))
                buttons.at(index)->setEnabled(false);

            if (!Self->isDead() && Self->getGeneral()) {
                if (Self->getGeneral()->getKingdom() == general->getKingdom()
                    && Self->getGeneralName() != general->objectName()
                    && !general->isLord() && !buttons.at(index)->isEnabled())
                    buttons.at(index)->setEnabled(true);
                if (Self->getGeneral()->getKingdom() != general->getKingdom()
                    || Self->getGeneralName() == general->objectName()
                    || general->isLord())
                    buttons.at(index)->setEnabled(false);
            }

            if (Self->isDead())
                buttons.at(index)->setEnabled(true);
            index++;
        }
    }

    QLayout *layout = NULL;
    const int columns = tooManyGenerals ? G_COMMON_LAYOUT.m_chooseGeneralBoxSwitchIconEachRowForTooManyGenerals :
        G_COMMON_LAYOUT.m_chooseGeneralBoxSwitchIconEachRow;
    if (generals.length() <= columns) {
        layout = new QHBoxLayout;

        foreach(OptionButton *button, buttons)
            layout->addWidget(button);
    }
    else {
        QGridLayout *grid_layout = new QGridLayout;
        QHBoxLayout *hlayout = new QHBoxLayout;
        QVBoxLayout *lord_layout = new QVBoxLayout;

        lord_layout->addStretch();
        hlayout->addLayout(lord_layout);

        int columns_x = qMin(columns, (buttons.length() + 1) / 2);
        for (int i = 0; i < buttons.length(); i++) {
            int row = i / columns_x;
            int column = i % columns_x;
            grid_layout->addWidget(buttons.at(i), row, column);
        }
        hlayout->addLayout(grid_layout);
        layout = hlayout;
    }

    QString default_name = generals.first()->objectName();
    for (int i = 0; i < buttons.size(); i++) {
        if (buttons.at(i)->isEnabled()) {
            default_name = generals.at(i)->objectName();
            break;
        }
    }

    if (!view_only) {
        mapper->setMapping(this, default_name);
        connect(this, SIGNAL(rejected()), mapper, SLOT(map()));

        connect(mapper, SIGNAL(mapped(QString)), ClientInstance, SLOT(onPlayerChooseGeneral(QString)));
    }

    QVBoxLayout *dialog_layout = new QVBoxLayout;
    dialog_layout->addLayout(layout);

    // progress bar & free choose button
    QHBoxLayout *last_layout = new QHBoxLayout;
    if (view_only || ServerInfo.OperationTimeout == 0) {
        progress_bar = NULL;
    }
    else {
        progress_bar = new QSanCommandProgressBar();
        progress_bar->setFixedWidth(300);
        progress_bar->setTimerEnabled(true);
        progress_bar->setCountdown(S_COMMAND_CHOOSE_GENERAL);
        progress_bar->show();
        last_layout->addWidget(progress_bar);
    }

    bool free_choose = ServerInfo.FreeChoose
        || ServerInfo.GameMode.startsWith("_mini_") || ServerInfo.GameMode == "custom_scenario";

    if (!view_only && free_choose) {
        QPushButton *free_choose_button = new QPushButton(tr("Free choose ..."));
        connect(free_choose_button, SIGNAL(clicked()), this, SLOT(freeChoose()));
        last_layout->addWidget(free_choose_button);
    }

    last_layout->addStretch();

    if (last_layout->count() != 0) {
        dialog_layout->addLayout(last_layout);
    }

    setLayout(dialog_layout);
}

void ChooseGeneralDialog::done(int result) {
    if (m_freeChooseDialog != NULL) {
        m_freeChooseDialog->reject();
        delete m_freeChooseDialog;
        m_freeChooseDialog = NULL;
    }
    QDialog::done(result);
}

void ChooseGeneralDialog::freeChoose() {
    QDialog *dialog = new FreeChooseDialog(this);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(general_chosen(QString)), ClientInstance, SLOT(onPlayerChooseGeneral(QString)));

    m_freeChooseDialog = dialog;

    dialog->exec();
}

FreeChooseDialog::FreeChooseDialog(QWidget *parent, ButtonGroupType type)
: QDialog(parent), type(type)
{
    setWindowTitle(tr("Free choose generals"));

    QTabWidget *tab_widget = new QTabWidget;

    group = new QButtonGroup(this);
    group->setExclusive(type == Exclusive);

    QList<const General *> all_generals = Sanguosha->findChildren<const General *>();
    QMap<QString, QList<const General *> > map;
    foreach(const General *general, all_generals) {
        if (general->isTotallyHidden())
            continue;

        map[general->getKingdom()] << general;
    }

    QStringList kingdoms = Sanguosha->getKingdoms();

    foreach(QString kingdom, kingdoms) {
        QList<const General *> generals = map[kingdom];

        if (!generals.isEmpty()) {
            QWidget *tab = createTab(generals);
            tab_widget->addTab(tab,
                QIcon(G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_KINGDOM_ICON, kingdom)),
                Sanguosha->translate(kingdom));
        }
    }

    QPushButton *ok_button = new QPushButton(tr("OK"));
    connect(ok_button, SIGNAL(clicked()), this, SLOT(chooseGeneral()));

    QPushButton *cancel_button = new QPushButton(tr("Cancel"));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *button_layout = new QHBoxLayout;
    button_layout->addStretch();
    button_layout->addWidget(ok_button);
    button_layout->addWidget(cancel_button);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tab_widget);
    layout->addLayout(button_layout);

    setLayout(layout);

    if (type == Exclusive)
        group->buttons().first()->click();
}

void FreeChooseDialog::chooseGeneral() {
    if (type == Pair) {
        QList<QAbstractButton *> buttons = group->buttons();
        QString first, second;
        foreach(QAbstractButton *button, buttons) {
            if (!button->isChecked())
                continue;

            if (first.isEmpty())
                first = button->objectName();
            else {
                second = button->objectName();
                emit pair_chosen(first, second);
                break;
            }
        }
        if (second.isEmpty()){
            QMessageBox::information(this, tr("Information"), tr("You can only select 2 generals in Pairs mode."));
            return;
        }
    }
    else if (type == Multi) {
        QStringList general_names;
        foreach(QAbstractButton *button, group->buttons()) {
            if (button->isChecked())
                general_names << button->objectName();
        }
        if (!general_names.isEmpty()) emit general_chosen(general_names.join("+"));
    }
    else {
        QAbstractButton *button = group->checkedButton();
        if (button) emit general_chosen(button->objectName());
    }

    accept();
}

QWidget *FreeChooseDialog::createTab(const QList<const General *> &generals) {
    QWidget *tab = new QWidget;

    QGridLayout *layout = new QGridLayout;
    layout->setOriginCorner(Qt::TopLeftCorner);
    QIcon lord_icon("image/system/roles/lord.png");

    const int columns = 4;

    for (int i = 0; i < generals.length(); i++) {
        const General *general = generals.at(i);
        QString general_name = general->objectName();
        QString text = QString("%1[%2]")
            .arg(Sanguosha->translate(general_name))
            .arg(Sanguosha->translate(general->getPackage()));

        QAbstractButton *button;
        if (type == Exclusive)
            button = new QRadioButton(text);
        else
            button = new QCheckBox(text);

        button->setObjectName(general_name);
        button->setToolTip(general->getSkillDescription(true));
        if (general->isLord())
            button->setIcon(lord_icon);

        group->addButton(button);

        int row = i / columns;
        int column = i % columns;
        layout->addWidget(button, row, column);
    }

    QVBoxLayout *layout2 = new QVBoxLayout;
    layout2->addStretch();

    QVBoxLayout *tablayout = new QVBoxLayout;
    tablayout->addLayout(layout);
    tablayout->addLayout(layout2);

    tab->setLayout(tablayout);

    if (type == Pair) {
        connect(group, SIGNAL(buttonClicked(QAbstractButton *)),
            this, SLOT(disableButtons(QAbstractButton *)));
    }

    return tab;
}

void FreeChooseDialog::disableButtons(QAbstractButton *) {
    QList<QAbstractButton *> buttons = group->buttons();
    QList<QAbstractButton *> checked;
    foreach(QAbstractButton *btn, buttons){
        if (btn->isChecked())
            checked << btn;
    }
    if (checked.length() == 2){
        foreach(QAbstractButton *btn, buttons){
            if (!btn->isChecked())
                btn->setEnabled(false);
            else
                btn->setEnabled(true);
        }
    }
    else if (checked.length() == 1){
        QString checked_kingdom = Sanguosha->getGeneral(checked.first()->objectName())->getKingdom();
        foreach(QAbstractButton *btn, buttons){
            QString btn_kingdom = Sanguosha->getGeneral(btn->objectName())->getKingdom();
            btn->setEnabled(checked_kingdom == btn_kingdom);
        }
    }
    else if (checked.length() == 0){
        foreach(QAbstractButton *btn, buttons){
            btn->setEnabled(true);
        }
    }
    else
        Q_ASSERT(false);
}

