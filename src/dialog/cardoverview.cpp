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

#include "cardoverview.h"
#include "ui_cardoverview.h"
#include "engine.h"
#include "stylehelper.h"
#include "clientstruct.h"
#include "settings.h"
#include "client.h"
#include "clientplayer.h"
#include "skinbank.h"

#include <QScrollBar>
#include <QMessageBox>
#include <QFile>

static CardOverview *Overview;

CardOverview *CardOverview::getInstance(QWidget *main_window) {
    if (Overview == NULL)
        Overview = new CardOverview(main_window);

    return Overview;
}

CardOverview::CardOverview(QWidget *parent)
    : FlatDialog(parent, false), ui(new Ui::CardOverview)
{
    ui->setupUi(this);

    connect(this, &CardOverview::windowTitleChanged, ui->titleLabel, &QLabel::setText);

    ui->tableWidget->setColumnWidth(0, 80);
    ui->tableWidget->setColumnWidth(1, 60);
    ui->tableWidget->setColumnWidth(2, 30);
    ui->tableWidget->setColumnWidth(3, 60);
    ui->tableWidget->setColumnWidth(4, 70);

    connect(ui->getCardButton, &QPushButton::clicked, this, &CardOverview::askCard);
    connect(ui->closeButton, &QPushButton::clicked, this, &CardOverview::reject);

    ui->cardDescriptionBox->setProperty("description", true);
    ui->malePlayButton->hide();
    ui->femalePlayButton->hide();
    ui->playAudioEffectButton->hide();

    const QString style = StyleHelper::styleSheetOfScrollBar();
    ui->tableWidget->verticalScrollBar()->setStyleSheet(style);
    ui->cardDescriptionBox->verticalScrollBar()->setStyleSheet(style);
}

void CardOverview::loadFromAll() {
    int n = Sanguosha->getCardCount();
    ui->tableWidget->setRowCount(n);
    for (int i = 0; i < n; i++) {
        const Card *card = Sanguosha->getEngineCard(i);
        addCard(i, card);
    }

    if (n > 0) {
        ui->tableWidget->setCurrentItem(ui->tableWidget->item(0, 0));

        const Card *card = Sanguosha->getEngineCard(0);
        if (card->getTypeId() == Card::TypeEquip) {
            ui->playAudioEffectButton->show();
            ui->malePlayButton->hide();
            ui->femalePlayButton->hide();
        }
        else {
            ui->playAudioEffectButton->hide();
            ui->malePlayButton->show();
            ui->femalePlayButton->show();
        }
    }
}

void CardOverview::loadFromList(const QList<const Card *> &list) {
    int n = list.length();
    ui->tableWidget->setRowCount(n);
    for (int i = 0; i < n; i++)
        addCard(i, list.at(i));

    if (n > 0) {
        ui->tableWidget->setCurrentItem(ui->tableWidget->item(0, 0));

        const Card *card = list.first();
        if (card->getTypeId() == Card::TypeEquip) {
            ui->playAudioEffectButton->show();
            ui->malePlayButton->hide();
            ui->femalePlayButton->hide();
        }
        else {
            ui->playAudioEffectButton->hide();
            ui->malePlayButton->show();
            ui->femalePlayButton->show();
        }
    }
}

