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

#include "generaloverview.h"
#include "engine.h"
#include "ui_generaloverview.h"
#include "settings.h"
#include "clientplayer.h"
#include "generalmodel.h"
#include "skinbank.h"
#include "stylehelper.h"

#include <QHBoxLayout>
#include <QGroupBox>
#include <QAbstractButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QCommandLinkButton>
#include <QClipboard>
#include <QMessageBox>
#include <QScrollBar>

struct SearchDetails {
    bool include_hidden;
    QString nickname;
    QString name;
    QStringList genders;
    QStringList kingdoms;
    int lower;
    int upper;
    QStringList packages;
};

static QLayout *HLay(QWidget *left, QWidget *right) {
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(left);
    layout->addWidget(right);
    return layout;
}

GeneralSearch::GeneralSearch(GeneralOverview *parent)
    : FlatDialog(parent)
{
    setWindowTitle(tr("Search..."));

    layout->addWidget(createInfoTab());
    layout->addLayout(createButtonLayout());

    connect(this, &GeneralSearch::search, parent, (void (GeneralOverview::*)(const SearchDetails &))(&GeneralOverview::startSearch));
}

QWidget *GeneralSearch::createInfoTab() {
    QVBoxLayout *layout = new QVBoxLayout;

    include_hidden_checkbox = new QCheckBox;
    include_hidden_checkbox->setText(tr("Include hidden generals"));
    include_hidden_checkbox->setChecked(true);
    layout->addWidget(include_hidden_checkbox);

    nickname_label = new QLabel(tr("Nickname"));
    nickname_label->setToolTip(tr("<font color=%1>Input characters included by the nickname. '?' and '*' is available. Every nickname meets the condition if the line is empty.</font>").arg(Config.SkillDescriptionInToolTipColor.name()));
    nickname_edit = new QLineEdit;
    nickname_edit->clear();
    layout->addLayout(HLay(nickname_label, nickname_edit));

    name_label = new QLabel(tr("Name"));
    name_label->setToolTip(tr("<font color=%1>Input characters included by the name. '?' and '*' is available. Every name meets the condition if the line is empty.</font>").arg(Config.SkillDescriptionInToolTipColor.name()));
    name_edit = new QLineEdit;
    name_edit->clear();
    layout->addLayout(HLay(name_label, name_edit));

    maxhp_lower_label = new QLabel(tr("MaxHp Min"));
    maxhp_lower_label->setToolTip(tr("<font color=%1>Set lowerlimit and upperlimit of max HP. 0 ~ 0 meets all conditions.</font>").arg(Config.SkillDescriptionInToolTipColor.name()));
    maxhp_upper_label = new QLabel(tr("MaxHp Max"));
    maxhp_upper_label->setToolTip(tr("<font color=%1>Set lowerlimit and upperlimit of max HP. 0 ~ 0 meets all conditions.</font>").arg(Config.SkillDescriptionInToolTipColor.name()));

    maxhp_lower_spinbox = new QSpinBox;
    maxhp_lower_spinbox->setRange(0, 10);
    maxhp_upper_spinbox = new QSpinBox;
    maxhp_upper_spinbox->setRange(0, 10);

    QHBoxLayout *maxhp_hlay = new QHBoxLayout;
    maxhp_hlay->addWidget(maxhp_lower_label);
    maxhp_hlay->addWidget(maxhp_lower_spinbox);
    maxhp_hlay->addWidget(maxhp_upper_label);
    maxhp_hlay->addWidget(maxhp_upper_spinbox);

    layout->addLayout(maxhp_hlay);

    QGroupBox *gender_group = new QGroupBox(tr("Gender"));
    gender_group->setToolTip(tr("<font color=%1>Select genders. Every gender meets the condition if none is selected.</font>").arg(Config.SkillDescriptionInToolTipColor.name()));
    gender_buttons = new QButtonGroup;
    gender_buttons->setExclusive(false);

    QCheckBox *male = new QCheckBox;
    male->setObjectName("male");
    male->setText(tr("Male"));
    male->setChecked(false);

    QCheckBox *female = new QCheckBox;
    female->setObjectName("female");
    female->setText(tr("Female"));
    female->setChecked(false);

    QCheckBox *genderless = new QCheckBox;
    genderless->setObjectName("nogender");
    genderless->setText(tr("NoGender"));
    genderless->setChecked(false);

    gender_buttons->addButton(male);
    gender_buttons->addButton(female);
    gender_buttons->addButton(genderless);

    QGridLayout *gender_layout = new QGridLayout;
    gender_group->setLayout(gender_layout);
    gender_layout->addWidget(male, 0, 1);
    gender_layout->addWidget(female, 0, 2);
    gender_layout->addWidget(genderless, 0, 3);

    layout->addWidget(gender_group);

    kingdom_buttons = new QButtonGroup;
    kingdom_buttons->setExclusive(false);

    QGroupBox *kingdom_box = new QGroupBox(tr("Kingdoms"));
    kingdom_box->setToolTip(tr("<font color=%1>Select kingdoms. Every kingdom meets the condition if none is selected.</font>").arg(Config.SkillDescriptionInToolTipColor.name()));

    QGridLayout *kingdom_layout = new QGridLayout;
    kingdom_box->setLayout(kingdom_layout);

    int i = 0;
    foreach(QString kingdom, Sanguosha->getKingdoms()) {
        QCheckBox *checkbox = new QCheckBox;
        checkbox->setObjectName(kingdom);
        checkbox->setIcon(QIcon(QString("image/kingdom/icon/%1.png").arg(kingdom)));
        checkbox->setChecked(false);

        kingdom_buttons->addButton(checkbox);

        int row = i / 5;
        int column = i % 5;
        i++;
        kingdom_layout->addWidget(checkbox, row, column + 1);
    }
    layout->addWidget(kingdom_box);

    package_buttons = new QButtonGroup;
    package_buttons->setExclusive(false);

    QStringList extensions = Sanguosha->getExtensions();

    QGroupBox *package_box = new QGroupBox(tr("Packages"));
    package_box->setToolTip(tr("<font color=%1>Select packages. Every package meets the condition if none is selected.</font>").arg(Config.SkillDescriptionInToolTipColor.name()));

    QVBoxLayout *package_layout = new QVBoxLayout;

    QHBoxLayout *package_button_layout = new QHBoxLayout;
    select_all_button = new QPushButton(tr("Select All"));
    connect(select_all_button, &QPushButton::clicked, this, &GeneralSearch::selectAllPackages);
    unselect_all_button = new QPushButton(tr("Unselect All"));
    connect(unselect_all_button, &QPushButton::clicked, this, &GeneralSearch::unselectAllPackages);
    package_button_layout->addWidget(select_all_button);
    package_button_layout->addWidget(unselect_all_button);
    package_button_layout->addStretch();

    QGridLayout *packages_layout = new QGridLayout;

    i = 0;
    foreach(QString extension, extensions) {
        const Package *package = Sanguosha->findChild<const Package *>(extension);
        if (package == NULL || package->getType() != Package::GeneralPack)
            continue;
        QCheckBox *checkbox = new QCheckBox;
        checkbox->setObjectName(extension);
        checkbox->setText(Sanguosha->translate(extension));
        checkbox->setChecked(false);

        package_buttons->addButton(checkbox);

        int row = i / 5;
        int column = i % 5;
        i++;
        packages_layout->addWidget(checkbox, row, column + 1);
    }
    package_layout->addLayout(package_button_layout);
    package_layout->addLayout(packages_layout);
    package_box->setLayout(package_layout);
    layout->addWidget(package_box);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    return widget;
}

