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
#include <QtGui/QTableWidget>

class CustomAssignDialog: public QDialog{
    Q_OBJECT

public:
    CustomAssignDialog(QWidget *parent);

    void setCardGotId(int card_id);

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

    QMap<QString, QList<int> > player_equips, player_handcards, player_judges;
    QList<int> set_pile;

    QString general_name, general_name2;
    bool choose_general2;
    int temp_card_id;

private slots:
    void updateRole(int index);
    void updateNumber(int num);
    void doGeneralAssign();
    void doGeneralAssign2();
    void doEquipCardAssign();
    void doHandCardAssign();

    void on_list_itemSelectionChanged(QListWidgetItem *current);

public slots:
    void getChosenGeneral(QString general_name);
    void getEquipCard(int card_id);
    void getHandCard(int card_id);
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

signals:
    void general_chosen(const QString &name);
};

class CardAssignDialog : public QDialog {
    Q_OBJECT
public:

    CardAssignDialog(QWidget *parent = 0, QString card_type = QString(), QString class_name = QString());
    void loadFromAll();
    void loadFromList(const QList<const Card*> &list);

private:
    void addCard(const Card *card);

    QListWidget *card_list;

private slots:
    void askCard();

signals:
    void card_chosen(int card_id);
};

#endif // CUSTOMASSIGNDIALOG_H
