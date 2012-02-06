#ifndef CUSTOMASSIGNDIALOG_H
#define CUSTOMASSIGNDIALOG_H

#include "engine.h"

#include <QHBoxLayout>
#include <QSpinBox>
#include <QDialog>
#include <QListWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QMap>
#include <QButtonGroup>
#include <QLabel>
#include <QTextEdit>

class LabelButton : public QLabel {
    Q_OBJECT
public:
    LabelButton()
        :QLabel(){}

    void mouseDoubleClickEvent(QMouseEvent *)
    {
        emit double_clicked();
    }

    void mousePressEvent(QMouseEvent *)
    {
        emit clicked();
    }
signals:
    void double_clicked();
    void clicked();
};

class CustomAssignDialog: public QDialog{
    Q_OBJECT

public:
    CustomAssignDialog(QWidget *parent);

    QString setListText(QString name, QString role, int index = -1);
    void exchangePlayersInfo(QListWidgetItem *first, QListWidgetItem *second);
    void exchangeCardRange(QListWidgetItem *first, QListWidgetItem *second, QString flag);

protected:
    virtual void accept();
    virtual void reject();

private:
    QListWidget *list, *equip_list, *hand_list, *judge_list, *pile_list;
    QComboBox *role_combobox, *num_combobox, *marks_combobox;
    QCheckBox  *starter_box;
    LabelButton *general_label, *general_label2;
    QCheckBox *max_hp_prompt,*hp_prompt;
    QSpinBox *max_hp_spin,*hp_spin;
    QSpinBox *player_draw, *marks_count;
    QCheckBox *self_select_general, *self_select_general2;
    QPushButton *removeEquipButton, *removeHandButton, *removeJudgeButton, *removePileButton;
    QCheckBox *set_turned, *set_chained;
    QComboBox *single_turn_box, *before_next_box;
    QCheckBox *random_roles_box;
    QCheckBox *single_turn, *before_next;
    QLabel *single_turn_text, *single_turn_text2, *before_next_text, *before_next_text2;
    QPushButton *extra_skill_set;
    QPushButton *move_list_up_button, *move_list_down_button, *move_judge_up_button, *move_judge_down_button,
                *move_pile_up_button, *move_pile_down_button;

    QMap<QString, QString> role_mapping, general_mapping, general2_mapping;
    QMap<int, QString> player_mapping;
    QMap<int, QListWidgetItem *> item_map;

    QMap<QString, QList<int> > player_equips, player_handcards, player_judges;
    QMap<QString, int> player_maxhp, player_hp;
    QMap<QString, bool> player_turned, player_chained;
    QList<int> set_pile;
    QMap<QString, int> player_start_draw;
    QMap<QString, QMap<QString, int> > player_marks;
    QList<QLabel *> mark_icons;
    QMap<QString, bool> free_choose_general, free_choose_general2;
    QMap<QString, QStringList> player_exskills;

    QString general_name, general_name2;
    bool choose_general2;
    QString starter;
    bool is_single_turn, is_before_next;
    bool is_random_roles;

    QList<bool> set_options;
private slots:
    void updateRole(int index);
    void updateNumber(int num);
    void updateListItems();
    void updatePileInfo();
    void updatePlayerInfo(QString name);
    void updatePlayerHpInfo(QString name);
    void updateAllRoles(bool toggled = false);
    void updatePlayerExSkills(QStringList update_skills);

    void freeChoose(bool toggled);
    void freeChoose2(bool toggled);
    void doPlayerTurns(bool toggled);
    void doPlayerChains(bool toggled);
    void doSkillSelect();

    void setPlayerHpEnabled(bool toggled);
    void setPlayerMaxHpEnabled(bool toggled);
    void getPlayerHp(int hp);
    void getPlayerMaxHp(int maxhp);
    void setPlayerStartDraw(int draw_num);
    void setPlayerMarks(int value);
    void getPlayerMarks(int index);
    void setStarter(bool toggled);

    void removeEquipCard();
    void removeHandCard();
    void removeJudgeCard();
    void removePileCard();

    void doGeneralAssign();
    void doGeneralAssign2();
    void doEquipCardAssign();
    void doHandCardAssign();
    void doJudgeCardAssign();
    void doPileCardAssign();
    void clearGeneral2();

    void exchangeListItem();
   // void exchangeJudgeItem();
  //  void exchangePileItem();

    void checkSingleTurnBox(bool toggled);
    void checkBeforeNextBox(bool toggled);

    void on_list_itemSelectionChanged(QListWidgetItem *current);

    void load();
    bool save(QString path = QString());

public slots:
    void getChosenGeneral(QString general_name);
    void getEquipCard(int card_id);
    void getHandCard(int card_id);
    void getJudgeCard(int card_id);
    void getPileCard(int card_id);

signals:
    void card_addin(int card_id);
    void scenario_changed();
};


class GeneralAssignDialog: public QDialog{
    Q_OBJECT

public:
    explicit GeneralAssignDialog(QWidget *parent, bool can_ban = false);

private:
    QButtonGroup *group;
    QWidget *createTab(const QList<const General *> &generals);

private slots:
    void chooseGeneral();
    void clearGeneral();

signals:
    void general_chosen(const QString &name);
    void general_cleared();
};

class CardAssignDialog : public QDialog {
    Q_OBJECT
public:

    CardAssignDialog(QWidget *parent = 0, QString card_type = QString(), QString class_name = QString(), QList<int> excluded = QList<int>());
private:
    void addCard(const Card *card);

    QListWidget *card_list;
    QString card_type, class_name;
    QList<int> excluded_card;

private slots:
    void askCard();
    void updateCardList();
    void updateExcluded(int card_id);

signals:
    void card_chosen(int card_id);
};

class SkillAssignDialog : public QDialog{
    Q_OBJECT
public:
    SkillAssignDialog(QDialog *parent,QString player_name, QStringList &player_skills);

private:
    QListWidget *skill_list;
    QPushButton *select_skill, *delete_skill;
    QTextEdit *skill_info;

    QStringList update_skills;

private slots:
    void selectSkill();
    void deleteSkill();
    void addSkill();

    void changeSkillInfo();
    void updateSkillList();

    void getSkillFromGeneral(QString general);

    virtual void accept();

signals:
    void skill_update(QStringList);
};

#endif // CUSTOMASSIGNDIALOG_H