QLayout *GeneralSearch::createButtonLayout() {
    QHBoxLayout *button_layout = new QHBoxLayout;

    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    QPushButton *clear_button = new QPushButton(tr("Clear"));
    QPushButton *ok_button = new QPushButton(tr("OK"));

    button_layout->addWidget(cancelButton);
    button_layout->addWidget(clear_button);
    button_layout->addWidget(ok_button);

    connect(cancelButton, &QPushButton::clicked, this, &GeneralSearch::reject);
    connect(ok_button, &QPushButton::clicked, this, &GeneralSearch::accept);
    connect(clear_button, &QPushButton::clicked, this, &GeneralSearch::clearAll);

    return button_layout;
}

void GeneralSearch::accept() {
    SearchDetails detail;
    detail.include_hidden = include_hidden_checkbox->isChecked();
    detail.nickname = nickname_edit->text();
    detail.name = name_edit->text();
    foreach(QAbstractButton *button, gender_buttons->buttons()) {
        if (button->isChecked())
            detail.genders << button->objectName();
    }
    foreach(QAbstractButton *button, kingdom_buttons->buttons()) {
        if (button->isChecked())
            detail.kingdoms << button->objectName();
    }
    detail.lower = maxhp_lower_spinbox->value();
    detail.upper = qMax(detail.lower, maxhp_upper_spinbox->value());
    foreach(QAbstractButton *button, package_buttons->buttons()) {
        if (button->isChecked())
            detail.packages << button->objectName();
    }
    emit search(detail);
    QDialog::accept();
}

