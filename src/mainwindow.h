#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "engine.h"
#include "connectiondialog.h"

#include <QMainWindow>
#include <QGraphicsScene>
#include <QSettings>

namespace Ui {
    class MainWindow;
}

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

    void restoreFromConfig();

private slots:
    void on_actionGeneral_Overview_triggered();
    void on_actionStart_Server_triggered();
    void on_actionExit_triggered();

    void startConnection();
    void connectionError(const QString &error_msg);
    void enterRoom();
    void gotoScene(QGraphicsScene *scene);
    void startGameInAnotherInstance();
};

#endif // MAINWINDOW_H