void CardOverview::addCard(int i, const Card *card) {
    QString name = Sanguosha->translate(card->objectName());
    QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(card->getSuitString()));
    QString suit_str = Sanguosha->translate(card->getSuitString());
    QString point = card->getNumberString();
    QString type = Sanguosha->translate(card->getType());
    QString subtype = Sanguosha->translate(card->getSubtype());
    QString package = Sanguosha->translate(card->getPackage());

    QTableWidgetItem *name_item = new QTableWidgetItem(name);
    name_item->setData(Qt::UserRole, card->getId());

    ui->tableWidget->setItem(i, 0, name_item);
    ui->tableWidget->setItem(i, 1, new QTableWidgetItem(suit_icon, suit_str));
    ui->tableWidget->setItem(i, 2, new QTableWidgetItem(point));
    ui->tableWidget->setItem(i, 3, new QTableWidgetItem(type));
    ui->tableWidget->setItem(i, 4, new QTableWidgetItem(subtype));

    QTableWidgetItem *package_item = new QTableWidgetItem(package);
    if (Config.value("LuaPackages", QString()).toString().split("+").contains(card->getPackage())) {
        package_item->setBackgroundColor(QColor(0x66, 0xCC, 0xFF));
        package_item->setToolTip(tr("<font color=%1>This is an Lua extension</font>").arg(Config.SkillDescriptionInToolTipColor.name()));
    }
    ui->tableWidget->setItem(i, 5, package_item);
}

CardOverview::~CardOverview() {
    delete ui;
}

void CardOverview::on_tableWidget_itemSelectionChanged() {
    int row = ui->tableWidget->currentRow();
    int card_id = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toInt();
    const Card *card = Sanguosha->getEngineCard(card_id);
    QString pixmap_path = QString("image/big-card/%1.png").arg(card->objectName());
    ui->cardLabel->setPixmap(pixmap_path);

    ui->cardDescriptionBox->setText(card->getDescription(false));

    if (card->getTypeId() == Card::TypeEquip) {
        ui->playAudioEffectButton->show();
        ui->malePlayButton->hide();
        ui->femalePlayButton->hide();
    } else {
        ui->playAudioEffectButton->hide();
        ui->malePlayButton->show();
        ui->femalePlayButton->show();
    }
}

void CardOverview::askCard() {
    if (!ServerInfo.EnableCheat || !ClientInstance)
        return;

    int row = ui->tableWidget->currentRow();
    if (row >= 0) {
        int card_id = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toInt();
        if (!ClientInstance->getAvailableCards().contains(card_id)) {
            QMessageBox::warning(this, tr("Warning"), tr("These packages don't contain this card"));
            return;
        }
        ClientInstance->requestCheatGetOneCard(card_id);
    }
}

void CardOverview::on_tableWidget_itemDoubleClicked(QTableWidgetItem *) {
    if (Self) askCard();
}

void CardOverview::on_malePlayButton_clicked() {
    int row = ui->tableWidget->currentRow();
    if (row >= 0) {
        int card_id = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toInt();
        const Card *card = Sanguosha->getEngineCard(card_id);
        Sanguosha->playAudioEffect(G_ROOM_SKIN.getPlayerAudioEffectPath(card->objectName(), true));
    }
}

void CardOverview::on_femalePlayButton_clicked() {
    int row = ui->tableWidget->currentRow();
    if (row >= 0) {
        int card_id = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toInt();
        const Card *card = Sanguosha->getEngineCard(card_id);
        Sanguosha->playAudioEffect(G_ROOM_SKIN.getPlayerAudioEffectPath(card->objectName(), false));
    }
}

void CardOverview::on_playAudioEffectButton_clicked() {
    int row = ui->tableWidget->currentRow();
    if (row >= 0) {
        int card_id = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toInt();
        const Card *card = Sanguosha->getEngineCard(card_id);
        if (card->getTypeId() == Card::TypeEquip) {
            QString effectName = card->getEffectName();
            if (effectName == "vscrossbow")
                effectName = "crossbow";
            QString fileName = G_ROOM_SKIN.getPlayerAudioEffectPath(effectName, QString("equip"), -1);
            if (!QFile::exists(fileName))
                fileName = G_ROOM_SKIN.getPlayerAudioEffectPath(card->getCommonEffectName(), QString("common"), -1);
            Sanguosha->playAudioEffect(fileName);
        }
    }
}

void CardOverview::showEvent(QShowEvent *)
{
    if (ServerInfo.EnableCheat && ClientInstance) {
        ui->getCardButton->show();
    } else {
        ui->getCardButton->hide();
    }
}
