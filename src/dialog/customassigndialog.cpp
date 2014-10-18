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

#include "customassigndialog.h"
#include "engine.h"
#include "settings.h"
#include "scenario.h"
#include "miniscenarios.h"
#include "stylehelper.h"
#include "skinbank.h"

#include <QListWidget>
#include <QBoxLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QDir>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QLineEdit>
#include <QCompleter>
#include <QTextEdit>
#include <QCommandLinkButton>
#include <QScrollBar>

static QLayout *HLay(QWidget *left, QWidget *right, QWidget *mid = NULL,
    QWidget *rear = NULL, bool is_vertically = false) {
    QBoxLayout *layout;
    if (is_vertically)
        layout = new QVBoxLayout;
    else
        layout = new QHBoxLayout;

    layout->addWidget(left);
    if (mid)
        layout->addWidget(mid);
    layout->addWidget(right);
    if (rear) layout->addWidget(rear);

    return layout;
}

static void stylizeScrollBars(QListWidget *widget) {
    static QString styleSheet = StyleHelper::styleSheetOfScrollBar();
    widget->horizontalScrollBar()->setStyleSheet(styleSheet);
    widget->verticalScrollBar()->setStyleSheet(styleSheet);
}

CustomAssignDialog *CustomInstance = NULL;

CustomAssignDialog::CustomAssignDialog(QWidget *parent)
    : FlatDialog(parent),
    m_enableGeneral2(false),
    m_isEndedByPile(false), m_isSingleTurn(false), m_isBeforeNext(false)
{
    setWindowTitle(tr("Custom mini scene"));

    CustomInstance = this;

    m_list = new QListWidget;
    m_list->setFlow(QListView::TopToBottom);
    m_list->setMovement(QListView::Static);

    QScrollBar *bar = m_list->verticalScrollBar();
    bar->setStyleSheet(StyleHelper::styleSheetOfScrollBar());

    QVBoxLayout *vLayout = new QVBoxLayout;
    QVBoxLayout *vLayout2 = new QVBoxLayout;
    m_numComboBox = new QComboBox;
    for (int i = 0; i <= 9; i++) {
        if (i < 9)
            m_numComboBox->addItem(tr("%1 persons").arg(QString::number(i + 2)), i + 2);

        QString player = (i == 0 ? "Player" : "AI");
        QString text = (i == 0 ? QString("%1[%2]").arg(Sanguosha->translate(player)).arg(tr("default")) :
            QString("%1%2[%3]")
            .arg(Sanguosha->translate(player))
            .arg(QString::number(i))
            .arg(tr("default")));
        if (i != 0)
            player.append(QString::number(i));
        m_playerMap[i] = player;
        m_assignedNationality[player] = "default";
        m_settedNationality[player] = false;

        QListWidgetItem *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, player);
        m_itemMap[i] = item;
    }

    for (int i = 0; i < m_numComboBox->currentIndex() + 2; i++)
        m_list->addItem(m_itemMap[i]);
    m_list->setCurrentItem(m_itemMap[0]);

    m_playerDraw = new QSpinBox;
    m_playerDraw->setRange(0, Sanguosha->getCardCount());
    m_playerDraw->setValue(4);
    m_playerDraw->setEnabled(true);

    QGroupBox *starter_group = new QGroupBox(tr("Start Info"));
    m_starterCheckBox = new QCheckBox(tr("Set as Starter"));
    QLabel *draw_text = new QLabel(tr("Start Draw"));
    QLabel *mark_text = new QLabel(tr("marks"));
    QLabel *mark_num_text = new QLabel(tr("pieces"));

    m_marksComboBox = new QComboBox;
    m_marksComboBox->addItem(tr("None"));
    QString path = "image/mark";
    QDir *dir = new QDir(path);
    QStringList filter;
    filter << "*.png";
    dir->setNameFilters(filter);
    QList<QFileInfo> file_info(dir->entryInfoList(filter));
    foreach(QFileInfo file, file_info) {
        QString mark_name = file.fileName().split(".").first();
        QString mark_translate = Sanguosha->translate(mark_name);
        if (!mark_translate.startsWith("@")) {
            m_marksComboBox->addItem(mark_translate, mark_name);
            QLabel *mark_icon = new QLabel(mark_translate);
            mark_icon->setPixmap(QPixmap(file.filePath()));
            mark_icon->setObjectName(mark_name);
            mark_icon->setToolTip(tr("<font color=%1>%2 mark</font>").arg(Config.SkillDescriptionInToolTipColor.name()).arg(mark_translate));
            m_markIcons << mark_icon;
        }
    }

    m_marksCount = new QSpinBox;
    m_marksCount->setRange(0, 999);
    m_marksCount->setEnabled(false);

    QVBoxLayout *starter_lay = new QVBoxLayout;
    starter_group->setLayout(starter_lay);
    starter_lay->addWidget(m_starterCheckBox);
    starter_lay->addLayout(HLay(draw_text, m_playerDraw));
    starter_lay->addLayout(HLay(m_marksComboBox, m_marksCount, mark_text, mark_num_text));

    QGridLayout *grid_layout = new QGridLayout;
    const int columns = m_markIcons.length() > 10 ? 5 : 4;
    for (int i = 0; i < m_markIcons.length(); i++) {
        int row = i / columns;
        int column = i % columns;
        grid_layout->addWidget(m_markIcons.at(i), row, column + 1);
        m_markIcons.at(i)->hide();
    }
    starter_lay->addLayout(grid_layout);

    m_generalLabel = new LabelButton;
    m_generalLabel->setPixmap(QPixmap("image/system/disabled.png"));
    m_generalLabel->setFixedSize(G_COMMON_LAYOUT.m_tinyAvatarSize);
    QGroupBox *general_box = new QGroupBox(tr("General"));
    general_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QVBoxLayout *general_lay = new QVBoxLayout();
    general_box->setLayout(general_lay);
    general_lay->addWidget(m_generalLabel);

    m_generalLabel2 = new LabelButton;
    m_generalLabel2->setPixmap(QPixmap("image/system/disabled.png"));
    m_generalLabel2->setFixedSize(G_COMMON_LAYOUT.m_tinyAvatarSize);
    QGroupBox *general_box2 = new QGroupBox(tr("General2"));
    general_box2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QVBoxLayout *general_lay2 = new QVBoxLayout();
    general_box2->setLayout(general_lay2);
    general_lay2->addWidget(m_generalLabel2);

    QPushButton *equipAssign = new QPushButton(tr("EquipAssign"));
    QPushButton *handcardAssign = new QPushButton(tr("HandcardAssign"));
    QPushButton *judgeAssign = new QPushButton(tr("JudgeAssign"));
    QPushButton *pileAssign = new QPushButton(tr("PileCardAssign"));

    m_randomRolesBox = new QCheckBox(tr("RandomRoles"));
    m_restInDpBox = new QCheckBox(tr("RestInDiscardPile"));

    m_maxHpPrompt = new QCheckBox(tr("Max Hp"));
    m_maxHpPrompt->setChecked(false);
    m_maxHpSpin = new QSpinBox();
    m_maxHpSpin->setRange(2, 10);
    m_maxHpSpin->setValue(4);
    m_maxHpSpin->setEnabled(false);

    m_hpPrompt = new QCheckBox(tr("Hp"));
    m_hpPrompt->setChecked(false);
    m_hpSpin = new QSpinBox();
    m_hpSpin->setRange(1, m_maxHpSpin->value());
    m_hpSpin->setValue(4);
    m_hpSpin->setEnabled(false);

    m_selfSelectGeneral = new QCheckBox(tr("General Self Select"));
    m_selfSelectGeneral2 = new QCheckBox(tr("General2 Self Select"));

    m_headShownSetter = new QCheckBox(tr("Show Head General"));
    m_deputyShownSetter = new QCheckBox(tr("Show Deputy General"));

    m_turnedSetter = new QCheckBox(tr("Player Turned"));
    m_chainedSetter = new QCheckBox(tr("Player Chained"));

    m_nationalityIsSelectableCheckBox = new QCheckBox(tr("Customize Nationality"));
    nationalitiesComboBox = new QComboBox;
    int index = 0;
    foreach(QString kingdom, Sanguosha->getKingdoms()) {
        nationalitiesComboBox->addItem(QIcon(QString("image/kingdom/icon/%1.png").arg(kingdom)), Sanguosha->translate(kingdom), kingdom);
        m_kingdomIndex[kingdom] = index;
        index++;
    }
    nationalitiesComboBox->setEnabled(false);

    m_extraSkillSetter = new QPushButton(tr("Set Extra Skills"));

    m_endedByPileText = new QLabel(tr("When pile ends"));
    m_endedByPileText2 = new QLabel(tr("win"));
    m_endedByPileBox = new QComboBox();
    m_endedByPileCheckBox = new QCheckBox(tr("Ended by pile ends"));
    m_endedByPileCheckBox->setEnabled(m_settedPile.length() > 0);

    m_singleTurnText = new QLabel(tr("After this turn "));
    m_singleTurnText2 = new QLabel(tr("win"));
    m_singleTurnBox = new QComboBox();
    m_singleTurnCheckBox = new QCheckBox(tr("After this turn you lose"));

    m_beforeNextText = new QLabel(tr("Before next turn "));
    m_beforeNextText2 = new QLabel(tr("win"));
    m_beforeNextBox = new QComboBox();
    m_beforeNextCheckBox = new QCheckBox(tr("Before next turn begin player lose"));

    foreach(QString kingdom, Sanguosha->getKingdoms()) {
        m_endedByPileBox->addItem(Sanguosha->translate(kingdom), kingdom);
        m_singleTurnBox->addItem(Sanguosha->translate(kingdom), kingdom);
        m_beforeNextBox->addItem(Sanguosha->translate(kingdom), kingdom);
    }

    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    QPushButton *loadButton = new QPushButton(tr("load"));
    QPushButton *saveButton = new QPushButton(tr("save"));
    QPushButton *defaultLoadButton = new QPushButton(tr("Default load"));
    defaultLoadButton->setObjectName("default_load");

    vLayout->addWidget(m_numComboBox);
    QHBoxLayout *label_lay = new QHBoxLayout;
    label_lay->addWidget(general_box);
    label_lay->addWidget(general_box2);
    vLayout->addLayout(label_lay);
    vLayout->addLayout(HLay(m_selfSelectGeneral, m_selfSelectGeneral2));
    vLayout->addLayout(HLay(m_headShownSetter, m_deputyShownSetter));
    vLayout->addLayout(HLay(m_maxHpPrompt, m_maxHpSpin));
    vLayout->addLayout(HLay(m_hpPrompt, m_hpSpin));
    vLayout->addLayout(HLay(m_turnedSetter, m_chainedSetter));
    vLayout->addLayout(HLay(m_nationalityIsSelectableCheckBox, nationalitiesComboBox));
    vLayout->addWidget(m_extraSkillSetter);
    vLayout->addStretch();
    vLayout->addWidget(m_randomRolesBox);
    vLayout->addWidget(m_restInDpBox);
    vLayout2->addWidget(starter_group);
    vLayout2->addWidget(m_endedByPileCheckBox);
    vLayout2->addLayout(HLay(m_endedByPileText, m_endedByPileText2, m_endedByPileBox));
    vLayout2->addWidget(m_singleTurnCheckBox);
    vLayout2->addLayout(HLay(m_singleTurnText, m_singleTurnText2, m_singleTurnBox));
    vLayout2->addWidget(m_beforeNextCheckBox);
    vLayout2->addLayout(HLay(m_beforeNextText, m_beforeNextText2, m_beforeNextBox));
    vLayout2->addStretch();
    vLayout2->addWidget(defaultLoadButton);
    vLayout2->addLayout(HLay(loadButton, saveButton));
    vLayout2->addLayout(HLay(okButton, cancelButton));

    m_endedByPileText->hide();
    m_endedByPileText2->hide();
    m_endedByPileBox->hide();
    m_singleTurnText->hide();
    m_singleTurnText2->hide();
    m_singleTurnBox->hide();
    m_beforeNextText->hide();
    m_beforeNextText2->hide();
    m_beforeNextBox->hide();

    m_equipList = new QListWidget;
    stylizeScrollBars(m_equipList);
    m_handList = new QListWidget;
    stylizeScrollBars(m_handList);
    m_judgeList = new QListWidget;
    stylizeScrollBars(m_judgeList);
    m_pileList = new QListWidget;
    stylizeScrollBars(m_pileList);
    QVBoxLayout *infoLayout = new QVBoxLayout(), *equipLayout = new QVBoxLayout(), *handLayout = new QVBoxLayout(),
        *judgeLayout = new QVBoxLayout(), *pileLayout = new QVBoxLayout();

    m_moveListUpButton = new QPushButton(tr("Move Up"));
    m_moveListDownButton = new QPushButton(tr("Move Down"));
    m_moveListCheck = new QCheckBox(tr("Move Player List"));
    m_movePileCheck = new QCheckBox(tr("Move Pile List"));

    m_moveListCheck->setObjectName("list check");
    m_movePileCheck->setObjectName("pile check");
    m_moveListUpButton->setObjectName("list_up");
    m_moveListDownButton->setObjectName("list_down");
    m_moveListUpButton->setEnabled(false);
    m_moveListDownButton->setEnabled(false);
    QVBoxLayout *listMoveLayout = new QVBoxLayout;
    listMoveLayout->addWidget(m_moveListCheck);
    listMoveLayout->addWidget(m_movePileCheck);
    listMoveLayout->addStretch();
    listMoveLayout->addWidget(m_moveListUpButton);
    listMoveLayout->addWidget(m_moveListDownButton);
    QHBoxLayout *listLayout = new QHBoxLayout;
    listLayout->addWidget(m_list);
    listLayout->addLayout(listMoveLayout);
    infoLayout->addLayout(listLayout);
    QGroupBox *equipGroup = new QGroupBox(tr("Equips"));
    QGroupBox *handsGroup = new QGroupBox(tr("Handcards"));
    QGroupBox *judgeGroup = new QGroupBox(tr("Judges"));
    QGroupBox *pileGroup = new QGroupBox(tr("DrawPile"));
    equipGroup->setLayout(equipLayout);
    handsGroup->setLayout(handLayout);
    judgeGroup->setLayout(judgeLayout);
    pileGroup->setLayout(pileLayout);

    m_removeEquipButton = new QPushButton(tr("Remove Equip"));
    m_removeHandButton = new QPushButton(tr("Remove Handcard"));
    m_removeJudgeButton = new QPushButton(tr("Remove Judge"));
    m_removePileButton = new QPushButton(tr("Remove Pilecard"));

    m_removeEquipButton->setEnabled(false);
    m_removeHandButton->setEnabled(false);
    m_removeJudgeButton->setEnabled(false);
    m_removePileButton->setEnabled(false);
    equipLayout->addWidget(m_equipList);
    equipLayout->addLayout(HLay(equipAssign, m_removeEquipButton));
    handLayout->addWidget(m_handList);
    handLayout->addLayout(HLay(handcardAssign, m_removeHandButton));
    infoLayout->addLayout(HLay(equipGroup, handsGroup));
    judgeLayout->addWidget(m_judgeList);
    judgeLayout->addLayout(HLay(judgeAssign, m_removeJudgeButton));
    pileLayout->addWidget(m_pileList);
    pileLayout->addLayout(HLay(pileAssign, m_removePileButton));
    infoLayout->addLayout(HLay(judgeGroup, pileGroup));

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addLayout(infoLayout);
    hLayout->addLayout(vLayout);
    hLayout->addLayout(vLayout2);
    layout->addLayout(hLayout);

    connect(nationalitiesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateKingdom(int)));
    connect(m_list, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
        this, SLOT(on_list_itemSelectionChanged(QListWidgetItem *)));
    connect(m_moveListUpButton, SIGNAL(clicked()), this, SLOT(exchangeListItem()));
    connect(m_moveListDownButton, SIGNAL(clicked()), this, SLOT(exchangeListItem()));
    connect(m_moveListCheck, SIGNAL(toggled(bool)), this, SLOT(setMoveButtonAvaliable(bool)));
    connect(m_movePileCheck, SIGNAL(toggled(bool)), this, SLOT(setMoveButtonAvaliable(bool)));
    connect(m_numComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateNumber(int)));
    connect(m_generalLabel, SIGNAL(clicked()), this, SLOT(doGeneralAssign()));
    connect(m_generalLabel2, SIGNAL(clicked()), this, SLOT(doGeneralAssign2()));
    connect(m_maxHpPrompt, SIGNAL(toggled(bool)), m_maxHpSpin, SLOT(setEnabled(bool)));
    connect(m_hpPrompt, SIGNAL(toggled(bool)), m_hpSpin, SLOT(setEnabled(bool)));
    connect(m_hpPrompt, SIGNAL(toggled(bool)), this, SLOT(setPlayerHpEnabled(bool)));
    connect(m_maxHpPrompt, SIGNAL(toggled(bool)), this, SLOT(setPlayerMaxHpEnabled(bool)));
    connect(m_selfSelectGeneral, SIGNAL(toggled(bool)), this, SLOT(freeChoose(bool)));
    connect(m_selfSelectGeneral2, SIGNAL(toggled(bool)), this, SLOT(freeChoose2(bool)));
    connect(m_selfSelectGeneral, SIGNAL(toggled(bool)), m_generalLabel, SLOT(setDisabled(bool)));
    connect(m_selfSelectGeneral2, SIGNAL(toggled(bool)), m_generalLabel2, SLOT(setDisabled(bool)));
    connect(m_headShownSetter, SIGNAL(toggled(bool)), this, SLOT(doPlayerShows(bool)));
    connect(m_deputyShownSetter, SIGNAL(toggled(bool)), this, SLOT(doPlayerShows2(bool)));
    connect(m_turnedSetter, SIGNAL(toggled(bool)), this, SLOT(doPlayerTurns(bool)));
    connect(m_chainedSetter, SIGNAL(toggled(bool)), this, SLOT(doPlayerChains(bool)));
    connect(m_nationalityIsSelectableCheckBox, SIGNAL(toggled(bool)), nationalitiesComboBox, SLOT(setEnabled(bool)));
    connect(m_nationalityIsSelectableCheckBox, SIGNAL(toggled(bool)), this, SLOT(setNationalityEnable(bool)));
    connect(nationalitiesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setNationality(int)));
    connect(m_randomRolesBox, SIGNAL(toggled(bool)), this, SLOT(updateAllKingdoms(bool)));
    connect(m_extraSkillSetter, SIGNAL(clicked()), this, SLOT(doSkillSelect()));
    connect(m_hpSpin, SIGNAL(valueChanged(int)), this, SLOT(getPlayerHp(int)));
    connect(m_maxHpSpin, SIGNAL(valueChanged(int)), this, SLOT(getPlayerMaxHp(int)));
    connect(m_playerDraw, SIGNAL(valueChanged(int)), this, SLOT(setPlayerStartDraw(int)));
    connect(m_starterCheckBox, SIGNAL(toggled(bool)), this, SLOT(setStarter(bool)));
    connect(m_marksCount, SIGNAL(valueChanged(int)), this, SLOT(setPlayerMarks(int)));
    connect(m_marksComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(getPlayerMarks(int)));
    connect(m_pileList, SIGNAL(currentRowChanged(int)), this, SLOT(updatePileInfo(int)));
    connect(m_removeEquipButton, SIGNAL(clicked()), this, SLOT(removeEquipCard()));
    connect(m_removeHandButton, SIGNAL(clicked()), this, SLOT(removeHandCard()));
    connect(m_removeJudgeButton, SIGNAL(clicked()), this, SLOT(removeJudgeCard()));
    connect(m_removePileButton, SIGNAL(clicked()), this, SLOT(removePileCard()));
    connect(equipAssign, SIGNAL(clicked()), this, SLOT(doEquipCardAssign()));
    connect(handcardAssign, SIGNAL(clicked()), this, SLOT(doHandCardAssign()));
    connect(judgeAssign, SIGNAL(clicked()), this, SLOT(doJudgeCardAssign()));
    connect(pileAssign, SIGNAL(clicked()), this, SLOT(doPileCardAssign()));
    connect(m_endedByPileCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkEndedByPileBox(bool)));
    connect(m_singleTurnCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBeforeNextBox(bool)));
    connect(m_beforeNextCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkSingleTurnBox(bool)));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(loadButton, SIGNAL(clicked()), this, SLOT(load()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));
    connect(defaultLoadButton, SIGNAL(clicked()), this, SLOT(load()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void CustomAssignDialog::exchangePlayersInfo(QListWidgetItem *first, QListWidgetItem *second) {
    QString first_name = first->data(Qt::UserRole).toString();
    QString second_name = second->data(Qt::UserRole).toString();

    QString general = m_generalMap[first_name], general2 = m_generalMap2[first_name];
    QList<int> judges(m_playersJudges[first_name]), equips(m_playersEquips[first_name]), hands(m_playersHandcards[first_name]);
    int hp = m_playersHp[first_name], maxhp = m_playersMaxHp[first_name], start_draw = m_playerDrawNumWhenStarts[first_name];
    bool turned = m_playerIsTurned[first_name], chained = m_playerIsChained[first_name],
        free_general = m_canChooseGeneralFreely[first_name], free_general2 = m_canChooseGeneral2Freely[first_name];
    bool shown_head = m_playerHasShownHead[first_name], shown_deputy = m_playerHasShownDeputy[first_name];
    QStringList ex_skills(m_playersExtraSkills[first_name]);
    QMap<QString, int> marks(m_playersMarks[first_name]);
    bool setting_nationality = m_settedNationality.value(first_name, false);
    QString assigned_nationality = m_assignedNationality.value(first_name, "");

    m_generalMap[first_name] = m_generalMap[second_name];
    m_generalMap2[first_name] = m_generalMap2[second_name];
    m_playersJudges[first_name].clear();
    m_playersJudges[first_name].append(m_playersJudges[second_name]);
    m_playersEquips[first_name].clear();
    m_playersEquips[first_name].append(m_playersEquips[second_name]);
    m_playersHandcards[first_name].clear();
    m_playersHandcards[first_name].append(m_playersHandcards[second_name]);
    m_playersHp[first_name] = m_playersHp[second_name];
    m_playersMaxHp[first_name] = m_playersMaxHp[second_name];
    m_playerDrawNumWhenStarts[first_name] = m_playerDrawNumWhenStarts[second_name];
    m_playerIsTurned[first_name] = m_playerIsTurned[second_name];
    m_playerIsChained[first_name] = m_playerIsChained[second_name];
    m_canChooseGeneralFreely[first_name] = m_canChooseGeneralFreely[second_name];
    m_canChooseGeneral2Freely[first_name] = m_canChooseGeneral2Freely[second_name];
    m_playersExtraSkills[first_name].clear();
    m_playersExtraSkills[first_name].append(m_playersExtraSkills[second_name]);
    m_playersMarks[first_name].clear();
    m_playersMarks[first_name] = m_playersMarks[second_name];
    m_settedNationality[first_name] = m_settedNationality[second_name];
    m_assignedNationality[first_name] = m_settedNationality[second_name];
    m_playerHasShownHead[first_name] = m_playerHasShownHead[second_name];
    m_playerHasShownDeputy[first_name] = m_playerHasShownDeputy[second_name];

    m_generalMap[second_name] = general;
    m_generalMap2[second_name] = general2;
    m_playersJudges[second_name].clear();
    m_playersJudges[second_name].append(judges);
    m_playersEquips[second_name].clear();
    m_playersEquips[second_name].append(equips);
    m_playersHandcards[second_name].clear();
    m_playersHandcards[second_name].append(hands);
    m_playersHp[second_name] = hp;
    m_playersMaxHp[second_name] = maxhp;
    m_playerDrawNumWhenStarts[second_name] = start_draw;
    m_playerIsTurned[second_name] = turned;
    m_playerIsChained[second_name] = chained;
    m_canChooseGeneralFreely[second_name] = free_general;
    m_canChooseGeneral2Freely[second_name] = free_general2;
    m_playersExtraSkills[second_name].clear();
    m_playersExtraSkills[second_name].append(ex_skills);
    m_playersMarks[second_name].clear();
    m_playersMarks[second_name] = marks;
    m_settedNationality[second_name] = setting_nationality;
    m_assignedNationality[second_name] = assigned_nationality;
    m_playerHasShownHead[second_name] = shown_head;
    m_playerHasShownDeputy[second_name] = shown_deputy;
}

QString CustomAssignDialog::setListText(QString name, QString kingdom, int index) {
    QString text = m_randomRolesBox->isChecked() ? QString("[%1]").arg(Sanguosha->translate(kingdom)) :
        QString("%1[%2]").arg(Sanguosha->translate(name))
        .arg(Sanguosha->translate(kingdom));

    if (index >= 0)
        m_list->item(index)->setText(text);

    return text;
}

void CustomAssignDialog::updateListItems() {
    for (int i = 0; i <= 9; i++) {
        QString name = (i == 0 ? "Player" : "AI");
        if (i != 0)
            name.append(QString::number(i));

        if (m_assignedNationality[name].isEmpty()) m_assignedNationality[name] = "default";
        m_settedNationality[name] = m_assignedNationality[name] == "default" ? false : true;
        QListWidgetItem *item = new QListWidgetItem(setListText(name, m_assignedNationality[name]));
        item->setData(Qt::UserRole, name);
        m_itemMap[i] = item;
    }
}

void CustomAssignDialog::doEquipCardAssign() {
    QList<int> excluded;
    for (int i = 0; i < m_list->count(); i++) {
        excluded.append(m_playersEquips[m_list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(m_playersHandcards[m_list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(m_settedPile);
    }

    CardAssignDialog *dialog = new CardAssignDialog(this, "equip", "", excluded);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(cardChosen(int)), this, SLOT(getEquipCard(int)));
    dialog->exec();
}

void CustomAssignDialog::getEquipCard(int card_id) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    QString card_type = Sanguosha->getEngineCard(card_id)->getSubtype();
    foreach(int id, m_playersEquips[name]) {
        if (card_type == Sanguosha->getEngineCard(id)->getSubtype()) {
            emit card_addin(id);
            m_playersEquips[name].removeOne(id);
            break;
        }
    }

    m_playersEquips[name] << card_id;
    updatePlayerInfo(name);
    m_equipList->setCurrentRow(0);
    m_removeEquipButton->setEnabled(true);
}

void CustomAssignDialog::doHandCardAssign() {
    QList<int> excluded;
    for (int i = 0; i < m_list->count(); i++) {
        excluded.append(m_playersHandcards[m_list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(m_playersEquips[m_list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(m_playersJudges[m_list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(m_settedPile);
    }

    CardAssignDialog *dialog = new CardAssignDialog(this, "", "", excluded);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(cardChosen(int)), this, SLOT(getHandCard(int)));
    dialog->exec();
}

void CustomAssignDialog::getHandCard(int card_id) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    if (m_playersHandcards[name].contains(card_id))
        return;

    m_playersHandcards[name] << card_id;
    updatePlayerInfo(name);
    m_handList->setCurrentRow(0);
    m_removeHandButton->setEnabled(true);
}

void CustomAssignDialog::doJudgeCardAssign() {
    QList<int> excluded;
    for (int i = 0; i < m_list->count(); i++) {
        excluded.append(m_playersJudges[m_list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(m_playersHandcards[m_list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(m_settedPile);
    }

    CardAssignDialog *dialog = new CardAssignDialog(this, "", "DelayedTrick", excluded);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(cardChosen(int)), this, SLOT(getJudgeCard(int)));
    dialog->exec();
}

void CustomAssignDialog::getJudgeCard(int card_id) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    QString card_name = Sanguosha->getEngineCard(card_id)->objectName();
    foreach(int id, m_playersJudges[name]) {
        if (Sanguosha->getEngineCard(id)->objectName() == card_name) {
            emit card_addin(id);
            m_playersJudges[name].removeOne(id);
            break;
        }
    }

    m_playersJudges[name] << card_id;
    updatePlayerInfo(name);
    m_judgeList->setCurrentRow(0);
    m_removeJudgeButton->setEnabled(true);
}

void CustomAssignDialog::doPileCardAssign() {
    QList<int> excluded;
    for (int i = 0; i < m_list->count(); i++) {
        excluded.append(m_playersHandcards[m_list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(m_playersEquips[m_list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(m_playersJudges[m_list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(m_settedPile);
    }

    CardAssignDialog *dialog = new CardAssignDialog(this, "", "", excluded);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(cardChosen(int)), this, SLOT(getPileCard(int)));
    dialog->exec();
}

void CustomAssignDialog::getPileCard(int card_id) {
    if (m_settedPile.contains(card_id))
        return;

    m_settedPile << card_id;
    updatePileInfo();
    m_pileList->setCurrentRow(0);
    m_removePileButton->setEnabled(true);
}

void CustomAssignDialog::updateNumber(int num) {
    int count = m_numComboBox->itemData(num).toInt();
    if (count < m_list->count()) {
        for (int i = m_list->count() - 1; i >= count; i--)
            m_list->takeItem(i);
    }
    else {
        for (int i = m_list->count(); i < count; i++)
            m_list->addItem(m_itemMap[i]);
    }
}

void CustomAssignDialog::setNationalityEnable(bool toggled) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    m_settedNationality[name] = toggled;
    if (toggled == true) {
        QString kingdom = nationalitiesComboBox->itemData(nationalitiesComboBox->currentIndex()).toString();
        m_assignedNationality[name] = kingdom;
        setListText(name, kingdom, m_list->currentRow());
    }
    else {
        m_assignedNationality[name] = "default";
        setListText(name, "default", m_list->currentRow());
    }
}

void CustomAssignDialog::setNationality(int index) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    QString kingdom = nationalitiesComboBox->itemData(index).toString();
    m_assignedNationality[name] = kingdom;
    setListText(name, kingdom, m_list->currentRow());
}

void CustomAssignDialog::updatePlayerInfo(QString name) {
    m_equipList->clear();
    m_handList->clear();
    m_judgeList->clear();

    m_removeEquipButton->setEnabled(!m_playersEquips[name].isEmpty());
    m_removeHandButton->setEnabled(!m_playersHandcards[name].isEmpty());
    m_removeJudgeButton->setEnabled(!m_playersJudges[name].isEmpty());

    foreach(int equip_id, m_playersEquips[name]) {
        const Card *card = Sanguosha->getEngineCard(equip_id);
        QString card_name = Sanguosha->translate(card->objectName());
        QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(card->getSuitString()));
        QString point = card->getNumberString();

        QString card_info = point + "  " + card_name + "\t\t" + Sanguosha->translate(card->getSubtype());
        QListWidgetItem *name_item = new QListWidgetItem(card_info, m_equipList);
        name_item->setIcon(suit_icon);
        name_item->setData(Qt::UserRole, card->getId());
    }

    foreach(int hand_id, m_playersHandcards[name]) {
        const Card *card = Sanguosha->getEngineCard(hand_id);
        QString card_name = Sanguosha->translate(card->objectName());
        QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(card->getSuitString()));
        QString point = card->getNumberString();

        QString card_info = point + "  " + card_name + "\t\t" + Sanguosha->translate(card->getSubtype());
        QListWidgetItem *name_item = new QListWidgetItem(card_info, m_handList);
        name_item->setIcon(suit_icon);
        name_item->setData(Qt::UserRole, card->getId());
    }

    foreach(int judge_id, m_playersJudges[name]) {
        const Card *card = Sanguosha->getEngineCard(judge_id);
        QString card_name = Sanguosha->translate(card->objectName());
        QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(card->getSuitString()));
        QString point = card->getNumberString();

        QString card_info = point + "  " + card_name + "\t\t" + Sanguosha->translate(card->getSubtype());
        QListWidgetItem *name_item = new QListWidgetItem(card_info, m_judgeList);
        name_item->setIcon(suit_icon);
        name_item->setData(Qt::UserRole, card->getId());
    }

    m_equipList->setCurrentRow(0);
    m_handList->setCurrentRow(0);
    m_judgeList->setCurrentRow(0);

    for (int i = 0; i < m_markIcons.length(); i++)
        m_markIcons.at(i)->hide();

    foreach(QString mark, m_playersMarks[name].keys()) {
        if (m_playersMarks[name][mark] > 0) {
            for (int i = 0; i < m_markIcons.length(); i++) {
                if (m_markIcons.at(i)->objectName() == mark) {
                    m_markIcons.at(i)->show();
                    break;
                }
            }
        }
    }
}

void CustomAssignDialog::updatePileInfo(int row) {
    if (row >= 0) {
        if (m_movePileCheck->isChecked()) {
            m_moveListUpButton->setEnabled(row != 0);
            m_moveListDownButton->setEnabled(row != m_pileList->count() - 1);
        }
        return;
    }

    if (row == -1)
        return;

    m_pileList->clear();

    m_removePileButton->setDisabled(m_settedPile.isEmpty());
    m_endedByPileCheckBox->setDisabled(m_settedPile.isEmpty());

    foreach(int card_id, m_settedPile) {
        const Card *card = Sanguosha->getEngineCard(card_id);
        QString card_name = Sanguosha->translate(card->objectName());
        QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(card->getSuitString()));
        QString point = card->getNumberString();

        QString card_info = point + "  " + card_name + "\t\t" + Sanguosha->translate(card->getSubtype());
        QListWidgetItem *name_item = new QListWidgetItem(card_info, m_pileList);
        name_item->setIcon(suit_icon);
        name_item->setData(Qt::UserRole, card->getId());
    }

    if (m_pileList->count() > 0)
        m_pileList->setCurrentRow(0);

}

void CustomAssignDialog::updatePlayerHpInfo(QString name) {
    if (m_playersHp.value(name, 0) != 0) {
        m_hpSpin->setValue(m_playersHp[name]);
        m_hpPrompt->setChecked(true);
    }
    else {
        m_hpPrompt->setChecked(false);
    }

    if (m_playersMaxHp.value(name, 0) != 0) {
        m_maxHpSpin->setValue(m_playersMaxHp[name]);
        m_maxHpPrompt->setChecked(true);
    }
    else {
        m_maxHpPrompt->setChecked(false);
    }
}

void CustomAssignDialog::updateAllKingdoms(bool) {
    for (int i = 0; i < m_list->count(); i++) {
        QString name = m_playerMap[i];
        QString kingdom = m_assignedNationality[name];
        m_itemMap[i]->setText(setListText(name, kingdom, i));
    }
}

void CustomAssignDialog::getPlayerHp(int hp)
{
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    m_playersHp[name] = hp;
}

void CustomAssignDialog::getPlayerMaxHp(int maxhp) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    m_playersMaxHp[name] = maxhp;
    m_hpSpin->setRange(1, maxhp);
}

void CustomAssignDialog::setPlayerHpEnabled(bool toggled) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    if (!toggled)
        m_playersHp.remove(name);
    else
        m_playersHp[name] = m_hpSpin->value();
}

void CustomAssignDialog::setPlayerMaxHpEnabled(bool toggled) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    if (!toggled)
        m_playersMaxHp.remove(name);
    else
        m_playersMaxHp[name] = m_maxHpSpin->value();
}

void CustomAssignDialog::setPlayerStartDraw(int draw_num) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    m_playerDrawNumWhenStarts[name] = draw_num;
}

void CustomAssignDialog::setStarter(bool toggled) {
    if (toggled)
        m_starter = m_list->currentItem()->data(Qt::UserRole).toString();
    else
        m_starter.clear();
}

void CustomAssignDialog::setPlayerMarks(int value) {
    QString mark_name = m_marksComboBox->itemData(m_marksComboBox->currentIndex()).toString();
    QString player_name = m_list->item(m_list->currentRow())->data(Qt::UserRole).toString();
    m_playersMarks[player_name][mark_name] = value;

    for (int i = 0; i < m_markIcons.length(); i++) {
        if (m_markIcons.at(i)->objectName() == mark_name) {
            if (value > 0)
                m_markIcons.at(i)->show();
            else
                m_markIcons.at(i)->hide();
            break;
        }
    }
}

void CustomAssignDialog::getPlayerMarks(int index) {
    QString mark_name = m_marksComboBox->itemData(index).toString();
    QString player_name = m_list->item(m_list->currentRow())->data(Qt::UserRole).toString();

    m_marksCount->setEnabled(!mark_name.isEmpty());
    m_marksCount->setValue(m_playersMarks[player_name][mark_name]);
}

void CustomAssignDialog::updateKingdom(int index) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    QString kingdom = m_settedNationality[name] == true ? nationalitiesComboBox->itemData(index).toString() : "default";
    setListText(name, kingdom, m_list->currentRow());
    m_assignedNationality[name] = kingdom;
}

void CustomAssignDialog::removeEquipCard() {
    int card_id = m_equipList->currentItem()->data(Qt::UserRole).toInt();
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    if (m_playersEquips[name].contains(card_id)) {
        m_playersEquips[name].removeOne(card_id);
        int row = m_equipList->currentRow();
        m_equipList->takeItem(row);
        if (m_equipList->count() > 0)
            m_equipList->setCurrentRow(row >= m_equipList->count() ? row - 1 : row);
        else
            m_removeEquipButton->setEnabled(false);
    }
}

void CustomAssignDialog::removeHandCard() {
    int card_id = m_handList->currentItem()->data(Qt::UserRole).toInt();
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    if (m_playersHandcards[name].contains(card_id)) {
        m_playersHandcards[name].removeOne(card_id);
        int row = m_handList->currentRow();
        m_handList->takeItem(row);
        if (m_handList->count() > 0)
            m_handList->setCurrentRow(row >= m_handList->count() ? row - 1 : row);
        else
            m_removeHandButton->setEnabled(false);
    }
}

void CustomAssignDialog::removeJudgeCard() {
    int card_id = m_judgeList->currentItem()->data(Qt::UserRole).toInt();
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    if (m_playersJudges[name].contains(card_id)) {
        m_playersJudges[name].removeOne(card_id);
        int row = m_judgeList->currentRow();
        m_judgeList->takeItem(row);
        if (m_judgeList->count() > 0)
            m_judgeList->setCurrentRow(row >= m_judgeList->count() ? row - 1 : row);
        else
            m_removeJudgeButton->setEnabled(false);
    }
}

void CustomAssignDialog::removePileCard() {
    int card_id = m_pileList->currentItem()->data(Qt::UserRole).toInt();
    if (m_settedPile.contains(card_id)) {
        int row = m_pileList->currentRow();
        m_pileList->takeItem(row);
        if (m_pileList->count() > 0)
            m_pileList->setCurrentRow(row >= m_pileList->count() ? row - 1 : row);
        else {
            m_removePileButton->setEnabled(false);
            m_endedByPileCheckBox->setEnabled(false);
            m_endedByPileCheckBox->setChecked(false);
        }
        m_settedPile.removeOne(card_id);
    }
}

void CustomAssignDialog::doGeneralAssign() {
    m_enableGeneral2 = false;
    GeneralAssignDialog *dialog = new GeneralAssignDialog(this);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(generalChosen(QString)), this, SLOT(getChosenGeneral(QString)));
    dialog->exec();
}

void CustomAssignDialog::doGeneralAssign2() {
    m_enableGeneral2 = true;
    GeneralAssignDialog *dialog = new GeneralAssignDialog(this, true);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(generalChosen(QString)), this, SLOT(getChosenGeneral(QString)));
    connect(dialog, SIGNAL(generalCleared()), this, SLOT(clearGeneral2()));
    dialog->exec();
}

void CustomAssignDialog::setMoveButtonAvaliable(bool toggled) {
    if (sender()->objectName() == "list check") {
        m_movePileCheck->setChecked(false);
        m_moveListCheck->setChecked(toggled);
        if (toggled) {
            m_moveListUpButton->setEnabled(m_list->currentRow() != 0);
            m_moveListDownButton->setEnabled(m_list->currentRow() != m_list->count() - 1);
        }
    }
    else {
        m_moveListCheck->setChecked(false);
        m_movePileCheck->setChecked(toggled);
        if (toggled) {
            m_moveListUpButton->setEnabled(m_pileList->count() > 0 && m_pileList->currentRow() != 0);
            m_moveListDownButton->setEnabled(m_pileList->count() > 0 && m_pileList->currentRow() != m_pileList->count() - 1);
        }
    }

    if (!m_moveListCheck->isChecked() && !m_movePileCheck->isChecked()) {
        m_moveListUpButton->setEnabled(false);
        m_moveListDownButton->setEnabled(false);
    }
}

void CustomAssignDialog::accept() {
    if (save("etc/customScenes/custom_scenario.txt")) {
        const Scenario *scene = Sanguosha->getScenario("custom_scenario");
        MiniSceneRule *rule = qobject_cast<MiniSceneRule *>(scene->getRule());
        Q_ASSERT(rule != NULL);
        rule->loadSetting("etc/customScenes/custom_scenario.txt");
        emit scenario_changed();
        QDialog::accept();
    }
}

void CustomAssignDialog::reject() {
    QDialog::reject();
}

void CustomAssignDialog::clearGeneral2() {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    m_generalMap2[name].clear();

    m_generalLabel2->setPixmap(QPixmap("image/system/disabled.png"));
}

void CustomAssignDialog::getChosenGeneral(QString generalName) {
    if (m_enableGeneral2) {
        const General *general2 = Sanguosha->getGeneral(generalName);
        QPixmap pixmap = G_ROOM_SKIN.getGeneralPixmap(general2->objectName(), QSanRoomSkin::S_GENERAL_ICON_SIZE_TINY);
        pixmap = pixmap.scaled(G_COMMON_LAYOUT.m_tinyAvatarSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_generalLabel2->setPixmap(pixmap);
        if (m_list->currentItem())
            m_generalMap2[m_list->currentItem()->data(Qt::UserRole).toString()] = generalName;
    } else {
        QPixmap pixmap = G_ROOM_SKIN.getGeneralPixmap(generalName, QSanRoomSkin::S_GENERAL_ICON_SIZE_TINY);
        pixmap = pixmap.scaled(G_COMMON_LAYOUT.m_tinyAvatarSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_generalLabel->setPixmap(pixmap);
        if (m_list->currentItem())
            m_generalMap[m_list->currentItem()->data(Qt::UserRole).toString()] = generalName;
    }
}

void CustomAssignDialog::freeChoose(bool toggled) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    m_canChooseGeneralFreely[name] = toggled;
}

void CustomAssignDialog::freeChoose2(bool toggled) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    m_canChooseGeneral2Freely[name] = toggled;
}

void CustomAssignDialog::doPlayerChains(bool toggled) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    m_playerIsChained[name] = toggled;
}

void CustomAssignDialog::doPlayerTurns(bool toggled) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    m_playerIsTurned[name] = toggled;
}

