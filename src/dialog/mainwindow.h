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

#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/QDeclarativeContext>

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

class AcknowledgementScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit AcknowledgementScene(QObject *parent = 0);
signals:
    void go_back();
private:
    QDeclarativeView *view;
    QDeclarativeContext *ctxt;
    QList<QObject*> tokens,equipped,loaded;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

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

public slots:
    void startConnection();

private slots:
    void on_actionAbout_Lua_triggered();
    void on_actionAbout_fmod_triggered();
    void on_actionReplay_file_convert_triggered();
    void on_actionAI_Melee_triggered();
    void on_actionPackaging_triggered();
    void on_actionScript_editor_triggered();
    void on_actionPC_Console_Start_triggered();
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
    void on_actionCard_Overview_triggered();
    void on_actionGeneral_Overview_triggered();
    void on_actionStart_Server_triggered();
    void on_actionExit_triggered();

    void checkVersion(const QString &server_version, const QString &server_mod);
    void networkError(const QString &error_msg);
    void enterRoom();
    void gotoScene(QGraphicsScene *scene);
    void gotoStartScene();
    void sendLowLevelCommand();
    void startGameInAnotherInstance();
    void changeBackground();
    void on_actionView_ban_list_triggered();
};

#endif // MAINWINDOW_H
