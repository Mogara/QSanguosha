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

#include "server.h"
#include "settings.h"
#include "room.h"
#include "engine.h"
#include "nativesocket.h"
#include "scenario.h"
#include "FreeChooseDialog.h"
#include "customassigndialog.h"
#include "miniscenarios.h"
#include "SkinBank.h"
#include "banpair.h"

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

using namespace QSanProtocol;

static QLayout *HLay(QWidget *left, QWidget *right) {
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(left);
    layout->addWidget(right);
    return layout;
}

ServerDialog::ServerDialog(QWidget *parent)
    : FlatDialog(parent)
{
    setWindowTitle(tr("Start server"));

    QTabWidget *tab_widget = new QTabWidget;
    tab_widget->addTab(createBasicTab(), tr("Basic"));
    tab_widget->addTab(createPackageTab(), tr("Game Pacakge Selection"));
    tab_widget->addTab(createAdvancedTab(), tr("Advanced"));
    tab_widget->addTab(createConversionTab(), tr("Conversion Selection"));
    tab_widget->addTab(createMiscTab(), tr("Miscellaneous"));

    layout->addWidget(tab_widget);
    layout->addLayout(createButtonLayout());

    setMinimumSize(574, 380);
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
    timeout_spinbox->setDisabled(Config.OperationNoLimit);
    connect(nolimit_checkbox, SIGNAL(toggled(bool)), timeout_spinbox, SLOT(setDisabled(bool)));

    QPushButton *edit_button = new QPushButton(tr("Banlist ..."));
    edit_button->setFixedWidth(100);
    connect(edit_button, SIGNAL(clicked()), this, SLOT(editBanlist()));

    QFormLayout *form_layout = new QFormLayout;
    form_layout->addRow(tr("Server name"), server_name_edit);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->addWidget(timeout_spinbox);
    lay->addWidget(nolimit_checkbox);
    lay->addWidget(edit_button);
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

    extension_group = new QButtonGroup(this);
    extension_group->setExclusive(false);

    QSet<QString> ban_packages = Config.BanPackages.toSet();

    QGroupBox *box1 = new QGroupBox(tr("General package"));
    QGroupBox *box2 = new QGroupBox(tr("Card package"));

    QGridLayout *layout1 = new QGridLayout;
    QGridLayout *layout2 = new QGridLayout;
    box1->setLayout(layout1);
    box2->setLayout(layout2);

    int i = 0, j = 0;
    int row = 0, column = 0;
    const QList<const Package *> &packages = Sanguosha->getPackages();
    foreach(const Package *package, packages) {
        if (package->inherits("Scenario"))
            continue;

        const QString &extension = package->objectName();
        bool forbid_package = Config.value("ForbidPackages").toStringList().contains(extension);
        QCheckBox *checkbox = new QCheckBox;
        checkbox->setObjectName(extension);
        checkbox->setText(Sanguosha->translate(extension));
        checkbox->setChecked(!ban_packages.contains(extension) && !forbid_package);
        checkbox->setEnabled(!forbid_package);

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

    updateButtonEnablility(mode_group->checkedButton());
    connect(mode_group, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(updateButtonEnablility(QAbstractButton *)));

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
    QVBoxLayout *layout = new QVBoxLayout;

    bool enable_lord = Config.value("EnableLordConvertion", true).toBool();
    convert_lord = new QCheckBox(tr("Enable Lord Convertion"));
    convert_lord->setChecked(enable_lord);

    convert_ds_to_dp = new QCheckBox(tr("Convert DoubleSword to DragonPhoenix"));
    convert_ds_to_dp->setChecked(Config.value("CardConversions").toStringList().contains("DragonPhoenix") || enable_lord);
    convert_ds_to_dp->setDisabled(enable_lord);

    connect(convert_lord, SIGNAL(toggled(bool)), convert_ds_to_dp, SLOT(setChecked(bool)));
    connect(convert_lord, SIGNAL(toggled(bool)), convert_ds_to_dp, SLOT(setDisabled(bool)));

    QWidget *widget = new QWidget;
    layout->addWidget(convert_lord);
    layout->addWidget(convert_ds_to_dp);
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

    reward_the_first_showing_player_checkbox = new QCheckBox(tr("The first player to show general can draw 2 cards"));
    reward_the_first_showing_player_checkbox->setChecked(Config.RewardTheFirstShowingPlayer);

    QGroupBox *ai_groupbox = new QGroupBox(tr("Artificial intelligence"));
    ai_groupbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout *layout = new QVBoxLayout;

    forbid_adding_robot_checkbox = new QCheckBox(tr("Forbid adding robot"));
    forbid_adding_robot_checkbox->setChecked(Config.ForbidAddingRobot);

    ai_chat_checkbox = new QCheckBox(tr("Enable AI chat"));
    ai_chat_checkbox->setChecked(Config.value("AIChat", true).toBool());
    ai_chat_checkbox->setDisabled(Config.ForbidAddingRobot);
    connect(forbid_adding_robot_checkbox, SIGNAL(toggled(bool)), ai_chat_checkbox, SLOT(setDisabled(bool)));

    ai_delay_spinbox = new QSpinBox;
    ai_delay_spinbox->setMinimum(0);
    ai_delay_spinbox->setMaximum(5000);
    ai_delay_spinbox->setValue(Config.OriginAIDelay);
    ai_delay_spinbox->setSuffix(tr(" millisecond"));
    ai_delay_spinbox->setDisabled(Config.ForbidAddingRobot);
    connect(forbid_adding_robot_checkbox, SIGNAL(toggled(bool)), ai_delay_spinbox, SLOT(setDisabled(bool)));

    ai_delay_altered_checkbox = new QCheckBox(tr("Alter AI Delay After Death"));
    ai_delay_altered_checkbox->setChecked(Config.AlterAIDelayAD);
    ai_delay_altered_checkbox->setDisabled(Config.ForbidAddingRobot);
    connect(forbid_adding_robot_checkbox, SIGNAL(toggled(bool)), ai_delay_altered_checkbox, SLOT(setDisabled(bool)));

    ai_delay_ad_spinbox = new QSpinBox;
    ai_delay_ad_spinbox->setMinimum(0);
    ai_delay_ad_spinbox->setMaximum(5000);
    ai_delay_ad_spinbox->setValue(Config.AIDelayAD);
    ai_delay_ad_spinbox->setSuffix(tr(" millisecond"));
    ai_delay_ad_spinbox->setEnabled(ai_delay_altered_checkbox->isChecked());
    ai_delay_ad_spinbox->setDisabled(Config.ForbidAddingRobot);
    connect(ai_delay_altered_checkbox, SIGNAL(toggled(bool)), ai_delay_ad_spinbox, SLOT(setEnabled(bool)));
    connect(forbid_adding_robot_checkbox, SIGNAL(toggled(bool)), ai_delay_ad_spinbox, SLOT(setDisabled(bool)));

    layout->addLayout(HLay(forbid_adding_robot_checkbox, ai_chat_checkbox));
    layout->addLayout(HLay(new QLabel(tr("AI delay")), ai_delay_spinbox));
    layout->addWidget(ai_delay_altered_checkbox);
    layout->addLayout(HLay(new QLabel(tr("AI delay After Death")), ai_delay_ad_spinbox));

    ai_groupbox->setLayout(layout);

    QVBoxLayout *tablayout = new QVBoxLayout;
    tablayout->addLayout(HLay(new QLabel(tr("Game start count down")), game_start_spinbox));
    tablayout->addLayout(HLay(new QLabel(tr("Nullification count down")), nullification_spinbox));
    tablayout->addLayout(HLay(minimize_dialog_checkbox, surrender_at_death_checkbox));
    tablayout->addLayout(HLay(luck_card_label, luck_card_spinbox));
    tablayout->addWidget(reward_the_first_showing_player_checkbox);
    tablayout->addWidget(ai_groupbox);
    tablayout->addStretch();

    QWidget *widget = new QWidget;
    widget->setLayout(tablayout);
    return widget;
}

void ServerDialog::updateButtonEnablility(QAbstractButton *button) {
    if (!button) return;

    if (button->objectName().contains("scenario")) {
        mini_scene_button->setEnabled(true);
    } else {
        mini_scene_button->setEnabled(false);
    }
}

QGroupBox *ServerDialog::createGameModeBox() {
    QGroupBox *mode_box = new QGroupBox(tr("Game mode"));
    mode_box->setParent(this);
    mode_group = new QButtonGroup(this);

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

    //jiange defense
    QRadioButton *jiange_defense = new QRadioButton(tr("Jiange Defense"));
    jiange_defense->setObjectName("jiange_defense");
    mode_group->addButton(jiange_defense);

    if (Config.GameMode == "jiange_defense")
        jiange_defense->setChecked(true);

    item_list << jiange_defense;

    //mini scenes
    QRadioButton *mini_scenes = new QRadioButton(tr("Mini Scenes"));
    mini_scenes->setObjectName("custom_scenario");
    mode_group->addButton(mini_scenes);

    if (Config.GameMode == "custom_scenario")
        mini_scenes->setChecked(true);

    mini_scene_button = new QPushButton(tr("Custom Mini Scene"));
    connect(mini_scene_button, SIGNAL(clicked()), this, SLOT(doCustomAssign()));

    mini_scene_button->setEnabled(mode_group->checkedButton() ? mode_group->checkedButton()->objectName() == "mini" : false);

    item_list << mini_scenes;
    item_list << mini_scene_button;

    // ============

    QVBoxLayout *left = new QVBoxLayout;
    QVBoxLayout *right = new QVBoxLayout;

    for (int i = 0; i < item_list.length(); i++) {
        QObject *item = item_list.at(i);

        QVBoxLayout *side = i < (item_list.length() + 1) / 2 ? left : right;

        if (item->isWidgetType()) {
            QWidget *widget = qobject_cast<QWidget *>(item);
            side->addWidget(widget);
        }
        else {
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
    foreach(QHostAddress address, vAddressList) {
        if (!address.isNull() && address != QHostAddress::LocalHost
            && address.protocol() == QAbstractSocket::IPv4Protocol) {
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
    Config.RewardTheFirstShowingPlayer = reward_the_first_showing_player_checkbox->isChecked();
    Config.ForbidAddingRobot = forbid_adding_robot_checkbox->isChecked();
    Config.OriginAIDelay = ai_delay_spinbox->value();
    Config.AIDelay = Config.OriginAIDelay;
    Config.AIDelayAD = ai_delay_ad_spinbox->value();
    Config.AlterAIDelayAD = ai_delay_altered_checkbox->isChecked();
    Config.ServerPort = port_edit->text().toUShort();
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
    Config.setValue("RewardTheFirstShowingPlayer", Config.RewardTheFirstShowingPlayer);
    Config.setValue("ForbidAddingRobot", Config.ForbidAddingRobot);
    Config.setValue("OriginAIDelay", Config.OriginAIDelay);
    Config.setValue("AlterAIDelayAD", ai_delay_altered_checkbox->isChecked());
    Config.setValue("AIDelayAD", Config.AIDelayAD);
    Config.setValue("SurrenderAtDeath", Config.SurrenderAtDeath);
    Config.setValue("LuckCardLimitation", Config.LuckCardLimitation);
    Config.setValue("ServerPort", Config.ServerPort);
    Config.setValue("Address", Config.Address);
    Config.setValue("DisableLua", disable_lua_checkbox->isChecked());
    Config.setValue("AIChat", ai_chat_checkbox->isChecked());

    QSet<QString> ban_packages;
    QList<QAbstractButton *> checkboxes = extension_group->buttons();
    foreach(QAbstractButton *checkbox, checkboxes) {
        if (!checkbox->isChecked()) {
            QString package_name = checkbox->objectName();
            Sanguosha->addBanPackage(package_name);
            ban_packages.insert(package_name);
        }
    }

    Config.BanPackages = ban_packages.toList();
    Config.setValue("BanPackages", Config.BanPackages);

    QStringList general_conversions;
    Config.setValue("EnableLordConvertion", convert_lord->isChecked());

    QStringList card_conversions;
    if (convert_ds_to_dp->isChecked()) card_conversions << "DragonPhoenix";
    //if (add_peace_spell->isChecked()) card_conversions << "+109";
    Config.setValue("CardConversions", card_conversions);

    return true;
}

void ServerDialog::editBanlist() {
    BanlistDialog *dialog = new BanlistDialog(this);
    dialog->exec();
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
    foreach(Room *room, server->rooms){
        foreach(ServerPlayer *p, room->getPlayers()){
            if (p->getState() != "offline" && p->getState() != "robot") {
                sp_list << p;
            }
        }
    }

    left->clear();

    foreach(ServerPlayer *p, sp_list){
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

void BanIPDialog::addPlayer(ServerPlayer *player)
{
    if (player->getState() != "offline" && player->getState() != "robot") {
        sp_list << player;
    }

    QString parsed_string = QString("%1::%2").arg(player->screenName(), player->getIp());
    left->addItem(parsed_string);
    connect(player, SIGNAL(disconnected()), this, SLOT(removePlayer()));
}

void BanIPDialog::removePlayer()
{
    ServerPlayer *player = qobject_cast<ServerPlayer *>(sender());
    if (player) {
        int row = sp_list.indexOf(player);
        if (row != -1) {
            delete left->takeItem(row);
            sp_list.removeAt(row);
        }
    }
}

void BanlistDialog::switchTo(int item) {
    this->item = item;
    list = lists.at(item);
}

BanlistDialog::BanlistDialog(QWidget *parent, bool view)
    : QDialog(parent)
{
    setWindowTitle(tr("Select generals that are excluded"));
    setMinimumWidth(455);

    if (ban_list.isEmpty())
        ban_list << "Generals" << "Pairs";
    QVBoxLayout *layout = new QVBoxLayout;

    QTabWidget *tab = new QTabWidget;
    layout->addWidget(tab);
    connect(tab, SIGNAL(currentChanged(int)), this, SLOT(switchTo(int)));

    foreach(QString item, ban_list) {
        QWidget *apage = new QWidget;

        list = new QListWidget;
        list->setObjectName(item);

        if (item == "Pairs") {
            foreach(BanPair pair, BanPair::getBanPairSet().toList())
                addPair(pair.first, pair.second);
        }
        else {
            QStringList banlist = Config.value(QString("Banlist/%1").arg(item)).toStringList();
            foreach(QString name, banlist)
                addGeneral(name);
        }

        lists << list;

        QVBoxLayout *vlay = new QVBoxLayout;
        vlay->addWidget(list);
        apage->setLayout(vlay);

        tab->addTab(apage, Sanguosha->translate(item));
    }

    QPushButton *add = new QPushButton(tr("Add ..."));
    QPushButton *remove = new QPushButton(tr("Remove"));
    QPushButton *ok = new QPushButton(tr("OK"));

    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    connect(this, SIGNAL(accepted()), this, SLOT(saveAll()));
    connect(remove, SIGNAL(clicked()), this, SLOT(doRemoveButton()));
    connect(add, SIGNAL(clicked()), this, SLOT(doAddButton()));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    if (!view) {
        hlayout->addWidget(add);
        hlayout->addWidget(remove);
        list = lists.first();
    }

    hlayout->addWidget(ok);
    layout->addLayout(hlayout);

    setLayout(layout);

    foreach(QListWidget *alist, lists) {
        if (alist->objectName() == "Pairs")
            continue;
        alist->setViewMode(QListView::IconMode);
        alist->setDragDropMode(QListView::NoDragDrop);
        alist->setResizeMode(QListView::Adjust);
    }
}

void BanlistDialog::addGeneral(const QString &name) {
    if (list->objectName() == "Pairs") {
        if (banned_items["Pairs"].contains(name)) return;
        banned_items["Pairs"].append(name);
        QString text = QString(tr("Banned for all: %1")).arg(Sanguosha->translate(name));
        QListWidgetItem *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, QVariant::fromValue(name));
        list->addItem(item);
    } else {
        foreach(QString general_name, name.split("+")) {
            if (banned_items[list->objectName()].contains(general_name)) continue;
            banned_items[list->objectName()].append(general_name);
            QIcon icon(G_ROOM_SKIN.getGeneralPixmap(general_name, QSanRoomSkin::S_GENERAL_ICON_SIZE_TINY));
            QString text = Sanguosha->translate(general_name);
            QListWidgetItem *item = new QListWidgetItem(icon, text, list);
            item->setSizeHint(QSize(60, 60));
            item->setData(Qt::UserRole, general_name);
        }
    }
}

void BanlistDialog::addPair(const QString &first, const QString &second) {
    if (banned_items["Pairs"].contains(QString("%1+%2").arg(first, second))
        || banned_items["Pairs"].contains(QString("%1+%2").arg(second, first))) return;
    banned_items["Pairs"].append(QString("%1+%2").arg(first, second));
    QString trfirst = Sanguosha->translate(first);
    QString trsecond = Sanguosha->translate(second);
    QListWidgetItem *item = new QListWidgetItem(QString("%1 + %2").arg(trfirst, trsecond));
    item->setData(Qt::UserRole, QVariant::fromValue(QString("%1+%2").arg(first, second)));
    list->addItem(item);
}

void BanlistDialog::doAddButton() {
    FreeChooseDialog *chooser = new FreeChooseDialog(this,
        (list->objectName() == "Pairs") ? FreeChooseDialog::Pair : FreeChooseDialog::Multi);
    connect(chooser, SIGNAL(general_chosen(QString)), this, SLOT(addGeneral(QString)));
    connect(chooser, SIGNAL(pair_chosen(QString, QString)), this, SLOT(addPair(QString, QString)));
    chooser->exec();
}

void BanlistDialog::doRemoveButton() {
    int row = list->currentRow();
    if (row != -1) {
        banned_items[list->objectName()].removeOne(list->item(row)->data(Qt::UserRole).toString());
        delete list->takeItem(row);
    }
}

void BanlistDialog::save() {
    QSet<QString> banset;

    for (int i = 0; i < list->count(); i++)
        banset << list->item(i)->data(Qt::UserRole).toString();

    QStringList banlist = banset.toList();
    Config.setValue(QString("Banlist/%1").arg(ban_list.at(item)), QVariant::fromValue(banlist));
}

void BanlistDialog::saveAll() {
    for (int i = 0; i < lists.length(); i++) {
        switchTo(i);
        save();
    }
    BanPair::loadBanPairs();
}


Server::Server(QObject *parent)
    : QObject(parent)
{
    server = new NativeServerSocket;
    server->setParent(this);

    //synchronize ServerInfo on the server side to avoid ambiguous usage of Config and ServerInfo
    ServerInfo.parse(Sanguosha->getSetupString());

    current = NULL;

    connect(server, SIGNAL(new_connection(ClientSocket *)), this, SLOT(processNewConnection(ClientSocket *)));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
}

void Server::broadcastSystemMessage(const QString &msg) {
    JsonArray arg;
    arg << ".";
    arg << msg;

    Packet packet(S_SRC_ROOM | S_TYPE_NOTIFICATION | S_DEST_CLIENT, S_COMMAND_SPEAK);
    packet.setMessageBody(arg);

    foreach(Room *room, rooms)
        room->broadcast(&packet);
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
    QString address = socket->peerAddress();
    if (Config.ForbidSIMC) {
        if (addresses.contains(address)) {
            addresses.append(address);
            socket->disconnectFromHost();
            emit server_message(tr("Forbid the connection of address %1").arg(address));
            return;
        } else {
            addresses.append(address);
        }
    }

    if (Config.value("BannedIP").toStringList().contains(address)){
        socket->disconnectFromHost();
        emit server_message(tr("Forbid the connection of address %1").arg(address));
        return;
    }

    connect(socket, SIGNAL(disconnected()), this, SLOT(cleanup()));

    notifyClient(socket, S_COMMAND_CHECK_VERSION, Sanguosha->getVersion());
    notifyClient(socket, S_COMMAND_SETUP, Sanguosha->getSetupString());

    emit server_message(tr("%1 connected").arg(socket->peerName()));

    connect(socket, SIGNAL(message_got(QByteArray)), this, SLOT(processRequest(QByteArray)));
}

void Server::processRequest(const QByteArray &request)
{
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());

    Packet packet;
    if (!packet.parse(request)) {
        emit server_message(tr("Invalid message %1 from %2").arg(QString::fromUtf8(request)).arg(socket->peerAddress()));
        return;
    }

    switch (packet.getPacketSource()) {
    case S_SRC_CLIENT:
        processClientRequest(socket, packet);
        break;
    default:
        emit server_message(tr("Packet %1 from an unknown source %2").arg(QString::fromUtf8(request)).arg(socket->peerAddress()));
    }
}

void Server::notifyClient(ClientSocket *socket, CommandType command, const QVariant &arg)
{
    Packet packet(S_SRC_ROOM | S_TYPE_NOTIFICATION | S_DEST_CLIENT, command);
    packet.setMessageBody(arg);
    socket->send(packet.toJson());
}

void Server::processClientRequest(ClientSocket *socket, const Packet &signup)
{
    socket->disconnect(this, SLOT(processRequest(QByteArray)));

    if (signup.getCommandType() != S_COMMAND_SIGNUP) {
        emit server_message(tr("Invalid signup string: %1").arg(signup.toString()));
        notifyClient(socket, S_COMMAND_WARN, "INVALID_FORMAT");
        socket->disconnectFromHost();
        return;
    }

    JsonArray body = signup.getMessageBody().value<JsonArray>();
    bool is_reconnection = body[0].toBool();
    QString screen_name = body[1].toString();
    QString avatar = body[2].toString();

    if (is_reconnection) {
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
    emit newPlayer(player);

    if (current->getPlayers().length() == 1 && current->getScenario() && current->getScenario()->objectName() == "jiange_defense") {
        for (int i = 0; i < 4; ++i)
            current->addRobotCommand(player, QVariant());
    }
}

void Server::cleanup() {
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());
    if (Config.ForbidSIMC)
        addresses.removeOne(socket->peerAddress());

    socket->deleteLater();
}

void Server::signupPlayer(ServerPlayer *player) {
    name2objname.insert(player->screenName(), player->objectName());
    players.insert(player->objectName(), player);
}

void Server::gameOver() {
    Room *room = qobject_cast<Room *>(sender());
    rooms.remove(room);

    foreach(ServerPlayer *player, room->findChildren<ServerPlayer *>()) {
        name2objname.remove(player->screenName(), player->objectName());
        players.remove(player->objectName());
    }
}