void CustomAssignDialog::doSkillSelect() {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    SkillAssignDialog *dialog = new SkillAssignDialog(this, name, m_playersExtraSkills[name]);

    connect(dialog, SIGNAL(skillUpdated(QStringList)), this, SLOT(updatePlayerExSkills(QStringList)));
    dialog->exec();
}

void CustomAssignDialog::updatePlayerExSkills(QStringList updatedSkills) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    m_playersExtraSkills[name].clear();
    m_playersExtraSkills[name].append(updatedSkills);
}

void CustomAssignDialog::exchangeListItem() {
    int first_index = -1, second_index = -1;
    if (m_moveListCheck->isChecked())
        first_index = m_list->currentRow();
    else if (m_movePileCheck->isChecked())
        first_index = m_pileList->currentRow();

    if (sender()->objectName() == "list_up")
        second_index = first_index - 1;
    else if (sender()->objectName() == "list_down")
        second_index = first_index + 1;

    if (first_index < 0 && second_index < 0)
        return;

    if (m_moveListCheck->isChecked()) {
        exchangePlayersInfo(m_itemMap[first_index], m_itemMap[second_index]);
        updateListItems();
        int row = m_list->count();
        m_list->clear();
        for (int i = 0; i < row; i++)
            m_list->addItem(m_itemMap[i]);
        m_list->setCurrentRow(second_index);
    }
    else if (m_movePileCheck->isChecked()) {
        int id1 = m_pileList->item(first_index)->data(Qt::UserRole).toInt();
        int id2 = m_pileList->item(second_index)->data(Qt::UserRole).toInt();

        m_settedPile.swap(m_settedPile.indexOf(id1), m_settedPile.indexOf(id2));
        updatePileInfo();
        m_pileList->setCurrentRow(second_index);
    }
}

