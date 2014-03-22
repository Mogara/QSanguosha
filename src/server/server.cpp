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
#include "server.h"
#include "settings.h"
#include "room.h"
#include "engine.h"
#include "nativesocket.h"
#include "scenario.h"
#include "choosegeneraldialog.h"
#include "customassigndialog.h"
#include "miniscenarios.h"
#include "SkinBank.h"

#include <QMessageBox>
#include <QFormLayout>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QApplication>
#include <QHostInfo>
#include <QAction>

static QLayout *HLay(QWidget *left, QWidget *right) {
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(left);
    layout->addWidget(right);
    return layout;
}

ServerDialog::ServerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Start server"));

    QTabWidget *tab_widget = new QTabWidget;
    tab_widget->addTab(createBasicTab(), tr("Basic"));
    tab_widget->addTab(createPackageTab(), tr("Game Pacakge Selection"));
    tab_widget->addTab(createAdvancedTab(), tr("Advanced"));
    tab_widget->addTab(createConversionTab(), tr("Conversion Selection"));
    tab_widget->addTab(createMiscTab(), tr("Miscellaneous"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tab_widget);
    layout->addLayout(createButtonLayout());
    setLayout(layout);

    setMinimumWidth(300);
}

QWidget *ServerDialog::createBasicTab() {
    server_name_edit = new QLineEdit;
    server_name_edit->setText(Config.ServerName);

    timeout_spinbox = new QSpinBox;
    timeout_spinbox->setMinimum(5);
    timeout_spinbox->setMaximum(60);
    timeout_spinbox->setValue(Config.OperationTimeout);
    timeout_spinbox->setSuffix(tr(" seconds"));
    nolimit_checkbox = new QCheckBox(tr("No limit"));
    nolimit_checkbox->setChecked(Config.OperationNoLimit);
    connect(nolimit_checkbox, SIGNAL(toggled(bool)), timeout_spinbox, SLOT(setDisabled(bool)));

    QFormLayout *form_layout = new QFormLayout;
    form_layout->addRow(tr("Server name"), server_name_edit);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->addWidget(timeout_spinbox);
    lay->addWidget(nolimit_checkbox);
    form_layout->addRow(tr("Operation timeout"), lay);
    form_layout->addRow(createGameModeBox());

    QWidget *widget = new QWidget;
    widget->setLayout(form_layout);
    return widget;
}

QWidget *ServerDialog::createPackageTab() {
    disable_lua_checkbox = new QCheckBox(tr("Disable Lua"));
    disable_lua_checkbox->setChecked(Config.DisableLua);
    disable_lua_checkbox->setToolTip(tr("<font color=%1>The setting takes effect after reboot</font>").arg(Config.SkillDescriptionInToolTipColor.name()));

    extension_group = new QButtonGroup;
    extension_group->setExclusive(false);

    QStringList extensions = Sanguosha->getExtensions();
    QSet<QString> ban_packages = Config.BanPackages.toSet();

    QGroupBox *box1 = new QGroupBox(tr("General package"));
    QGroupBox *box2 = new QGroupBox(tr("Card package"));

    QGridLayout *layout1 = new QGridLayout;
    QGridLayout *layout2 = new QGridLayout;
    box1->setLayout(layout1);
    box2->setLayout(layout2);

    int i = 0, j = 0;
    int row = 0, column = 0;
    foreach (QString extension, extensions) {
        const Package *package = Sanguosha->findChild<const Package *>(extension);
        if (package == NULL)
            continue;

        QCheckBox *checkbox = new QCheckBox;
        checkbox->setObjectName(extension);
        checkbox->setText(Sanguosha->translate(extension));
        checkbox->setChecked(!ban_packages.contains(extension));

        extension_group->addButton(checkbox);

        switch (package->getType()) {
        case Package::GeneralPack: {
                row = i / 5;
                column = i % 5;
                i++;

                layout1->addWidget(checkbox, row, column + 1);
                break;
            }
        case Package::CardPack: {
                row = j / 5;
                column = j % 5;
                j++;

                layout2->addWidget(checkbox, row, column + 1);
                break;
            }
        default:
                break;
        }
    }

    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(disable_lua_checkbox);
    layout->addWidget(box1);
    layout->addWidget(box2);

    widget->setLayout(layout);
    return widget;
}

