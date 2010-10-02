#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

namespace Ui {
    class ConfigDialog;
}

class ConfigDialog : public QDialog {
    Q_OBJECT
public:
    ConfigDialog(QWidget *parent = 0);
    ~ConfigDialog();

private:
    Ui::ConfigDialog *ui;

private slots:
    void on_resetBgButton_clicked();
    void on_browseBgButton_clicked();
    void saveConfig();

signals:
    void bg_changed();
};

#endif // CONFIGDIALOG_H
