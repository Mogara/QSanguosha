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

#include "connectiondialog.h"
#include "ui_connectiondialog.h"
#include "settings.h"
#include "engine.h"
#include "StyleHelper.h"
#include "UdpDetectorDialog.h"
#include "AvatarModel.h"
#include "SkinBank.h"

#include <QMessageBox>
#include <QRadioButton>
#include <QBoxLayout>
#include <QScrollBar>

static const int ShrinkWidth = 317;
static const int ExpandWidth = 619;

ConnectionDialog::ConnectionDialog(QWidget *parent)
    : FlatDialog(parent, false), ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);

    ui->nameLineEdit->setText(Config.UserName.left(8));

    ui->hostComboBox->addItems(Config.HistoryIPs);
    ui->hostComboBox->lineEdit()->setText(Config.HostAddress);

    ui->connectButton->setFocus();

    ui->avatarPixmap->setPixmap(G_ROOM_SKIN.getGeneralPixmap(Config.UserAvatar,
        QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE));

    ui->reconnectionCheckBox->setChecked(Config.value("EnableReconnection", false).toBool());

    connect(this, SIGNAL(windowTitleChanged(QString)), ui->title, SLOT(setText(QString)));

    QScrollBar *bar = ui->avatarList->verticalScrollBar();
    bar->setStyleSheet(StyleHelper::styleSheetOfScrollBar());

    resize(ShrinkWidth, height());

    ui->avatarList->hide();
}

ConnectionDialog::~ConnectionDialog() {
    delete ui;
}

void ConnectionDialog::hideAvatarList() {
    if (!ui->avatarList->isVisible()) return;
    ui->avatarList->hide();
}

void ConnectionDialog::showAvatarList() {
    if (ui->avatarList->isVisible()) return;

    if (ui->avatarList->model() == NULL) {
        QList<const General *> generals = Sanguosha->getGeneralList();
        QMutableListIterator<const General *> itor = generals;
        while (itor.hasNext()) {
            if (itor.next()->isTotallyHidden())
                itor.remove();
        }

        AvatarModel *model = new AvatarModel(generals);
        model->setParent(this);
        ui->avatarList->setModel(model);
    }
    ui->avatarList->show();
}

void ConnectionDialog::on_connectButton_clicked() {
    QString username = ui->nameLineEdit->text();

    if (username.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("The user name can not be empty!"));
        return;
    }

    Config.UserName = username;
    Config.HostAddress = ui->hostComboBox->lineEdit()->text();

    Config.setValue("UserName", Config.UserName);
    Config.setValue("HostAddress", Config.HostAddress);
    Config.setValue("EnableReconnection", ui->reconnectionCheckBox->isChecked());

    accept();
}

void ConnectionDialog::on_changeAvatarButton_clicked() {
    if (ui->avatarList->isVisible()) {
        QModelIndex index = ui->avatarList->currentIndex();
        if (index.isValid()) {
            on_avatarList_doubleClicked(index);
        } else {
            hideAvatarList();
            resize(ShrinkWidth, height());
        }
    } else {
        showAvatarList();
        //Avoid violating the constraints
        //setFixedWidth(ExpandWidth);
        resize(ExpandWidth, height());
    }
}

void ConnectionDialog::on_avatarList_doubleClicked(const QModelIndex &index) {
    QString general_name = ui->avatarList->model()->data(index, Qt::UserRole).toString();
    QPixmap avatar(G_ROOM_SKIN.getGeneralPixmap(general_name, QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE));
    ui->avatarPixmap->setPixmap(avatar);
    Config.UserAvatar = general_name;
    Config.setValue("UserAvatar", general_name);
    hideAvatarList();

    resize(ShrinkWidth, height());
}

void ConnectionDialog::on_clearHistoryButton_clicked() {
    ui->hostComboBox->clear();
    ui->hostComboBox->lineEdit()->clear();

    Config.HistoryIPs.clear();
    Config.remove("HistoryIPs");
}

void ConnectionDialog::on_detectLANButton_clicked() {
    UdpDetectorDialog *detector_dialog = new UdpDetectorDialog(this);
    connect(detector_dialog, SIGNAL(address_chosen(QString)),
        ui->hostComboBox->lineEdit(), SLOT(setText(QString)));

    detector_dialog->exec();
}