QWidget *ServerDialog::createAdvancedTab() {
    QVBoxLayout *layout = new QVBoxLayout;

    forbid_same_ip_checkbox = new QCheckBox(tr("Forbid same IP with multiple connection"));
    forbid_same_ip_checkbox->setChecked(Config.ForbidSIMC);

    disable_chat_checkbox = new QCheckBox(tr("Disable chat"));
    disable_chat_checkbox->setChecked(Config.DisableChat);

    random_seat_checkbox = new QCheckBox(tr("Arrange the seats randomly"));
    random_seat_checkbox->setChecked(Config.RandomSeat);

    enable_cheat_checkbox = new QCheckBox(tr("Enable cheat"));
    enable_cheat_checkbox->setToolTip(tr("<font color=%1>This option enables the cheat menu</font>").arg(Config.SkillDescriptionInToolTipColor.name()));
    enable_cheat_checkbox->setChecked(Config.EnableCheat);

    free_choose_checkbox = new QCheckBox(tr("Choose generals and cards freely"));
    free_choose_checkbox->setChecked(Config.FreeChoose);
    free_choose_checkbox->setVisible(Config.EnableCheat);

    connect(enable_cheat_checkbox, SIGNAL(toggled(bool)), free_choose_checkbox, SLOT(setVisible(bool)));

    pile_swapping_label = new QLabel(tr("Pile-swapping limitation"));
    pile_swapping_label->setToolTip(tr("<font color=%1>-1 means no limitations</font>").arg(Config.SkillDescriptionInToolTipColor.name()));
    pile_swapping_spinbox = new QSpinBox;
    pile_swapping_spinbox->setRange(-1, 15);
    pile_swapping_spinbox->setValue(Config.value("PileSwappingLimitation", 5).toInt());

    hegemony_maxchoice_label = new QLabel(tr("Upperlimit for hegemony"));
    hegemony_maxchoice_spinbox = new QSpinBox;
    hegemony_maxchoice_spinbox->setRange(5, 7); //wait for a new extension
    hegemony_maxchoice_spinbox->setValue(Config.value("HegemonyMaxChoice", 7).toInt());

    address_edit = new QLineEdit;
    address_edit->setText(Config.Address);
#if QT_VERSION >= 0x040700
    address_edit->setPlaceholderText(tr("Public IP or domain"));
#endif

    QPushButton *detect_button = new QPushButton(tr("Detect my WAN IP"));
    connect(detect_button, SIGNAL(clicked()), this, SLOT(onDetectButtonClicked()));

    port_edit = new QLineEdit;
    port_edit->setText(QString::number(Config.ServerPort));
    port_edit->setValidator(new QIntValidator(1, 9999, port_edit));

    layout->addLayout(HLay(forbid_same_ip_checkbox, disable_chat_checkbox));
    layout->addWidget(random_seat_checkbox);
    layout->addWidget(enable_cheat_checkbox);
    layout->addWidget(free_choose_checkbox);
    layout->addLayout(HLay(pile_swapping_label, pile_swapping_spinbox));
    layout->addLayout(HLay(hegemony_maxchoice_label, hegemony_maxchoice_spinbox));
    layout->addLayout(HLay(new QLabel(tr("Address")), address_edit));
    layout->addWidget(detect_button);
    layout->addLayout(HLay(new QLabel(tr("Port")), port_edit));
    layout->addStretch();

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    return widget;
}

