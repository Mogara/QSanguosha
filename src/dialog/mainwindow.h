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

#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include "engine.h"
#include "connectiondialog.h"
#include "configdialog.h"
#include "UpdateCheckerThread.h"

#include <QMainWindow>
#include <QSettings>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>

namespace Ui {
    class MainWindow;
}

class FitView;
class QGraphicsScene;
class QSystemTrayIcon;
class Server;
class QTextEdit;
class QToolButton;
class QGroupBox;
class RoomItem;

class BroadcastBox : public QDialog {
    Q_OBJECT

public:
    BroadcastBox(Server *server, QWidget *parent = 0);

protected:
    virtual void accept();

private:
    Server *server;
    QTextEdit *text_edit;
};

class BackLoader {
public:
    static void preload();
};

#ifdef AUDIO_SUPPORT
class SoundTestBox :public QDialog{
    Q_OBJECT

public:
    SoundTestBox(QWidget *parent = NULL);

private slots:
    void btn_clicked();
    
};
#endif

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setBackgroundBrush(bool center_as_origin);

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    FitView *view;
    QGraphicsScene *scene;
    Ui::MainWindow *ui;
    ConnectionDialog *connection_dialog;
    ConfigDialog *config_dialog;
    QSystemTrayIcon *systray;
    Server *server;
    UpdateInfoStruct update_info;

    void restoreFromConfig();

public slots:
    void startConnection();

private slots:
    void on_actionAbout_GPLv3_triggered();
    void on_actionAbout_Lua_triggered();
    void on_actionAbout_fmod_triggered();
    void on_actionReplay_file_convert_triggered();
    void on_actionPackaging_triggered();
    void on_actionPC_Console_Start_triggered();
    void on_actionRecord_analysis_triggered();
    void on_actionCard_editor_triggered();
    void on_actionAcknowledgement_triggered();
    void on_actionBroadcast_triggered();
    void on_actionRule_Summary_triggered();
    void on_actionMinimize_to_system_tray_triggered();
    void on_actionShow_Hide_Menu_triggered();
    void on_actionFullscreen_triggered();
    void on_actionReplay_triggered();
    void on_actionAbout_triggered();
    void on_actionAbout_Us_triggered();
    void on_actionEnable_Hotkey_toggled(bool);
    void on_actionNever_nullify_my_trick_toggled(bool);
    void on_actionCard_Overview_triggered();
    void on_actionGeneral_Overview_triggered();
    void on_actionStart_Server_triggered();
    void on_actionExit_triggered();
    void on_actionCheckUpdate_triggered();
    void on_actionSound_Test_triggered();

    void checkVersion(const QString &server_version, const QString &server_mod);
    void networkError(const QString &error_msg);
    void enterRoom();
    void gotoScene(QGraphicsScene *scene);
    void gotoStartScene();
    void startGameInAnotherInstance();
    void changeBackground();
    void changeTableBg();
    void on_actionManage_Ban_IP_triggered();

    void storeKeyAndValue(const QString &key, const QString &value);

signals:
    void about_to_close();
};

#endif

