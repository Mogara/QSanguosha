#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QComboBox>
#include <QButtonGroup>
#include <QProgressBar>

class Detector;

namespace Ui {
    class ConnectionDialog;
}

class ConnectionDialog : public QDialog {
    Q_OBJECT

public:
    ConnectionDialog(QWidget *parent);
    ~ConnectionDialog();

private:
    Ui::ConnectionDialog *ui;

private slots:
    void on_detectButton_clicked();
    void on_clearHistoryButton_clicked();
    void on_avatarList_itemDoubleClicked(QListWidgetItem* item);
    void on_changeAvatarButton_clicked();
    void on_connectButton_clicked();
};

class DetectorDialog : public QDialog{
    Q_OBJECT

public:
    DetectorDialog(QDialog *parent);

private:
    QListWidget *list;
    QButtonGroup *method_group;
    Detector *detector;
    QPushButton *detect_button;
    QProgressBar *progress_bar;

private slots:
    void onIrcServerConnected();
    void startDetection();
    void stopDetection();
    void chooseAddress(QListWidgetItem *item);
    void addServerAddress(const QString &server_name, const QString &address);

signals:
    void address_chosen(const QString &address);
};

#endif // CONNECTIONDIALOG_H