void CustomAssignDialog::on_list_itemSelectionChanged(QListWidgetItem *current) {
    if (m_list->count() == 0 || current == NULL) return;

    QString player_name = current->data(Qt::UserRole).toString();
    if (!m_generalMap.value(player_name, "").isEmpty()) {
        QPixmap pixmap = G_ROOM_SKIN.getGeneralPixmap(m_generalMap.value(player_name), QSanRoomSkin::S_GENERAL_ICON_SIZE_TINY);
        pixmap = pixmap.scaled(G_COMMON_LAYOUT.m_tinyAvatarSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_generalLabel->setPixmap(pixmap);
    }
    else
        m_generalLabel->setPixmap(QPixmap(QString("image/system/disabled.png")));


    if (!m_generalMap2.value(player_name, "").isEmpty()) {
        QPixmap pixmap = G_ROOM_SKIN.getGeneralPixmap(m_generalMap2.value(player_name), QSanRoomSkin::S_GENERAL_ICON_SIZE_TINY);
        pixmap = pixmap.scaled(G_COMMON_LAYOUT.m_tinyAvatarSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_generalLabel2->setPixmap(pixmap);
    }
    else
        m_generalLabel2->setPixmap(QPixmap(QString("image/system/disabled.png")));

    if (!m_assignedNationality[player_name].isEmpty()) {
        for (int i = 0; i < nationalitiesComboBox->count(); i++) {
            if (m_assignedNationality[player_name] == nationalitiesComboBox->itemData(i).toString()) {
                nationalitiesComboBox->setCurrentIndex(i);
                updateKingdom(i);
                break;
            }
        }
    }

    m_selfSelectGeneral->setChecked(m_canChooseGeneralFreely[player_name]);
    m_selfSelectGeneral2->setChecked(m_canChooseGeneral2Freely[player_name]);

    m_headShownSetter->setChecked(m_playerHasShownHead.value(player_name, false));
    m_deputyShownSetter->setChecked(m_playerHasShownDeputy.value(player_name, false));

    m_turnedSetter->setChecked(m_playerIsTurned.value(player_name, false));
    m_chainedSetter->setChecked(m_playerIsChained.value(player_name, false));

    m_endedByPileCheckBox->setChecked(m_isEndedByPile);
    m_singleTurnCheckBox->setChecked(m_isSingleTurn);
    m_beforeNextCheckBox->setChecked(m_isBeforeNext);

    if (m_moveListCheck->isChecked()) {
        m_moveListUpButton->setEnabled(m_list->currentRow() != 0);
        m_moveListDownButton->setEnabled(m_list->currentRow() != m_list->count() - 1);
    }

    int val = 4;
    if (m_playerDrawNumWhenStarts.contains(player_name)) val = m_playerDrawNumWhenStarts[player_name];
    m_playerDraw->setValue(val);

    m_starterCheckBox->setEnabled(m_starter.isEmpty() || m_starter == player_name);

    QString kingdom = m_assignedNationality.value(player_name, "");
    if (!kingdom.isEmpty())
        nationalitiesComboBox->setCurrentIndex(m_kingdomIndex[kingdom]);

    m_nationalityIsSelectableCheckBox->setChecked(m_settedNationality.value(player_name, false));

    QString mark_name = m_marksComboBox->itemData(m_marksComboBox->currentIndex()).toString();
    if (!mark_name.isEmpty())
        m_marksCount->setValue(m_playersMarks.value(player_name)[mark_name]);
    else
        m_marksCount->setValue(0);

    updatePlayerInfo(player_name);
    updatePlayerHpInfo(player_name);
}

void CustomAssignDialog::checkBeforeNextBox(bool toggled) {
    if (toggled) {
        m_beforeNextCheckBox->setChecked(false);
        m_isBeforeNext = false;
        m_isSingleTurn = true;

        m_singleTurnText->show();
        m_singleTurnText2->show();
        m_singleTurnBox->show();
    }
    else {
        m_isSingleTurn = false;

        m_singleTurnText->hide();
        m_singleTurnText2->hide();
        m_singleTurnBox->hide();
    }
}

void CustomAssignDialog::checkSingleTurnBox(bool toggled) {
    if (toggled) {
        m_singleTurnCheckBox->setChecked(false);
        m_isBeforeNext = true;
        m_isSingleTurn = false;

        m_beforeNextText->show();
        m_beforeNextText2->show();
        m_beforeNextBox->show();
    }
    else {
        m_isBeforeNext = false;

        m_beforeNextText->hide();
        m_beforeNextText2->hide();
        m_beforeNextBox->hide();
    }
}

void CustomAssignDialog::checkEndedByPileBox(bool toggled) {
    if (toggled) {
        m_isEndedByPile = true;

        m_endedByPileText->show();
        m_endedByPileText2->show();
        m_endedByPileBox->show();
    }
    else {
        m_isEndedByPile = false;

        m_endedByPileText->hide();
        m_endedByPileText2->hide();
        m_endedByPileBox->hide();
    }
}

void CustomAssignDialog::load() {
    QString filename;
    if (sender()->objectName() == "default_load") filename = "etc/customScenes/custom_scenario.txt";
    else filename = QFileDialog::getOpenFileName(this,
        tr("Open mini scenario settings"),
        "etc/customScenes",
        tr("Pure text replay file (*.txt)"));

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    m_settedPile.clear();
    m_itemMap.clear();
    m_generalMap.clear();
    m_generalMap2.clear();
    m_playersMaxHp.clear();
    m_playersHp.clear();
    m_playerDrawNumWhenStarts.clear();
    m_playerIsChained.clear();
    m_playerIsTurned.clear();
    m_playersMarks.clear();
    m_playersExtraSkills.clear();
    m_playersHandcards.clear();
    m_playersEquips.clear();
    m_playersJudges.clear();
    m_settedNationality.clear();
    m_assignedNationality.clear();
    m_playerHasShownHead.clear();
    m_playerHasShownDeputy.clear();

    m_canChooseGeneralFreely.clear();
    m_canChooseGeneral2Freely.clear();

    m_isEndedByPile = false;
    m_isSingleTurn = false;
    m_isBeforeNext = false;

    int i = 0;
    for (i = 0; i < m_markIcons.length(); i++)
        m_markIcons.at(i)->hide();

    QTextStream in(&file);
    int numPlayer = 0;
    QMap<QString, int> role_index;
    role_index["lord+loyalist"] = 0;
    role_index["renegade"] = 1;
    role_index["rebel"] = 2;

    QList<QString> options;
    while (!in.atEnd()) {
        QString line = in.readLine();
        line = line.trimmed();
        if (line.isEmpty()) continue;

        if (!line.startsWith("setPile:") && !line.startsWith("extraOptions:") && !line.startsWith("general:")) {
            QMessageBox::warning(this, tr("Warning"), tr("Data is unreadable"));
            file.close();
            return;
        }

        if (line.startsWith("setPile:")) {
            QStringList list = line.remove("setPile:").split(",");
            foreach(QString id, list)
                m_settedPile.prepend(id.toInt());
            continue;
        }
        else if (line.startsWith("extraOptions:")) {
            line.remove("extraOptions:");

            foreach(QString option, line.split(" ")) {
                if (option.isEmpty()) continue;
                options << option;
            }
            continue;
        }

        QString name = numPlayer == 0 ? "Player" : QString("AI%1").arg(numPlayer);

        QMap<QString, QString> player;
        QStringList features;
        if (line.contains("|"))
            features = line.split("|");
        else
            features = line.split(" ");
        foreach(QString str, features) {
            QStringList keys = str.split(":");
            if (keys.size() < 2) continue;
            if (keys.first().size() < 1) continue;
            player.insert(keys.at(0), keys.at(1));
        }

        if (player["general"] == "select")
            m_canChooseGeneralFreely[name] = true;
        else if (player["general"] != QString())
            m_generalMap[name] = player["general"];

        if (player["general2"] == "select")
            m_canChooseGeneral2Freely[name] = true;
        else if (player["general2"] != QString())
            m_generalMap2[name] = player["general2"];

        if (player["maxhp"] != QString()) {
            m_playersMaxHp[name] = player["maxhp"].toInt();
            if (m_playersHp[name] > m_playersMaxHp[name])
                m_playersHp[name] = m_playersMaxHp[name];
        }
        if (player["hp"] != QString())
            m_playersHp[name] = player["hp"].toInt();
        if (player["draw"] != QString())
            m_playerDrawNumWhenStarts[name] = player["draw"].toInt();
        else
            m_playerDrawNumWhenStarts[name] = 4;

        if (player["starter"] != QString()) m_starter = name;
        if (player["chained"] != QString()) m_playerIsChained[name] = true;
        if (player["turned"] != QString()) m_playerIsTurned[name] = true;
        if (player["shown_head"] != QString()) m_playerHasShownHead[name] = true;
        if (player["shown_deputy"] != QString()) m_playerHasShownDeputy[name] = true;
        if (player["nationality"] != QString()) {
            m_assignedNationality[name] = player["nationality"];
            m_settedNationality[name] = true;
        }
        else {
            m_settedNationality[name] = false;
        }
        if (player["acquireSkills"] != QString()) {
            QStringList skills;
            foreach(QString skill_name, player["acquireSkills"].split(","))
                skills << skill_name;

            m_playersExtraSkills[name].append(skills);
        }
        if (player["endedByPile"] != QString()) {
            m_endedByPileBox->setCurrentIndex(role_index.value(player["endedByPile"], 0));
            m_isEndedByPile = true;
        }
        if (player["singleTurn"] != QString()) {
            m_singleTurnBox->setCurrentIndex(role_index.value(player["singleTurn"], 0));
            m_isSingleTurn = true;
        }
        if (player["beforeNext"] != QString()) {
            m_beforeNextBox->setCurrentIndex(role_index.value(player["beforeNext"], 0));
            m_isBeforeNext = true;
        }
        if (player["marks"] != QString()) {
            foreach(QString mark, player["marks"].split(",")) {
                QString mark_name = mark.split("*").at(0);
                int mark_number = mark.split("*").at(1).toInt();
                m_playersMarks[name][mark_name] = mark_number;
            }
        }

        if (player["hand"] != QString()) {
            foreach(QString id, player["hand"].split(",")) {
                bool ok;
                int num = id.toInt(&ok);
                if (!ok) {
                    for (int i = 0; i < Sanguosha->getCardCount(); i++) {
                        if (Sanguosha->getEngineCard(i)->objectName() == id) {
                            m_playersHandcards[name].prepend(i);
                            break;
                        }
                    }
                }
                else
                    m_playersHandcards[name].prepend(num);
            }
        }

        if (player["equip"] != QString()) {
            foreach(QString id, player["equip"].split(",")) {
                bool ok;
                int num = id.toInt(&ok);
                if (!ok) {
                    for (int i = 0; i < Sanguosha->getCardCount(); i++) {
                        if (Sanguosha->getEngineCard(i)->objectName() == id) {
                            m_playersEquips[name].prepend(i);
                            break;
                        }
                    }
                }
                else
                    m_playersEquips[name].prepend(num);
            }
        }

        if (player["judge"] != QString()) {
            foreach(QString id, player["judge"].split(",")) {
                bool ok;
                int num = id.toInt(&ok);
                if (!ok) {
                    for (int i = 0; i < Sanguosha->getCardCount(); i++) {
                        if (Sanguosha->getEngineCard(i)->objectName() == id) {
                            m_playersJudges[name].prepend(i);
                            break;
                        }
                    }
                }
                else
                    m_playersJudges[name].prepend(num);
            }
        }

        updateListItems();
        numPlayer++;
    }

    updateNumber(numPlayer - 2);
    for (int i = m_list->count() - 1; i >= 0; i--) {
        m_list->setCurrentItem(m_list->item(i));
        if (m_list->item(i)->data(Qt::UserRole).toString() == m_starter)
            m_starterCheckBox->setChecked(true);
    }
    m_list->setCurrentRow(0);

    m_playerDraw->setValue(m_playerDrawNumWhenStarts[m_list->currentItem()->data(Qt::UserRole).toString()]);
    m_numComboBox->setCurrentIndex(m_list->count() - 2);

    m_randomRolesBox->setChecked(options.contains(MiniSceneRule::S_EXTRA_OPTION_RANDOM_ROLES));
    m_restInDpBox->setChecked(options.contains(MiniSceneRule::S_EXTRA_OPTION_REST_IN_DISCARD_PILE));

    updatePileInfo();
    file.close();
}

bool CustomAssignDialog::save(QString path) {
    if (m_starter.isEmpty()) {
        QMessageBox::warning(NULL, tr("Warning"), tr("There is not a starter"));
        return false;
    }

    QString line;

    m_settedOptions << m_randomRolesBox->isChecked() << m_restInDpBox->isChecked();
    foreach(bool option, m_settedOptions) {
        if (option) {
            line.append("extraOptions:");
            break;
        }
    }
    if (m_randomRolesBox->isChecked()) {
        line.append(MiniSceneRule::S_EXTRA_OPTION_RANDOM_ROLES);
        line.append(" ");
    }
    if (m_restInDpBox->isChecked()) {
        line.append(MiniSceneRule::S_EXTRA_OPTION_REST_IN_DISCARD_PILE);
        line.append(" ");
    }
    line.remove(line.length() - 1, 1);
    line.append("\n");

    if (m_settedPile.length()) {
        line.append("setPile:");
        for (int i = m_settedPile.length() - 1; i >= 0; i--) {
            int id = m_settedPile.at(i);
            line.append(QString::number(id));
            line.append(",");
        }
        line.remove(line.length() - 1, 1);
        line.append("\n");
    }

    for (int i = 0; i < m_list->count(); i++) {
        QString name = (i == 0) ? "Player" : QString("AI%1").arg(QString::number(i));

        if (m_canChooseGeneralFreely[name] || m_generalMap[name].isEmpty())
            line.append("general:select ");
        else
            line.append(QString("general:%1 ").arg(m_generalMap[name]));

        if (m_canChooseGeneral2Freely[name] || m_generalMap2[name].isEmpty())
            line.append("general2:select ");
        else if (!m_generalMap2[name].isEmpty())
            line.append(QString("general2:%1 ").arg(m_generalMap2[name]));

        if (m_starter == name) line.append("starter:true ");
        if (!m_playersMarks[name].isEmpty()) {
            line.append("marks:");
            QMap<QString, int> marks = m_playersMarks[name];
            foreach(QString mark_name, marks.keys()) {
                if (marks.value(mark_name) > 0)
                    line.append(QString("%1*%2,").arg(mark_name).arg(QString::number(marks.value(mark_name))));
            }

            if (line.endsWith("marks:"))
                line.remove(line.length() - 7, 6);
            else {
                line.remove(line.length() - 1, 1);
                line.append(" ");
            }
        }
        if (m_playersMaxHp[name] > 0) line.append(QString("maxhp:%1 ").arg(m_playersMaxHp[name]));
        if (m_playersHp[name] > 0) line.append(QString("hp:%1 ").arg(m_playersHp[name]));
        if (m_playerIsTurned[name]) line.append("turned:true ");
        if (m_playerIsChained[name]) line.append("chained:true ");
        if (m_playerHasShownHead[name]) line.append("shown_head:true ");
        if (m_playerHasShownDeputy[name]) line.append("shown_deputy:true ");
        if (m_settedNationality[name]) line.append(QString("nationality:%1 ").arg(m_assignedNationality[name]));
        if (m_playersExtraSkills[name].length() > 0) {
            line.append("acquireSkills:");
            foreach(QString skill_name, m_playersExtraSkills[name]) {
                line.append(skill_name + ",");
            }
            line.remove(line.length() - 1, 1);
            line.append(" ");
        }
        if (i == 0) {
            if (m_isEndedByPile) {
                QString winner = m_endedByPileBox->itemData(m_endedByPileBox->currentIndex()).toString();
                line.append(QString("endedByPile:%1 ").arg(winner));
            }
            if (m_isSingleTurn) {
                QString winner = m_singleTurnBox->itemData(m_singleTurnBox->currentIndex()).toString();
                line.append(QString("singleTurn:%1 ").arg(winner));
            }
            else if (m_isBeforeNext) {
                QString winner = m_beforeNextBox->itemData(m_beforeNextBox->currentIndex()).toString();
                line.append(QString("beforeNext:%1 ").arg(winner));
            }
        }
        if (m_playerDrawNumWhenStarts.contains(name) && m_playerDrawNumWhenStarts[name] != 4)
            line.append(QString("draw:%1 ").arg(m_playerDrawNumWhenStarts[name]));

        if (m_playersEquips[name].length()) {
            line.append("equip:");
            foreach(int equip, m_playersEquips[name]) line.append(QString("%1,").arg(equip));
            line.chop(1);
            line.append(" ");
        }

        if (m_playersHandcards[name].length()) {
            line.append("hand:");
            foreach(int hand, m_playersHandcards[name]) line.append(QString("%1,").arg(hand));
            line.chop(1);
            line.append(" ");
        }

        if (m_playersJudges[name].length()) {
            line.append("judge:");
            foreach(int judge, m_playersJudges[name]) line.append(QString("%1,").arg(judge));
            line.chop(1);
            line.append(" ");
        }

        line.append("\n");
    }

    QString filename = path;
    if (path.size() < 1)
        filename = QFileDialog::getSaveFileName(this,
        tr("Save mini scenario settings"),
        "etc/customScenes/",
        tr("Pure text replay file (*.txt)"));

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;


    QTextStream out(&file);
    out << line;
    file.close();
    return true;
}

//---------------------------------------

GeneralAssignDialog::GeneralAssignDialog(QWidget *parent, bool canBan)
    : FlatDialog(parent) {
    setWindowTitle(tr("Mini choose generals"));

    QTabWidget *tab_widget = new QTabWidget;

    group = new QButtonGroup(this);
    group->setExclusive(true);

    QList<const General *> all_generals = Sanguosha->getGeneralList();
    QMap<QString, QList<const General *> > map;
    foreach(const General *general, all_generals)
        map[general->getKingdom()] << general;

    QStringList kingdoms = Sanguosha->getKingdoms();

    foreach(QString kingdom, kingdoms) {
        QList<const General *> generals = map[kingdom];

        if (!generals.isEmpty()) {
            QWidget *tab = createTab(generals);
            tab_widget->addTab(tab,
                QIcon(QString("image/kingdom/icon/%1.png").arg(kingdom)),
                Sanguosha->translate(kingdom));
        }
    }

    QPushButton *ok_button = new QPushButton(tr("OK"));
    connect(ok_button, SIGNAL(clicked()), this, SLOT(chooseGeneral()));

    QPushButton *cancel_button = new QPushButton(tr("Cancel"));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *button_layout = new QHBoxLayout;
    if (canBan) {
        QPushButton *clear_button = new QPushButton(tr("Clear General"));
        connect(clear_button, SIGNAL(clicked()), this, SLOT(clearGeneral()));

        button_layout->addWidget(clear_button);
    }

    button_layout->addStretch();
    button_layout->addWidget(ok_button);
    button_layout->addWidget(cancel_button);

    layout->addWidget(tab_widget);
    layout->addLayout(button_layout);

    group->buttons().first()->click();
}

QWidget *GeneralAssignDialog::createTab(const QList<const General *> &generals) {
    QWidget *tab = new QWidget;

    QGridLayout *layout = new QGridLayout;
    layout->setOriginCorner(Qt::TopLeftCorner);
    QIcon lord_icon("image/system/roles/lord.png");

    const int columns = 4;

    for (int i = 0; i < generals.length(); i++) {
        const General *general = generals.at(i);
        QString general_name = general->objectName();
        if (general->isTotallyHidden())
            continue;

        QString text = QString("%1[%2]")
            .arg(Sanguosha->translate(general_name))
            .arg(Sanguosha->translate(general->getPackage()));

        QAbstractButton *button;
        button = new QRadioButton(text);
        button->setObjectName(general_name);
        button->setToolTip(general->getSkillDescription(true));
        if (general->isLord())
            button->setIcon(lord_icon);

        group->addButton(button);

        int row = i / columns;
        int column = i % columns;
        layout->addWidget(button, row, column);
    }

    tab->setLayout(layout);
    return tab;
}

void GeneralAssignDialog::chooseGeneral() {
    QAbstractButton *button = group->checkedButton();
    if (button)
        emit generalChosen(button->objectName());
    this->reject();
}

void GeneralAssignDialog::clearGeneral() {
    emit generalCleared();
    this->reject();
}

//------------------------------

CardAssignDialog::CardAssignDialog(QWidget *parent, QString cardType, QString className, QList<int> excluded)
    : FlatDialog(parent), m_cardType(cardType), m_className(className),
    m_excludedCards(excluded)
{
    setWindowTitle(tr("Custom Card Chosen"));
    QVBoxLayout *vLayout = new QVBoxLayout;
    m_cardList = new QListWidget;
    stylizeScrollBars(m_cardList);

    updateCardList();

    QPushButton *getCardButton = new QPushButton(tr("Get card"));
    QPushButton *back = new QPushButton(tr("Back"));

    vLayout->addWidget(getCardButton);
    vLayout->addWidget(back);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_cardList);
    hLayout->addLayout(vLayout);
    layout->addLayout(hLayout);

    connect(back, SIGNAL(clicked()), this, SLOT(reject()));
    connect(getCardButton, SIGNAL(clicked()), this, SLOT(askCard()));
    connect(CustomInstance, SIGNAL(card_addin(int)), this, SLOT(updateExcluded(int)));
}

void CardAssignDialog::addCard(const Card *card) {
    QString name = Sanguosha->translate(card->objectName());
    QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(card->getSuitString()));
    QString point = card->getNumberString();

    QString card_info = point + "  " + name + "\t\t" + Sanguosha->translate(card->getSubtype());
    QListWidgetItem *name_item = new QListWidgetItem(card_info, m_cardList);
    name_item->setIcon(suit_icon);
    name_item->setData(Qt::UserRole, card->getId());
}

