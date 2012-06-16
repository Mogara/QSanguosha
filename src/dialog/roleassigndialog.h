#ifndef ROLEASSIGNDIALOG_H
#define ROLEASSIGNDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QMap>

class RoleAssignDialog: public QDialog{
    Q_OBJECT

public:
    RoleAssignDialog(QWidget *parent);

protected:
    virtual void accept();
    virtual void reject();

private:
    QListWidget *list;
    QComboBox *role_ComboBox;
    QMap<QString, QString> role_mapping;

private slots:
    void updateRole(int index);
    void updateRole(QListWidgetItem *current);
    void moveUp();
    void moveDown();
};

#endif // ROLEASSIGNDIALOG_H
