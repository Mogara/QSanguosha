#ifndef CUSTOMASSIGNDIALOG_H
#define CUSTOMASSIGNDIALOG_H

#include "engine.h"

#include <QHBoxLayout>
#include <QDialog>
#include <QListWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QMap>
#include <QButtonGroup>
#include <QLabel>

class CustomAssignDialog: public QDialog{
    Q_OBJECT

public:
    CustomAssignDialog(QWidget *parent);

protected:
  //  virtual void accept();
    virtual void reject();

private:
    QListWidget *list;
    QComboBox *role_combobox, *num_combobox;
    QLabel *general_label, *general_label2;

    QMap<QString, QString> role_mapping, general_mapping, general2_mapping;
    QMap<int, QString> player_mapping;
    QMap<int, QListWidgetItem *> item_map;

    QString general_name, general_name2;
    bool choose_general2;
private slots:
    void updateRole(int index);
    void updateNumber(int num);
    void doGeneralAssign();
    void doGeneralAssign2();
    void on_list_itemSelectionChanged(QListWidgetItem *current);

public slots:
    void getChosenGeneral(QString general_name);
};


class GeneralAssignDialog: public QDialog{
    Q_OBJECT

public:
    explicit GeneralAssignDialog(QWidget *parent);

private:
    QButtonGroup *group;
    QWidget *createTab(const QList<const General *> &generals);

private slots:
    void chooseGeneral();
    void uncheckExtraButton(QAbstractButton *button);

signals:
    void general_chosen(const QString &name);
};
#endif // CUSTOMASSIGNDIALOG_H