void GeneralSearch::clearAll() {
    include_hidden_checkbox->setChecked(true);
    nickname_edit->clear();
    name_edit->clear();
    foreach(QAbstractButton *button, gender_buttons->buttons())
        button->setChecked(false);
    foreach(QAbstractButton *button, kingdom_buttons->buttons())
        button->setChecked(false);
    maxhp_lower_spinbox->setValue(0);
    maxhp_upper_spinbox->setValue(0);
    foreach(QAbstractButton *button, package_buttons->buttons())
        button->setChecked(false);
}

void GeneralSearch::selectAllPackages() {
    foreach(QAbstractButton *button, package_buttons->buttons())
        button->setChecked(true);
}

void GeneralSearch::unselectAllPackages() {
    foreach(QAbstractButton *button, package_buttons->buttons())
        button->setChecked(false);
}

static GeneralOverview *Overview;

GeneralOverview *GeneralOverview::getInstance(QWidget *main_window) {
    if (Overview == NULL)
        Overview = new GeneralOverview(main_window);

    return Overview;
}

GeneralOverview::GeneralOverview(QWidget *parent)
    : FlatDialog(parent, false), ui(new Ui::GeneralOverview), all_generals(NULL)
{
    ui->setupUi(this);
    origin_window_title = windowTitle();
    connect(this, &GeneralOverview::windowTitleChanged, ui->titleLabel, &QLabel::setText);
    connect(ui->closeButton, &QPushButton::clicked, this, &GeneralOverview::reject);

    const QString style = StyleHelper::styleSheetOfScrollBar();
    ui->tableView->verticalScrollBar()->setStyleSheet(style);
    ui->skillTextEdit->verticalScrollBar()->setStyleSheet(style);
    ui->scrollArea->verticalScrollBar()->setStyleSheet(style);

    button_layout = new QVBoxLayout;

    QGroupBox *group_box = new QGroupBox;
    group_box->setTitle(tr("Effects"));
    group_box->setLayout(button_layout);
    ui->scrollArea->setWidget(group_box);
    ui->skillTextEdit->setProperty("description", true);
    connect(ui->changeHeroSkinButton, &QPushButton::clicked, this, &GeneralOverview::showNextSkin);

    general_search = new GeneralSearch(this);
    connect(ui->searchButton, &QPushButton::clicked, general_search, &GeneralSearch::show);
    ui->returnButton->hide();
    connect(ui->returnButton, &QPushButton::clicked, this, &GeneralOverview::fillAllGenerals);
}