QWidget *ServerDialog::createConversionTab() {
    conversions_group = new QButtonGroup;
    conversions_group->setExclusive(false);

    QGroupBox *formation_conversions = new QGroupBox(tr("Formation Conversions"));
    QGroupBox *momentum_conversions = new QGroupBox(tr("Momentum Conversions"));

    QVBoxLayout *formation_layout = new QVBoxLayout;
    QVBoxLayout *momentum_layout = new QVBoxLayout;
    formation_conversions->setLayout(formation_layout);
    momentum_conversions->setLayout(momentum_layout);

    const bool enable_lord_liubei = Config.value("GeneralConversions").toStringList().contains("liubei");
    convert_liubei_to_lord = new QCheckBox(tr("Convert Liu Bei to Lord Liu Bei"));
    convert_liubei_to_lord->setChecked(enable_lord_liubei);

    convert_ds_to_dp = new QCheckBox(tr("Convert DoubleSword to DragonPhoenix"));
    convert_ds_to_dp->setChecked(Config.value("CardConversions").toStringList().contains("DragonPhoenix"));
    convert_ds_to_dp->setDisabled(enable_lord_liubei);

    connect(convert_liubei_to_lord, SIGNAL(toggled(bool)), convert_ds_to_dp, SLOT(setChecked(bool)));
    connect(convert_liubei_to_lord, SIGNAL(toggled(bool)), convert_ds_to_dp, SLOT(setDisabled(bool)));

    conversions_group->addButton(convert_liubei_to_lord);
    conversions_group->addButton(convert_ds_to_dp);
    formation_layout->addWidget(convert_liubei_to_lord);
    formation_layout->addWidget(convert_ds_to_dp);

    const bool enable_lord_zhangjiao = Config.value("GeneralConversions").toStringList().contains("zhangjiao");
    convert_zhangjiao_to_lord = new QCheckBox(tr("Convert Zhang Jiao to Lord Zhang Jiao"));
    convert_zhangjiao_to_lord->setChecked(enable_lord_zhangjiao);
/*
    add_peace_spell = new QCheckBox(tr("Add Peace Spell"));
    add_peace_spell->setChecked(Config.value("CardConversions").toStringList().contains("+109"));
    add_peace_spell->setDisabled(enable_lord_zhangjiao);

    connect(convert_zhangjiao_to_lord, SIGNAL(toggled(bool)), add_peace_spell, SLOT(setChecked(bool)));
    connect(convert_zhangjiao_to_lord, SIGNAL(toggled(bool)), add_peace_spell, SLOT(setDisabled(bool)));
*/
    conversions_group->addButton(convert_zhangjiao_to_lord);
    //conversions_group->addButton(add_peace_spell);
    momentum_layout->addWidget(convert_zhangjiao_to_lord);
    //momentum_layout->addWidget(add_peace_spell);

    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(formation_conversions);
    layout->addWidget(momentum_conversions);
    widget->setLayout(layout);
    return widget;
}

QWidget *ServerDialog::createMiscTab() {
    game_start_spinbox = new QSpinBox;
    game_start_spinbox->setRange(0, 10);
    game_start_spinbox->setValue(Config.CountDownSeconds);
    game_start_spinbox->setSuffix(tr(" seconds"));

    nullification_spinbox = new QSpinBox;
    nullification_spinbox->setRange(5, 15);
    nullification_spinbox->setValue(Config.NullificationCountDown);
    nullification_spinbox->setSuffix(tr(" seconds"));

    minimize_dialog_checkbox = new QCheckBox(tr("Minimize the dialog when server runs"));
    minimize_dialog_checkbox->setChecked(Config.EnableMinimizeDialog);

    surrender_at_death_checkbox = new QCheckBox(tr("Surrender at the time of Death"));
    surrender_at_death_checkbox->setChecked(Config.SurrenderAtDeath);

    luck_card_label = new QLabel(tr("Upperlimit for use time of luck card"));
    luck_card_spinbox = new QSpinBox;
    luck_card_spinbox->setRange(0, 3);
    luck_card_spinbox->setValue(Config.LuckCardLimitation);

    QGroupBox *ai_groupbox = new QGroupBox(tr("Artificial intelligence"));
    ai_groupbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout *layout = new QVBoxLayout;

    ai_enable_checkbox = new QCheckBox(tr("Enable AI"));
    ai_enable_checkbox->setChecked(true);
    ai_enable_checkbox->setEnabled(false); // Force to enable AI for disabling it causes crashes!!

    ai_delay_spinbox = new QSpinBox;
    ai_delay_spinbox->setMinimum(0);
    ai_delay_spinbox->setMaximum(5000);
    ai_delay_spinbox->setValue(Config.OriginAIDelay);
    ai_delay_spinbox->setSuffix(tr(" millisecond"));

    ai_delay_altered_checkbox = new QCheckBox(tr("Alter AI Delay After Death"));
    ai_delay_altered_checkbox->setChecked(Config.AlterAIDelayAD);

    ai_delay_ad_spinbox = new QSpinBox;
    ai_delay_ad_spinbox->setMinimum(0);
    ai_delay_ad_spinbox->setMaximum(5000);
    ai_delay_ad_spinbox->setValue(Config.AIDelayAD);
    ai_delay_ad_spinbox->setSuffix(tr(" millisecond"));
    ai_delay_ad_spinbox->setEnabled(ai_delay_altered_checkbox->isChecked());
    connect(ai_delay_altered_checkbox, SIGNAL(toggled(bool)), ai_delay_ad_spinbox, SLOT(setEnabled(bool)));

    layout->addWidget(ai_enable_checkbox);
    layout->addLayout(HLay(new QLabel(tr("AI delay")), ai_delay_spinbox));
    layout->addWidget(ai_delay_altered_checkbox);
    layout->addLayout(HLay(new QLabel(tr("AI delay After Death")), ai_delay_ad_spinbox));

    ai_groupbox->setLayout(layout);

    QVBoxLayout *tablayout = new QVBoxLayout;
    tablayout->addLayout(HLay(new QLabel(tr("Game start count down")), game_start_spinbox));
    tablayout->addLayout(HLay(new QLabel(tr("Nullification count down")), nullification_spinbox));
    tablayout->addLayout(HLay(minimize_dialog_checkbox, surrender_at_death_checkbox));
    tablayout->addLayout(HLay(luck_card_label, luck_card_spinbox));
    tablayout->addWidget(luck_card_spinbox);
    tablayout->addWidget(ai_groupbox);
    tablayout->addStretch();

    QWidget *widget = new QWidget;
    widget->setLayout(tablayout);
    return widget;
}

