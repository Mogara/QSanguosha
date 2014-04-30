#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QString getSystemVersion();

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

    int askForUploading();
    ~MainWindow();

    QString getSystemVersion() const;
};

#endif // MAINWINDOW_H
