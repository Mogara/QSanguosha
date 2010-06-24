#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "engine.h"

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
    Engine *engine;

    void restoreFromConfig();

private slots:
    void on_actionStart_Server_triggered();
    void on_actionStart_Game_triggered();
    void on_actionExit_triggered();

    void gotoScene(QGraphicsScene *scene);
    void scriptException(const QScriptValue &exception);
};

#endif // MAINWINDOW_H