void ServerDialog::ensureEnableAI() {
    ai_enable_checkbox->setChecked(true);
}

QGroupBox *ServerDialog::createGameModeBox() {
    QGroupBox *mode_box = new QGroupBox(tr("Game mode"));
    mode_group = new QButtonGroup;

    QObjectList item_list;

    // normal modes
    QMap<QString, QString> modes = Sanguosha->getAvailableModes();
    QMapIterator<QString, QString> itor(modes);
    while (itor.hasNext()) {
        itor.next();

        QRadioButton *button = new QRadioButton(itor.value());
        button->setObjectName(itor.key());
        mode_group->addButton(button);

        item_list << button;

        if (itor.key() == Config.GameMode)
            button->setChecked(true);
    }
    // ============

    QVBoxLayout *left = new QVBoxLayout;
    QVBoxLayout *right = new QVBoxLayout;

    for (int i = 0; i < item_list.length(); i++) {
        QObject *item = item_list.at(i);

        QVBoxLayout *side = i <= item_list.length() / 2 ? left : right;

        if (item->isWidgetType()) {
            QWidget *widget = qobject_cast<QWidget *>(item);
            side->addWidget(widget);
        } else {
            QLayout *item_layout = qobject_cast<QLayout *>(item);
            side->addLayout(item_layout);
        }
        if (i == item_list.length() / 2)
            side->addStretch();
    }

    right->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addLayout(left);
    layout->addLayout(right);

    mode_box->setLayout(layout);

    return mode_box;
}

