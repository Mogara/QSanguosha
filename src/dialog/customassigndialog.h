/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#ifndef CUSTOMASSIGNDIALOG_H
#define CUSTOMASSIGNDIALOG_H

#include "flatdialog.h"

#include <QLabel>
#include <QMap>

class QListWidgetItem;
class QListWidget;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QButtonGroup;
class General;
class Card;
class QLineEdit;
class QTextEdit;

class LabelButton : public QLabel {
    Q_OBJECT

public:
    LabelButton() : QLabel() {}

    void mouseDoubleClickEvent(QMouseEvent *) { emit double_clicked(); }
    void mousePressEvent(QMouseEvent *) { emit clicked(); }

signals:
    void double_clicked();
    void clicked();
};

class CustomAssignDialog : public FlatDialog {
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
    QListWidget *m_list;
    QListWidget *m_equipList;
    QListWidget *m_handList;
    QListWidget *m_judgeList;
    QListWidget *m_pileList;
    QComboBox *m_numComboBox;
    QComboBox *m_marksComboBox;
    QCheckBox *m_starterCheckBox;
    LabelButton *m_generalLabel;
    LabelButton *m_generalLabel2;
    QCheckBox *m_maxHpPrompt;
    QCheckBox *m_hpPrompt;
    QSpinBox *m_maxHpSpin;
    QSpinBox *m_hpSpin;
    QSpinBox *m_playerDraw;
    QSpinBox *m_marksCount;
    QCheckBox *m_selfSelectGeneral;
    QCheckBox *m_selfSelectGeneral2;
    QCheckBox *m_headShownSetter;
    QCheckBox *m_deputyShownSetter;
    QPushButton *m_removeEquipButton;
    QPushButton *m_removeHandButton;
    QPushButton *m_removeJudgeButton;
    QPushButton *m_removePileButton;
    QCheckBox *m_turnedSetter;
    QCheckBox *m_chainedSetter;
    QComboBox *m_endedByPileBox;
    QComboBox *m_singleTurnBox;
    QComboBox *m_beforeNextBox;
    QCheckBox *m_randomRolesBox;
    QCheckBox *m_restInDpBox;
    QCheckBox *m_endedByPileCheckBox;
    QCheckBox *m_singleTurnCheckBox;
    QCheckBox *m_beforeNextCheckBox;
    QLabel *m_endedByPileText;
    QLabel *m_endedByPileText2;
    QLabel *m_singleTurnText;
    QLabel *m_singleTurnText2;
    QLabel *m_beforeNextText;
    QLabel *m_beforeNextText2;
    QPushButton *m_extraSkillSetter;
    QPushButton *m_moveListUpButton;
    QPushButton *m_moveListDownButton;
    QCheckBox *m_moveListCheck;
    QCheckBox *m_movePileCheck;
    QCheckBox *m_nationalityIsSelectableCheckBox;
    QComboBox *nationalitiesComboBox;

    QMap<QString, QString> m_generalMap;
    QMap<QString, QString> m_generalMap2;
    QMap<int, QString> m_playerMap;
    QMap<int, QListWidgetItem *> m_itemMap;

    QMap<QString, QList<int> > m_playersEquips;
    QMap<QString, QList<int> > m_playersHandcards;
    QMap<QString, QList<int> > m_playersJudges;
    QMap<QString, int> m_playersMaxHp;
    QMap<QString, int> m_playersHp;
    QMap<QString, bool> m_playerIsTurned;
    QMap<QString, bool> m_playerIsChained;
    QMap<QString, bool> m_playerHasShownHead;
    QMap<QString, bool> m_playerHasShownDeputy;
    QList<int> m_settedPile;
    QMap<QString, int> m_playerDrawNumWhenStarts;
    QMap<QString, QMap<QString, int> > m_playersMarks;
    QList<QLabel *> m_markIcons;
    QMap<QString, bool> m_canChooseGeneralFreely;
    QMap<QString, bool> m_canChooseGeneral2Freely;
    QMap<QString, QStringList> m_playersExtraSkills;
    QMap<QString, bool> m_settedNationality;
    QMap<QString, QString> m_assignedNationality;

    QString m_generalName;
    QString m_generalName2;
    bool m_enableGeneral2;
    QString m_starter;
    bool m_isEndedByPile;
    bool m_isSingleTurn;
    bool m_isBeforeNext;

    QList<bool> m_settedOptions;

    QMap<QString, int> m_kingdomIndex;

private slots:
    void updateKingdom(int index);
    void updateNumber(int num);
    void updateListItems();
    void updatePileInfo(int row = -2);
    void updatePlayerInfo(QString name);
    void updatePlayerHpInfo(QString name);
    void updateAllKingdoms(bool toggled = false);
    void updatePlayerExSkills(QStringList updatedSkills);

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
    void setMoveButtonAvaliable(bool toggled);
    void setNationality(int);
    void setNationalityEnable(bool toggled);

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

    void checkSingleTurnBox(bool toggled);
    void checkBeforeNextBox(bool toggled);
    void checkEndedByPileBox(bool toggled);

    void on_list_itemSelectionChanged(QListWidgetItem *current);

    void load();
    bool save(const QString &path);
    bool save();

    void doPlayerShows(bool toggled);
    void doPlayerShows2(bool toggled);

public slots:
    void getChosenGeneral(QString generalName);
    void getEquipCard(int card_id);
    void getHandCard(int card_id);
    void getJudgeCard(int card_id);
    void getPileCard(int card_id);

signals:
    void card_addin(int card_id);
    void scenario_changed();
};

class GeneralAssignDialog : public FlatDialog {
    Q_OBJECT

public:
    explicit GeneralAssignDialog(QWidget *parent, bool canBan = false);

private:
    QButtonGroup *group;
    QWidget *createTab(const QList<const General *> &generals);

private slots:
    void chooseGeneral();
    void clearGeneral();

signals:
    void generalChosen(const QString &name);
    void generalCleared();
};

class CardAssignDialog : public FlatDialog {
    Q_OBJECT

public:
    CardAssignDialog(QWidget *parent = 0, QString cardType = QString(), QString className = QString(), QList<int> excluded = QList<int>());

private:
    void addCard(const Card *card);

    QListWidget *m_cardList;
    QString m_cardType;
    QString m_className;
    QList<int> m_excludedCards;

private slots:
    void askCard();
    void updateCardList();
    void updateExcluded(int cardId);

signals:
    void cardChosen(int cardId);
};

class SkillAssignDialog : public FlatDialog {
    Q_OBJECT

public:
    SkillAssignDialog(QDialog *parent, QString playerName, QStringList &playerSkills);

private:
    QListWidget *m_skillList;
    QLineEdit *m_skillInput;
    QPushButton *m_selectSkillButton;
    QPushButton *m_deleteSkillButton;
    QTextEdit *m_skillInfo;

    QStringList m_updatedSkills;

private slots:
    void selectSkill();
    void deleteSkill();
    void addSkill();

    void updateSkillInfo();
    void updateSkillList();

    void getSkillFromGeneral(QString general);

    virtual void accept();

signals:
    void skillUpdated(QStringList);
};

#endif // CUSTOMASSIGNDIALOG_H