void GeneralOverview::fillGenerals(const QList<const General *> &generals, bool init) {
    QList<const General *> generalsCopy = generals;
    QMap<const General *, int> tempGeneralMap;
    foreach(const General *general, generalsCopy) {
        if (!general->isTotallyHidden()) {
            int skinId = 0;
            if (Self && Self->getGeneral()) {
                if (general == Self->getGeneral())
                    skinId = Self->getHeadSkinId();
                else if (general == Self->getGeneral2())
                    skinId = Self->getDeputySkinId();
            }
            if (skinId != 0)
                general->tryLoadingSkinTranslation(skinId);
            tempGeneralMap[general] = skinId;
        } else {
            generalsCopy.removeOne(general);
        }
    }

    if (tempGeneralMap.isEmpty())
        return;

    GeneralModel *model = new GeneralModel(tempGeneralMap, generalsCopy);

    if (init) {
        ui->returnButton->hide();
        setWindowTitle(origin_window_title);
        all_generals = model->generalMap();
    }

    model->setParent(this);
    ui->tableView->setModel(model);

    ui->tableView->setColumnWidth(0, 80);
    ui->tableView->setColumnWidth(1, 95);
    ui->tableView->setColumnWidth(2, 40);
    ui->tableView->setColumnWidth(3, 50);
    ui->tableView->setColumnWidth(4, 60);
    ui->tableView->setColumnWidth(5, 85);

    on_tableView_clicked(model->firstIndex());
}

void GeneralOverview::resetButtons() {
    QLayoutItem *child;
    while ((child = button_layout->takeAt(0))) {
        QWidget *widget = child->widget();
        if (widget)
            delete widget;
    }
}

GeneralOverview::~GeneralOverview() {
    delete ui;
}

bool GeneralOverview::hasSkin(const General *general) const
{
    const int skinId = all_generals->value(general);

    if (skinId == 0)
        return G_ROOM_SKIN.doesGeneralHaveSkin(general->objectName(), 1, true);

    return true;
}

QString GeneralOverview::getIllustratorInfo(const QString &generalName) {
    const int skinId = all_generals->value(Sanguosha->getGeneral(generalName));
    QString prefix = (skinId > 0) ? QString::number(skinId) : QString();
    QString illustratorText = Sanguosha->translate(QString("illustrator:%1%2").arg(prefix).arg(generalName));
    if (!illustratorText.startsWith("illustrator:")) {
        return illustratorText;
    } else {
        illustratorText = Sanguosha->translate("illustrator:" + generalName);
        if (!illustratorText.startsWith("illustrator:"))
            return illustratorText;
        else
            return Sanguosha->translate("DefaultIllustrator");
    }
}

QList<const Skill *> qsgs_allSkillsOf(const General *general)
{
    QList<const Skill *> skills = general->getVisibleSkillList();
    foreach(QString skill_name, general->getRelatedSkillNames()) {
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if (skill && skill->isVisible()) skills << skill;
    }
    return skills;
}

QString GeneralOverview::getCvInfo(const QString &generalName)
{
    const int skinId = all_generals->value(Sanguosha->getGeneral(generalName));
    QString prefix = (skinId > 0) ? QString::number(skinId) : QString();
    QString cvText = Sanguosha->translate(QString("cv:%1%2").arg(prefix).arg(generalName));
    if (!cvText.startsWith("cv:")) {
        return cvText;
    } else {
        cvText = Sanguosha->translate("cv:" + generalName);
        if (!cvText.startsWith("cv:"))
            return cvText;
        else
            return tr("Sanguosha OL");
    }
}

void GeneralOverview::addLines(const General *general, const Skill *skill) {
    QString skill_name = Sanguosha->translate(skill->objectName());

    const int skinId = all_generals->value(general);
    QStringList sources = skill->getSources(general->objectName(),
                                            all_generals->value(general));

    bool usingDefault = false;

    if (skinId != 0 && sources.isEmpty()) {
        sources = skill->getSources();
        usingDefault = true;
    }

    if (sources.isEmpty()) {
        QCommandLinkButton *button = new QCommandLinkButton(skill_name);

        button->setEnabled(false);
        button_layout->addWidget(button);
    } else {
        QRegExp rx(".+/(\\w+\\d?).ogg");
        for (int i = 0; i < sources.length(); i++) {
            QString source = sources[i];
            if (!rx.exactMatch(source))
                continue;

            QString button_text = skill_name;
            if (sources.length() != 1)
                button_text.append(QString(" (%1)").arg(i + 1));

            QCommandLinkButton *button = new QCommandLinkButton(button_text);
            button->setObjectName(source);
            button_layout->addWidget(button);

            const int skinId = all_generals->value(general);
            QString filename = rx.capturedTexts().at(1);
            QString skill_line;
            if (skinId == 0 || usingDefault)
                skill_line = Sanguosha->translate("$" + filename);
            else
                skill_line = Sanguosha->translate("$"+ QString::number(skinId) + filename);
            button->setDescription(skill_line);
            
            connect(button, &QCommandLinkButton::clicked, this, &GeneralOverview::playAudioEffect);

            addCopyAction(button);
        }
    }
}

