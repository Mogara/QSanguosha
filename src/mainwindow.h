#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "engine.h"
#include "connectiondialog.h"
#include "configdialog.h"

#include <QMainWindow>
#include <QSettings>
#include <QComboBox>

namespace Ui {
    class MainWindow;
}

class QGraphicsScene;
class QSystemTrayIcon;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void closeEvent(QCloseEvent *);

private:    
    QGraphicsScene *scene;
    Ui::MainWindow *ui;
    ConnectionDialog *connection_dialog;
    ConfigDialog *config_dialog;
    QSystemTrayIcon *systray;

    void restoreFromConfig();

private slots:
    void on_actionMinimize_to_system_tray_triggered();
    void on_actionWAN_IP_detect_triggered();
    void on_actionAbout_libircclient_triggered();
    void on_actionShow_Hide_Menu_triggered();
    void on_actionFullscreen_triggered();
    void on_actionReplay_triggered();
    void on_actionAbout_audiere_triggered();
    void on_actionAbout_triggered();
    void on_actionEnable_Hotkey_toggled(bool );
    void on_actionCard_Overview_triggered();
    void on_actionGeneral_Overview_triggered();
    void on_actionStart_Server_triggered();
    void on_actionExit_triggered();

    void startConnection();
    void restartConnection();
    void networkError(const QString &error_msg);
    void enterRoom();
    void gotoScene(QGraphicsScene *scene);
    void startGameInAnotherInstance();
    void changeBackground();
};

#endif // MAINWINDOW_H
