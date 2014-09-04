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

#ifndef _SERVER_H
#define _SERVER_H

class Room;
class QGroupBox;
class QLabel;
class QRadioButton;

#include "socket.h"
#include "detector.h"
#include "clientstruct.h"
#include "FlatDialog.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QButtonGroup>
#include <QComboBox>
#include <QLayoutItem>
#include <QListWidget>
#include <QSplitter>
#include <QTabWidget>
#include <QMultiHash>
#include <QTableWidget>

class Package;

class ServerDialog : public FlatDialog {
    Q_OBJECT

public:
    //************************************
    // Method:    ServerDialog
    // FullName:  ServerDialog::ServerDialog
    // Access:    public
    // Returns:
    // Qualifier:
    // Parameter: QWidget * parent
    // Description: Construct a dialog for user setting.
    //
    // Last Updated By Yanguam Siliagim
    // To use a proper way to convert generals and cards
    //
    // QSanguosha-Hegemony Team
    // March 17 2014
    //************************************
    ServerDialog(QWidget *parent);
    //************************************
    // Method:    config
    // FullName:  ServerDialog::config
    // Access:    public
    // Returns:   bool
    // Qualifier:
    // Description: To save all selections by user.
    //
    // Last Updated By Yanguam Siliagim
    // To use a proper way to convert generals and cards
    //
    // QSanguosha-Hegemony Team
    // March 17 2014
    //************************************
    bool config();

private:
    QWidget *createBasicTab();
    QWidget *createPackageTab();
    //************************************
    // Method:    createAdvancedTab
    // FullName:  ServerDialog::createAdvancedTab
    // Access:    private
    // Returns:   QWidget *
    // Qualifier:
    // Description: Create the Tab "advanced" for advanced setting.
    //
    // Last Updated By Yanguam Siliagim
    // To use a proper way to convert generals and cards
    //
    // QSanguosha-Hegemony Team
    // March 17 2014
    //************************************
    QWidget *createAdvancedTab();
    //************************************
    // Method:    createConversionTab
    // FullName:  ServerDialog::createConversionTab
    // Access:    private
    // Returns:   QWidget *
    // Qualifier:
    // Description: Create the Tab "conversions" to set conversions of cards and generals.
    //
    // Last Updated By Yanguam Siliagim
    // To use a proper way to convert generals and cards
    //
    // QSanguosha-Hegemony Team
    // March 17 2014
    //************************************
    QWidget *createConversionTab();
    QWidget *createMiscTab();
    QLayout *createButtonLayout();

    QGroupBox *createGameModeBox();

    QLineEdit *server_name_edit;
    QSpinBox *timeout_spinbox;
    QCheckBox *nolimit_checkbox;
    QCheckBox *random_seat_checkbox;
    QCheckBox *enable_cheat_checkbox;
    QCheckBox *free_choose_checkbox;
    QLabel *pile_swapping_label;
    QSpinBox *pile_swapping_spinbox;
    QCheckBox *forbid_same_ip_checkbox;
    QCheckBox *disable_chat_checkbox;
    QLabel *hegemony_maxchoice_label;
    QSpinBox *hegemony_maxchoice_spinbox;
    QPushButton *mini_scene_button;
    QLineEdit *address_edit;
    QLineEdit *port_edit;
    QSpinBox *game_start_spinbox;
    QSpinBox *nullification_spinbox;
    QCheckBox *minimize_dialog_checkbox;
    QCheckBox *reward_the_first_showing_player_checkbox;
    QCheckBox *forbid_adding_robot_checkbox;
    QSpinBox *ai_delay_spinbox;
    QCheckBox *ai_delay_altered_checkbox;
    QSpinBox *ai_delay_ad_spinbox;
    QCheckBox *surrender_at_death_checkbox;
    QLabel *luck_card_label;
    QSpinBox *luck_card_spinbox;
    QCheckBox *disable_lua_checkbox;

    QButtonGroup *extension_group;
    QButtonGroup *mode_group;


    QCheckBox *convert_ds_to_dp;
    QCheckBox *convert_lord;

    //QCheckBox *add_peace_spell;
    QCheckBox *convert_zhangjiao_to_lord;

    QCheckBox *ai_chat_checkbox;

private slots:
    void onOkButtonClicked();
    void onDetectButtonClicked();
    void editBanlist();

    void doCustomAssign();

    void updateButtonEnablility(QAbstractButton *button);
};

class Scenario;
class ServerPlayer;
class BanIPDialog;

class Server : public QObject {
    Q_OBJECT

public:
    friend class BanIPDialog;

    explicit Server(QObject *parent);

    void broadcastSystemMessage(const QString &msg);

    bool listen();
    void daemonize();


    Room *createNewRoom();
    void signupPlayer(ServerPlayer *player);

private:
    void notifyClient(ClientSocket *socket, QSanProtocol::CommandType command, const QVariant &arg = QVariant());

    void processClientRequest(ClientSocket *socket, const QSanProtocol::Packet &signup);

    ServerSocket *server;
    Room *current;
    QSet<Room *> rooms;
    QHash<QString, ServerPlayer *> players;
    QStringList addresses;
    QMultiHash<QString, QString> name2objname;

private slots:
    void processNewConnection(ClientSocket *socket);
    void processRequest(const QByteArray &request);
    void cleanup();
    void gameOver();

signals:
    void server_message(const QString &);
    void newPlayer(ServerPlayer *player);
};

class BanIPDialog : public QDialog {
    Q_OBJECT

public:
    BanIPDialog(QWidget *parent, Server *theserver);

private:
    QListWidget *left;
    QListWidget *right;

    Server *server;
    QList<ServerPlayer *> sp_list;

    void loadIPList();
    void loadBannedList();

private slots:
    void insertClicked();
    void removeClicked();
    void kickClicked();

    void save();

    void addPlayer(ServerPlayer *player);
    void removePlayer();
};

class BanlistDialog : public QDialog {
    Q_OBJECT

public:
    BanlistDialog(QWidget *parent, bool view = false);

private:
    QList<QListWidget *>lists;
    QListWidget *list;
    int item;
    QStringList ban_list;
    QMap<QString, QStringList> banned_items;

private slots:
    void addGeneral(const QString &name);
    void addPair(const QString &first, const QString &second);
    void doAddButton();
    void doRemoveButton();
    void save();
    void saveAll();
    void switchTo(int item);
};

#endif