void GeneralOverview::addDeathLine(const General *general)
{
    QString last_word;
    const int skinId = all_generals->value(general);
    const QString id = QString::number(skinId);
    if (skinId == 0)
        last_word = Sanguosha->translate("~" + general->objectName());
    else
        last_word = Sanguosha->translate("~" + id + general->objectName());

    if (last_word.startsWith("~")) {
        if (general->objectName().contains("_")) {
            const QString generalName = general->objectName().split("_").last();
            if (skinId == 0) {
                last_word = Sanguosha->translate(("~") + generalName);
            } else {
                last_word = Sanguosha->translate(("~") + id + generalName);
                if (last_word.startsWith("~"))
                    last_word = Sanguosha->translate(("~") + generalName);
            }
        } else if (skinId != 0) {
            last_word = Sanguosha->translate("~" + general->objectName());
        }
    }

    if (!last_word.startsWith("~")) {
        QCommandLinkButton *death_button = new QCommandLinkButton(tr("Death"), last_word);
        button_layout->addWidget(death_button);

        death_button->setProperty("general", QVariant::fromValue(general));
        death_button->setProperty("skinId", skinId);
        connect(death_button, &QCommandLinkButton::clicked, this, &GeneralOverview::playDeathAudio);

        addCopyAction(death_button);
    }
}

void GeneralOverview::addWinLineOfCaoCao()
{
    QCommandLinkButton *win_button = new QCommandLinkButton(tr("Victory"),
        tr("Six dragons lead my chariot, "
        "I will ride the wind with the greatest speed."
        "With all of the feudal lords under my command,"
        "to rule the world with one name!"));

    button_layout->addWidget(win_button);
    addCopyAction(win_button);

    win_button->setObjectName("audio/system/win-cc.ogg");
    connect(win_button, &QCommandLinkButton::clicked, this, &GeneralOverview::playAudioEffect);
}

void GeneralOverview::addCopyAction(QCommandLinkButton *button) {
    QAction *action = new QAction(button);
    action->setData(button->description());
    button->addAction(action);
    action->setText(tr("Copy lines"));
    button->setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(action, &QAction::triggered, this, &GeneralOverview::copyLines);
}

void GeneralOverview::copyLines() {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(action->data().toString());
    }
}

void GeneralOverview::on_tableView_clicked(const QModelIndex &index)
{
    const QString generalName = ui->tableView->model()->data(index, Qt::UserRole).toString();
    const General *general = Sanguosha->getGeneral(generalName);
    const int skinId = all_generals->value(general);
    ui->generalPhoto->setPixmap(G_ROOM_SKIN.getGeneralCardPixmap(generalName, skinId));
    ui->changeHeroSkinButton->setVisible(hasSkin(general));

    ui->skillTextEdit->clear();

    resetButtons();

    foreach(const Skill *skill, qsgs_allSkillsOf(general))
        addLines(general, skill);

    addDeathLine(general);

    if (generalName.contains("caocao"))
        addWinLineOfCaoCao();

    QString designer_text = Sanguosha->translate("designer:" + generalName);
    if (!designer_text.startsWith("designer:"))
        ui->designerLineEdit->setText(designer_text);
    else
        ui->designerLineEdit->setText(tr("Official"));

    ui->cvLineEdit->setText(getCvInfo(generalName));
    ui->illustratorLineEdit->setText(getIllustratorInfo(generalName));

    button_layout->addStretch();
    QString companions_text = general->getCompanions();
    if (companions_text.isEmpty())
        ui->companionsLineEdit->setText(tr("None"));
    else
        ui->companionsLineEdit->setText(companions_text);
    ui->skillTextEdit->append(general->getSkillDescription(false, false));
}

