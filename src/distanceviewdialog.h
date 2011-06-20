#ifndef DISTANCEVIEWDIALOG_H
#define DISTANCEVIEWDIALOG_H

class ClientPlayer;

#include <QDialog>

struct DistanceViewDialogUI;

class DistanceViewDialog : public QDialog {
    Q_OBJECT

public:
    DistanceViewDialog(QWidget *parent = 0);
    ~DistanceViewDialog();

private:
    DistanceViewDialogUI *ui;

private slots:
    void showDistance();
};

#endif // DISTANCEVIEWDIALOG_H