void CardAssignDialog::askCard() {
    QListWidgetItem *card_item = m_cardList->currentItem();
    int card_id = card_item->data(Qt::UserRole).toInt();
    emit cardChosen(card_id);

    int row = m_cardList->currentRow();
    int id = m_cardList->item(row)->data(Qt::UserRole).toInt();
    m_excludedCards << id;
    updateCardList();
    m_cardList->setCurrentRow(row >= m_cardList->count() ? row - 1 : row);
}

void CardAssignDialog::updateExcluded(int cardId) {
    m_excludedCards.removeOne(cardId);
}

void CardAssignDialog::updateCardList() {
    m_cardList->clear();

    int n = Sanguosha->getCardCount();
    QList<const Card *> reasonable_cards;
    if (!m_cardType.isEmpty() || !m_className.isEmpty()) {
        for (int i = 0; i < n; i++) {
            if (m_excludedCards.contains(i))
                continue;

            const Card *card = Sanguosha->getEngineCard(i);
            if (Config.BanPackages.contains(card->getPackage()))
                continue;
            if (card->getType() == m_cardType || card->isKindOf(m_className.toStdString().c_str()))
                reasonable_cards << card;
        }
    }
    else {
        for (int i = 0; i < n; i++) {
            if (m_excludedCards.contains(i))
                continue;

            const Card *card = Sanguosha->getEngineCard(i);
            if (Config.BanPackages.contains(card->getPackage()))
                continue;
            reasonable_cards << card;
        }
    }

    for (int i = 0; i < reasonable_cards.length(); i++)
        addCard(reasonable_cards.at(i));

    if (reasonable_cards.length() > 0)
        m_cardList->setCurrentRow(0);
}