void GeneralOverview::playDeathAudio()
{
    const General *general = sender()->property("general").value<const General *>();
    const int skinId = sender()->property("skinId").toInt();
    general->lastWord(skinId);
}

void GeneralOverview::playAudioEffect() {
    QObject *button = sender();
    if (button) {
        QString source = button->objectName();
        if (!source.isEmpty())
            Sanguosha->playAudioEffect(source);
    }
}

void GeneralOverview::showNextSkin() {
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid())
        return;
    const QString generalName = ui->tableView->model()->data(index, Qt::UserRole).toString();

    const General *general = Sanguosha->getGeneral(generalName);
    int skinId = ++ (*all_generals)[general];

    QPixmap pixmap;
    if (G_ROOM_SKIN.doesGeneralHaveSkin(generalName, skinId, true)) {
        pixmap = G_ROOM_SKIN.getGeneralCardPixmap(generalName, skinId);
        general->tryLoadingSkinTranslation(skinId);
    } else {
        pixmap = G_ROOM_SKIN.getGeneralCardPixmap(generalName);
        (*all_generals)[general] = 0;
    }

    ui->tableView->update(index.sibling(index.row(), GeneralModel::TitleColumn));

    ui->generalPhoto->setPixmap(pixmap);
    ui->cvLineEdit->setText(getCvInfo(generalName));
    ui->illustratorLineEdit->setText(getIllustratorInfo(generalName));
    resetButtons();
    foreach(const Skill *skill, qsgs_allSkillsOf(general))
        addLines(general, skill);

    addDeathLine(general);

    if (generalName.contains("caocao"))
        addWinLineOfCaoCao();

    button_layout->addStretch();
}

void GeneralOverview::startSearch(const SearchDetails &detail) {
    startSearch(detail.include_hidden, detail.nickname, detail.name, detail.genders, detail.kingdoms, detail.lower, detail.upper, detail.packages);
}

void GeneralOverview::startSearch(bool include_hidden, const QString &nickname, const QString &name, const QStringList &genders, const QStringList &kingdoms, int lower, int upper, const QStringList &packages) {
    if (all_generals == NULL)
        return;

    QList<const General *> generals;
    foreach(const General *general, all_generals->keys()) {
        QString general_name = general->objectName();
        if (!include_hidden && Sanguosha->isGeneralHidden(general_name))
            continue;
        if (!nickname.isEmpty()) {
            QString v_nickname = nickname;
            v_nickname.replace("?", ".");
            v_nickname.replace("*", ".*");
            QRegExp rx(v_nickname);

            QString g_nickname = Sanguosha->translate("#" + general_name);
            if (g_nickname.startsWith("#"))
                g_nickname = Sanguosha->translate("#" + general_name.split("_").last());
            if (!rx.exactMatch(g_nickname))
                continue;
        }
        if (!name.isEmpty()) {
            QString v_name = name;
            v_name.replace("?", ".");
            v_name.replace("*", ".*");
            QRegExp rx(v_name);

            QString g_name = Sanguosha->translate(general_name);
            if (!rx.exactMatch(g_name))
                continue;
        }
        if (!genders.isEmpty()) {
            if (general->isMale() && !genders.contains("male"))
                continue;
            if (general->isFemale() && !genders.contains("female"))
                continue;
            if (general->isNeuter() && !genders.contains("nogender"))
                continue;
        }
        if (!kingdoms.isEmpty() && !kingdoms.contains(general->getKingdom()))
            continue;
        if (!(lower == 0 && upper == 0) && (general->getDoubleMaxHp() < lower || general->getDoubleMaxHp() > upper))
            continue;
        if (!packages.isEmpty() && !packages.contains(general->getPackage()))
            continue;
        generals << general;
    }
    if (generals.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No generals are found"));
    } else {
        ui->returnButton->show();
        if (windowTitle() == origin_window_title)
            setWindowTitle(windowTitle() + " " + tr("Search..."));
        fillGenerals(generals, false);
    }
}

void GeneralOverview::fillAllGenerals() {
    ui->returnButton->hide();
    setWindowTitle(origin_window_title);
    fillGenerals(all_generals->keys(), false);
}
