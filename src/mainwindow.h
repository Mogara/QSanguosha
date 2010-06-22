#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
    Ui::MainWindow *ui;
    QGraphicsScene *scene;

    void restoreFromConfig();

private slots:
    void gotoScene(QGraphicsScene *scene);

    void on_actionExit_triggered();
};

#endif // MAINWINDOW_H
