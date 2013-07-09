#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "engine.h"
#include "connectiondialog.h"
#include "configdialog.h"

#include <QMainWindow>
#include <QSettings>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QTableWidget>

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

class BroadcastBox: public QDialog{
    Q_OBJECT

public:
    BroadcastBox(Server *server, QWidget *parent = 0);

protected:
    virtual void accept();

private:
    Server *server;
    QTextEdit *text_edit;
};

class MeleeDialog: public QDialog{
    Q_OBJECT

public:
    MeleeDialog(QWidget *parent);

private slots:
    void selectGeneral();
    void setGeneral(const QString &general_name);
    void startTest();
    void onGameStart();
    void onGameOver(const QString &winner);

private:    
    QMap<QString, int> roleCount, winCount;

    QGroupBox *createGeneralBox();
    QGroupBox *createResultBox();
    void updateResultBox(QString role, int win);

    QToolButton *avatar_button;
    QPushButton *start_button;
    QCheckBox *loop_checkbox;
    QGraphicsScene *record_scene;
    QGroupBox *general_box;
    QGroupBox *result_box;
    QTextEdit *server_log;
    QSpinBox *spinbox;
    Server *server;
    int room_count;
    QList<RoomItem*> room_items;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setBackgroundBrush();

protected:
    virtual void closeEvent(QCloseEvent *);

private:
    FitView *view;
    QGraphicsScene *scene;
    Ui::MainWindow *ui;
    ConnectionDialog *connection_dialog;
    ConfigDialog *config_dialog;
    QSystemTrayIcon *systray;

    void restoreFromConfig();
    void fillTable(QTableWidget *table);
    QString getKeyFromUser();

public slots:
    void on_actionRestart_game_triggered();

private slots:
    void on_actionAbout_Lua_triggered();
    void on_actionAbout_fmod_triggered();
    void on_actionAbout_libtomcrypt_triggered();
    void on_actionReplay_file_convert_triggered();
    void on_actionCheck_resource_triggered();
    void on_actionAI_Melee_triggered();
    void on_actionPackaging_triggered();
    void on_actionScript_editor_triggered();
    void on_actionCard_editor_triggered();
    void on_actionAcknowledgement_triggered();
    void on_actionBroadcast_triggered();
    void on_actionScenario_Overview_triggered();
    void on_actionRole_assign_table_triggered();
    void on_actionMinimize_to_system_tray_triggered();
    void on_actionShow_Hide_Menu_triggered();
    void on_actionFullscreen_triggered();
    void on_actionReplay_triggered();
    void on_actionAbout_triggered();
    void on_actionEnable_Hotkey_toggled(bool );
    void on_actionEnable_Lua_triggered();
    void on_actionEnable_Lua_toggled(bool);
    void on_actionCard_Overview_triggered();
    void on_actionGeneral_Overview_triggered();
    void on_actionStart_Game_triggered();
    void on_actionReturn_main_triggered();
    void on_actionPause_toggled(bool);
    void on_actionExit_triggered();
    void on_actionView_ban_list_triggered();

    void on_actionCStandard_toggled(bool);
    void on_actionCGolden_Snake_toggled(bool);
    void on_actionSgs_OL_toggled(bool);
    void on_actionGStandard_toggled(bool);
    void on_actionGGolden_Snake_toggled(bool);
    void on_actionCustom_toggled(bool);

    void checkVersion(const QString &server_version, const QString &server_mod);
    void networkError(const QString &error_msg);
    void enterRoom();
    void gotoScene(QGraphicsScene *scene);
    void sendLowLevelCommand();
    void startGameInAnotherInstance();
    void changeBackground();
    void on_actionEncrypt_files_triggered();
    void on_actionDecrypt_files_triggered();
};

#endif // MAINWINDOW_H