//-----------------------------------

SkillAssignDialog::SkillAssignDialog(QDialog *parent, QString playerName, QStringList &playerSkills)
    : FlatDialog(parent), m_updatedSkills(playerSkills)
{
    setWindowTitle(tr("Skill Chosen"));
    QHBoxLayout *hLayout = new QHBoxLayout;
    m_skillList = new QListWidget;
    static const QString styleSheet = StyleHelper::styleSheetOfScrollBar();
    m_skillList->verticalScrollBar()->setStyleSheet(styleSheet);

    m_skillInput = new QLineEdit;
#if QT_VERSION >= 0x040700
    m_skillInput->setPlaceholderText(tr("Input the Skill Name"));
#endif
    m_skillInput->setToolTip(tr("<font color=%1>Internal skill name is a phonetic form, "
        "the rest of the special circumstances, "
        "please see the translation of documents in the lang directory.</font>").arg(Config.SkillDescriptionInToolTipColor.name()));

    QCompleter *completer = new QCompleter(Sanguosha->getSkillNames(), m_skillInput);
    m_skillInput->setCompleter(completer);

    QPushButton *addSkill = new QPushButton(tr("Add Skill"));
    addSkill->setObjectName("inline_add");

    m_selectSkillButton = new QPushButton(tr("Select Skill from Generals"));
    m_deleteSkillButton = new QPushButton(tr("Delete Current Skill"));

    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));

    m_skillInfo = new QTextEdit;
    m_skillInfo->verticalScrollBar()->setStyleSheet(styleSheet);
    m_skillInfo->setObjectName("skill_info");
    m_skillInfo->setReadOnly(true);

    updateSkillList();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(new QLabel(Sanguosha->translate(playerName)));
    vLayout->addWidget(m_skillList);
    hLayout->addLayout(vLayout);
    QVBoxLayout *sidedLayout = new QVBoxLayout;
    sidedLayout->addWidget(m_skillInfo);
    sidedLayout->addStretch();
    sidedLayout->addLayout(HLay(m_skillInput, addSkill));
    sidedLayout->addLayout(HLay(m_selectSkillButton, m_deleteSkillButton));
    sidedLayout->addLayout(HLay(okButton, cancelButton));
    hLayout->addLayout(sidedLayout);

    layout->addLayout(hLayout);

    connect(addSkill, SIGNAL(clicked()), this, SLOT(addSkill()));
    connect(m_selectSkillButton, SIGNAL(clicked()), this, SLOT(selectSkill()));
    connect(m_deleteSkillButton, SIGNAL(clicked()), this, SLOT(deleteSkill()));
    connect(m_skillList, SIGNAL(itemSelectionChanged()), this, SLOT(updateSkillInfo()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void SkillAssignDialog::updateSkillInfo() {
    QString skillName = m_skillList->currentItem()->data(Qt::UserRole).toString();
    m_skillInfo->clear();

    m_skillInfo->setText(Sanguosha->getSkill(skillName)->getDescription(false));
}

void SkillAssignDialog::selectSkill() {
    GeneralAssignDialog *dialog = new GeneralAssignDialog(this);

    connect(dialog, SIGNAL(generalChosen(QString)), this, SLOT(getSkillFromGeneral(QString)));
    dialog->exec();
}

void SkillAssignDialog::deleteSkill() {
    QString skill_name = m_skillList->currentItem()->data(Qt::UserRole).toString();
    m_updatedSkills.removeOne(skill_name);

    updateSkillList();
}

void SkillAssignDialog::getSkillFromGeneral(QString general_name) {
    QDialog *select_dialog = new QDialog(this);
    select_dialog->setWindowTitle(tr("Skill Chosen"));
    QVBoxLayout *layout = new QVBoxLayout;

    const General *general = Sanguosha->getGeneral(general_name);
    foreach(const Skill *skill, general->getVisibleSkillList()) {
        QCommandLinkButton *button = new QCommandLinkButton;
        button->setObjectName(skill->objectName());
        button->setText(Sanguosha->translate(skill->objectName()));
        button->setToolTip(Sanguosha->translate(":" + skill->objectName()));

        connect(button, SIGNAL(clicked()), select_dialog, SLOT(accept()));
        connect(button, SIGNAL(clicked()), this, SLOT(addSkill()));

        layout->addWidget(button);
    }

    select_dialog->setLayout(layout);
    select_dialog->exec();
}

void SkillAssignDialog::addSkill() {
    QString name = sender()->objectName();
    if (name == "inline_add") {
        name = m_skillInput->text();

        const Skill *skill = Sanguosha->getSkill(name);
        if (skill == NULL) {
            QMessageBox::warning(this, tr("Warning"), tr("There is no skill that internal name is %1").arg(name));
            return;
        }
    }

    if (!m_updatedSkills.contains(name)) {
        m_updatedSkills << name;
        updateSkillList();
    }

    m_skillInput->clear();
}

void SkillAssignDialog::updateSkillList() {
    int index = m_skillList->count() > 0 ? m_skillList->currentRow() : 0;

    m_skillList->clear();
    m_skillInfo->clear();

    foreach(QString skill_name, m_updatedSkills) {
        if (Sanguosha->getSkill(skill_name) != NULL) {
            QListWidgetItem *item = new QListWidgetItem(Sanguosha->translate(skill_name));
            item->setData(Qt::UserRole, skill_name);
            m_skillList->addItem(item);
        }
    }
    m_skillList->setCurrentRow(index >= m_skillList->count() ? m_skillList->count() - 1 : index);

    if (m_skillList->count() > 0) {
        updateSkillInfo();
        m_deleteSkillButton->setEnabled(true);
    }
    else
        m_deleteSkillButton->setEnabled(false);
}

void SkillAssignDialog::accept() {
    emit skillUpdated(m_updatedSkills);
    QDialog::accept();
}

void CustomAssignDialog::doPlayerShows(bool toggled) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    m_playerHasShownHead[name] = toggled;
}

void CustomAssignDialog::doPlayerShows2(bool toggled) {
    QString name = m_list->currentItem()->data(Qt::UserRole).toString();
    m_playerHasShownDeputy[name] = toggled;
}
