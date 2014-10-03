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

#ifndef SERVERDIALOG_H
#define SERVERDIALOG_H

#include "FlatDialog.h"

class QGroupBox;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class QButtonGroup;
class QAbstractButton;

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
    // QSanguosha-Rara
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
    // QSanguosha-Rara
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
    // QSanguosha-Rara
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
    // QSanguosha-Rara
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

#endif // SERVERDIALOG_H