QLayout *ServerDialog::createButtonLayout() {
    QHBoxLayout *button_layout = new QHBoxLayout;
    button_layout->addStretch();

    QPushButton *ok_button = new QPushButton(tr("OK"));
    QPushButton *cancel_button = new QPushButton(tr("Cancel"));

    button_layout->addWidget(ok_button);
    button_layout->addWidget(cancel_button);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(onOkButtonClicked()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

    return button_layout;
}

void ServerDialog::onDetectButtonClicked() {
    QHostInfo vHostInfo = QHostInfo::fromName(QHostInfo::localHostName());
    QList<QHostAddress> vAddressList = vHostInfo.addresses();
    foreach (QHostAddress address, vAddressList) {
        if (!address.isNull() && address != QHostAddress::LocalHost
            && address.protocol() ==  QAbstractSocket::IPv4Protocol) {
            address_edit->setText(address.toString());
            return;
        }
    }
}

void ServerDialog::onOkButtonClicked() {
    accept();
}

void ServerDialog::doCustomAssign() {
    CustomAssignDialog *dialog = new CustomAssignDialog(this);

    connect(dialog, SIGNAL(scenario_changed()), this, SLOT(setMiniCheckBox()));
    dialog->exec();
}

bool ServerDialog::config() {
    exec();

    if (result() != Accepted)
        return false;

    Config.ServerName = server_name_edit->text();
    Config.OperationTimeout = timeout_spinbox->value();
    Config.OperationNoLimit = nolimit_checkbox->isChecked();
    Config.RandomSeat = random_seat_checkbox->isChecked();
    Config.EnableCheat = enable_cheat_checkbox->isChecked();
    Config.FreeChoose = Config.EnableCheat && free_choose_checkbox->isChecked();
    Config.ForbidSIMC = forbid_same_ip_checkbox->isChecked();
    Config.DisableChat = disable_chat_checkbox->isChecked();
    Config.Address = address_edit->text();
    Config.CountDownSeconds = game_start_spinbox->value();
    Config.NullificationCountDown = nullification_spinbox->value();
    Config.EnableMinimizeDialog = minimize_dialog_checkbox->isChecked();
    Config.EnableAI = ai_enable_checkbox->isChecked();
    Config.OriginAIDelay = ai_delay_spinbox->value();
    Config.AIDelay = Config.OriginAIDelay;
    Config.AIDelayAD = ai_delay_ad_spinbox->value();
    Config.AlterAIDelayAD = ai_delay_altered_checkbox->isChecked();
    Config.ServerPort = port_edit->text().toInt();
    Config.DisableLua = disable_lua_checkbox->isChecked();
    Config.SurrenderAtDeath = surrender_at_death_checkbox->isChecked();
    Config.LuckCardLimitation = luck_card_spinbox->value();

    // game mode
    QString objname = mode_group->checkedButton()->objectName();
    Config.GameMode = objname;

    Config.setValue("ServerName", Config.ServerName);
    Config.setValue("GameMode", Config.GameMode);
    Config.setValue("OperationTimeout", Config.OperationTimeout);
    Config.setValue("OperationNoLimit", Config.OperationNoLimit);
    Config.setValue("RandomSeat", Config.RandomSeat);
    Config.setValue("EnableCheat", Config.EnableCheat);
    Config.setValue("FreeChoose", Config.FreeChoose);
    Config.setValue("PileSwappingLimitation", pile_swapping_spinbox->value());
    Config.setValue("ForbidSIMC", Config.ForbidSIMC);
    Config.setValue("DisableChat", Config.DisableChat);
    Config.setValue("HegemonyMaxChoice", hegemony_maxchoice_spinbox->value());
    Config.setValue("CountDownSeconds", game_start_spinbox->value());
    Config.setValue("NullificationCountDown", nullification_spinbox->value());
    Config.setValue("EnableMinimizeDialog", Config.EnableMinimizeDialog);
    Config.setValue("EnableAI", Config.EnableAI);
    Config.setValue("OriginAIDelay", Config.OriginAIDelay);
    Config.setValue("AlterAIDelayAD", ai_delay_altered_checkbox->isChecked());
    Config.setValue("AIDelayAD", Config.AIDelayAD);
    Config.setValue("SurrenderAtDeath", Config.SurrenderAtDeath);
    Config.setValue("LuckCardLimitation", Config.LuckCardLimitation);
    Config.setValue("ServerPort", Config.ServerPort);
    Config.setValue("Address", Config.Address);
    Config.setValue("DisableLua", disable_lua_checkbox->isChecked());

    QSet<QString> ban_packages;
    QList<QAbstractButton *> checkboxes = extension_group->buttons();
    foreach (QAbstractButton *checkbox, checkboxes) {
        if (!checkbox->isChecked()) {
            QString package_name = checkbox->objectName();
            Sanguosha->addBanPackage(package_name);
            ban_packages.insert(package_name);
        }
    }

    Config.BanPackages = ban_packages.toList();
    Config.setValue("BanPackages", Config.BanPackages);

    QStringList general_conversions;
    if (convert_liubei_to_lord->isChecked()) general_conversions << "liubei";
    if (convert_zhangjiao_to_lord->isChecked()) general_conversions << "zhangjiao";
    Config.setValue("GeneralConversions", general_conversions);

    QStringList card_conversions;
    if (convert_ds_to_dp->isChecked()) card_conversions << "DragonPhoenix";
    //if (add_peace_spell->isChecked()) card_conversions << "+109";
    Config.setValue("CardConversions", card_conversions);

    return true;
}

BanIPDialog::BanIPDialog(QWidget *parent, Server *theserver)
    : QDialog(parent), server(theserver){
/*
    if (Sanguosha->currentRoom() == NULL){
        QMessageBox::warning(this, tr("Warining!"), tr("Game is not started!"));
        return;
    }
*/
    QVBoxLayout *left_layout = new QVBoxLayout;
    QVBoxLayout *right_layout = new QVBoxLayout;

    left = new QListWidget;
    left->setSortingEnabled(false);
    right = new QListWidget;

    QPushButton *insert = new QPushButton(tr("Insert to banned IP list"));
    QPushButton *kick = new QPushButton(tr("Kick from server"));

    QPushButton *remove = new QPushButton(tr("Remove from banned IP list"));

    left_layout->addWidget(left);

    QHBoxLayout *left_button_layout = new QHBoxLayout;
    left_button_layout->addWidget(insert);
    left_button_layout->addWidget(kick);

    left_layout->addLayout(left_button_layout);

    right_layout->addWidget(right);
    right_layout->addWidget(remove);

    QPushButton *ok = new QPushButton(tr("OK"));
    QPushButton *cancel = new QPushButton(tr("Cancel"));

    QHBoxLayout *up_layout = new QHBoxLayout;
    up_layout->addLayout(left_layout);
    up_layout->addLayout(right_layout);

    QHBoxLayout *down_layout = new QHBoxLayout;
    down_layout->addWidget(ok);
    down_layout->addWidget(cancel);

    QVBoxLayout *total_layout = new QVBoxLayout;
    total_layout->addLayout(up_layout);
    total_layout->addLayout(down_layout);

    setLayout(total_layout);
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    connect(this, SIGNAL(accepted()), this, SLOT(save()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(insert, SIGNAL(clicked()), this, SLOT(insertClicked()));
    connect(remove, SIGNAL(clicked()), this, SLOT(removeClicked()));
    connect(kick, SIGNAL(clicked()), this, SLOT(kickClicked()));

    if (server)
        loadIPList();
    else
        QMessageBox::warning(this, tr("Warning!"), tr("There is no server running!"));

    loadBannedList();
}

void BanIPDialog::loadIPList(){
    foreach (Room *room, server->rooms){
        foreach (ServerPlayer *p, room->getPlayers()){
            if (p->getState() != "offline" && p->getState() != "robot") {
                sp_list << p;
            }
        }
    }

    left->clear();

    foreach (ServerPlayer *p, sp_list){
        QString parsed_string = QString("%1::%2").arg(p->screenName(), p->getIp());
        left->addItem(parsed_string);
    }
}

void BanIPDialog::loadBannedList() {
    QStringList banned = Config.value("BannedIP", QStringList()).toStringList();

    right->clear();
    right->addItems(banned);
}

void BanIPDialog::insertClicked() {
    int row = left->currentRow();
    if (row != -1){
        QString ip = left->currentItem()->text().split("::").last();

        if (ip.startsWith("127."))
            QMessageBox::warning(this, tr("Warning!"), tr("This is your local Loopback Address and can't be banned!"));

        if (right->findItems(ip, Qt::MatchFlags(Qt::MatchExactly)).isEmpty())
            right->addItem(ip);
    }
}

void BanIPDialog::removeClicked(){
    int row = right->currentRow();
    if (row != -1)
        delete right->takeItem(row);
}

void BanIPDialog::kickClicked(){
    int row = left->currentRow();
    if (row != -1){
        ServerPlayer *p = sp_list[row];
        QStringList split_data = left->currentItem()->text().split("::");
        QString ip = split_data.takeLast();
        QString screenName = split_data.join("::");
        if (p->screenName() == screenName && p->getIp() == ip){
            //procedure kick
            p->kick();
        }
    }
}

void BanIPDialog::save(){
    QSet<QString> ip_set;

    for (int i = 0; i < right->count(); i++)
        ip_set << right->item(i)->text();

    QStringList ips = ip_set.toList();
    Config.setValue("BannedIP", ips);
}

Server::Server(QObject *parent)
    : QObject(parent)
{
    server = new NativeServerSocket;
    server->setParent(this);

    //synchronize ServerInfo on the server side to avoid ambiguous usage of Config and ServerInfo
    ServerInfo.parse(Sanguosha->getSetupString());

    current = NULL;
    createNewRoom();

    connect(server, SIGNAL(new_connection(ClientSocket *)), this, SLOT(processNewConnection(ClientSocket *)));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
}

void Server::broadcast(const QString &msg) {
    QString to_sent = msg.toUtf8().toBase64();
    to_sent = ".:" + to_sent;
    foreach (Room *room, rooms)
        room->broadcastInvoke("speak", to_sent);
}

bool Server::listen() {
    return server->listen();
}

void Server::daemonize() {
    server->daemonize();
}

Room *Server::createNewRoom() {
    Room *new_room = new Room(this, Config.GameMode);
    current = new_room;
    rooms.insert(current);

    connect(current, SIGNAL(room_message(QString)), this, SIGNAL(server_message(QString)));
    connect(current, SIGNAL(game_over(QString)), this, SLOT(gameOver()));

    return current;
}

void Server::processNewConnection(ClientSocket *socket) {
    QString addr = socket->peerAddress();
    if (Config.ForbidSIMC) {
        if (addresses.contains(addr)) {
            socket->disconnectFromHost();
            emit server_message(tr("Forbid the connection of address %1").arg(addr));
            return;
        }
        else
            addresses.insert(addr);
    }

    if (Config.value("BannedIP").toStringList().contains(addr)){
        socket->disconnectFromHost();
        emit server_message(tr("Forbid the connection of address %1").arg(addr));
        return;
    }

    connect(socket, SIGNAL(disconnected()), this, SLOT(cleanup()));
    socket->send("checkVersion " + Sanguosha->getVersion());
    socket->send("setup " + Sanguosha->getSetupString());
    emit server_message(tr("%1 connected").arg(socket->peerName()));

    connect(socket, SIGNAL(message_got(const char *)), this, SLOT(processRequest(const char *)));
}

static inline QString ConvertFromBase64(const QString &base64) {
    QByteArray data = QByteArray::fromBase64(base64.toAscii());
    return QString::fromUtf8(data);
}

void Server::processRequest(const char *request) {
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());
    socket->disconnect(this, SLOT(processRequest(const char *)));

    QRegExp rx("(signupr?) (.+):(.+)(:.+)?\n");
    if (!rx.exactMatch(request)) {
        emit server_message(tr("Invalid signup string: %1").arg(request));
        socket->send("warn INVALID_FORMAT");
        socket->disconnectFromHost();
        return;
    }

    QStringList texts = rx.capturedTexts();
    QString command = texts.at(1);
    QString screen_name = ConvertFromBase64(texts.at(2));
    QString avatar = texts.at(3);

    if (command == "signupr") {
        foreach (QString objname, name2objname.values(screen_name)) {
            ServerPlayer *player = players.value(objname);
            if (player && player->getState() == "offline" && !player->getRoom()->isFinished()) {
                player->getRoom()->reconnect(player, socket);
                return;
            }
        }
    }

    if (current == NULL || current->isFull() || current->isFinished())
        createNewRoom();

    ServerPlayer *player = current->addSocket(socket);
    current->signup(player, screen_name, avatar, false);
}

void Server::cleanup() {
    const ClientSocket *socket = qobject_cast<const ClientSocket *>(sender());
    if (Config.ForbidSIMC)
        addresses.remove(socket->peerAddress());
}

void Server::signupPlayer(ServerPlayer *player) {
    name2objname.insert(player->screenName(), player->objectName());
    players.insert(player->objectName(), player);
}

void Server::gameOver() {
    Room *room = qobject_cast<Room *>(sender());
    rooms.remove(room);

    foreach (ServerPlayer *player, room->findChildren<ServerPlayer *>()) {
        name2objname.remove(player->screenName(), player->objectName());
        players.remove(player->objectName());
    }
}
