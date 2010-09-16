#ifndef DISTANCEVIEWDIALOG_H
#define DISTANCEVIEWDIALOG_H

class ClientPlayer;

#include <QDialog>

namespace Ui {
    class DistanceViewDialog;
}

class DistanceViewDialog : public QDialog {
    Q_OBJECT

public:
    DistanceViewDialog(QWidget *parent = 0);
    ~DistanceViewDialog();

private:
    Ui::DistanceViewDialog *ui;
    QList<const ClientPlayer *> players;

private slots:
    void showDistance();
};

#endif // DISTANCEVIEWDIALOG_H
