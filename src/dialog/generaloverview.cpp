#include "generaloverview.h"
#include "ui_generaloverview.h"
#include "engine.h"
#include "settings.h"
#include "SkinBank.h"
#include "clientstruct.h"
#include "client.h"

#include <QMessageBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QCommandLinkButton>
#include <QClipboard>

static QLayout *HLay(QWidget *left, QWidget *right) {
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(left);
    layout->addWidget(right);
    return layout;
}

GeneralSearch::GeneralSearch(GeneralOverview *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Search..."));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(createInfoTab());
    layout->addLayout(createButtonLayout());
    setLayout(layout);

    connect(this, SIGNAL(search(bool, QString, QString, QStringList, QStringList, int, int, QStringList)),
            parent, SLOT(startSearch(bool, QString, QString, QStringList, QStringList, int, int, QStringList)));
}

QWidget *GeneralSearch::createInfoTab() {
    QVBoxLayout *layout = new QVBoxLayout;

    include_hidden_checkbox = new QCheckBox;
    include_hidden_checkbox->setText(tr("Include hidden generals"));
    include_hidden_checkbox->setChecked(true);
    layout->addWidget(include_hidden_checkbox);

    nickname_label = new QLabel(tr("Nickname"));
    nickname_label->setToolTip(tr("Input characters included by the nickname. '?' and '*' is available. Every nickname meets the condition if the line is empty."));
    nickname_edit = new QLineEdit;
    nickname_edit->clear();
    layout->addLayout(HLay(nickname_label, nickname_edit));

    name_label = new QLabel(tr("Name"));
    name_label->setToolTip(tr("Input characters included by the name. '?' and '*' is available. Every name meets the condition if the line is empty."));
    name_edit = new QLineEdit;
    name_edit->clear();
    layout->addLayout(HLay(name_label, name_edit));

    maxhp_lower_label = new QLabel(tr("MaxHp Min"));
    maxhp_lower_label->setToolTip(tr("Set lowerlimit and upperlimit of max HP. 0 ~ 0 meets all conditions."));
    maxhp_upper_label = new QLabel(tr("MaxHp Max"));
    maxhp_upper_label->setToolTip(tr("Set lowerlimit and upperlimit of max HP. 0 ~ 0 meets all conditions."));

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
    gender_group->setToolTip(tr("Select genders. Every gender meets the condition if none is selected."));
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
    kingdom_box->setToolTip(tr("Select kingdoms. Every kingdom meets the condition if none is selected."));

    QGridLayout *kingdom_layout = new QGridLayout;
    kingdom_box->setLayout(kingdom_layout);

    int i = 0;
    foreach (QString kingdom, Sanguosha->getKingdoms()) {
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
    package_box->setToolTip(tr("Select packages. Every package meets the condition if none is selected."));

    QVBoxLayout *package_layout = new QVBoxLayout;

    QHBoxLayout *package_button_layout = new QHBoxLayout;
    select_all_button = new QPushButton(tr("Select All"));
    connect(select_all_button, SIGNAL(clicked()), this, SLOT(selectAllPackages()));
    unselect_all_button = new QPushButton(tr("Unselect All"));
    connect(unselect_all_button, SIGNAL(clicked()), this, SLOT(unselectAllPackages()));
    package_button_layout->addWidget(select_all_button);
    package_button_layout->addWidget(unselect_all_button);
    package_button_layout->addStretch();

    QGridLayout *packages_layout = new QGridLayout;

    i = 0;
    foreach (QString extension, extensions) {
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

    QPushButton *clear_button = new QPushButton(tr("Clear"));
    QPushButton *ok_button = new QPushButton(tr("OK"));

    button_layout->addWidget(clear_button);
    button_layout->addWidget(ok_button);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(clear_button, SIGNAL(clicked()), this, SLOT(clearAll()));

    return button_layout;
}

void GeneralSearch::accept() {
    QString nickname = nickname_edit->text();
    QString name = name_edit->text();
    QStringList genders;
    foreach (QAbstractButton *button, gender_buttons->buttons()) {
        if (button->isChecked())
            genders << button->objectName();
    }
    QStringList kingdoms;
    foreach (QAbstractButton *button, kingdom_buttons->buttons()) {
        if (button->isChecked())
            kingdoms << button->objectName();
    }
    int lower = maxhp_lower_spinbox->value();
    int upper = qMax(lower, maxhp_upper_spinbox->value());
    QStringList packages;
    foreach (QAbstractButton *button, package_buttons->buttons()) {
        if (button->isChecked())
            packages << button->objectName();
    }
    emit search(include_hidden_checkbox->isChecked(), nickname, name, genders, kingdoms, lower, upper, packages);
    QDialog::accept();
}

void GeneralSearch::clearAll() {
    include_hidden_checkbox->setChecked(true);
    nickname_edit->clear();
    name_edit->clear();
    foreach (QAbstractButton *button, gender_buttons->buttons())
        button->setChecked(false);
    foreach (QAbstractButton *button, kingdom_buttons->buttons())
        button->setChecked(false);
    maxhp_lower_spinbox->setValue(0);
    maxhp_upper_spinbox->setValue(0);
    foreach (QAbstractButton *button, package_buttons->buttons())
        button->setChecked(false);
}

void GeneralSearch::selectAllPackages() {
    foreach (QAbstractButton *button, package_buttons->buttons())
        button->setChecked(true);
}

void GeneralSearch::unselectAllPackages() {
    foreach (QAbstractButton *button, package_buttons->buttons())
        button->setChecked(false);
}

static GeneralOverview *Overview;

GeneralOverview *GeneralOverview::getInstance(QWidget *main_window) {
    if (Overview == NULL)
        Overview = new GeneralOverview(main_window);

    return Overview;
}

GeneralOverview::GeneralOverview(QWidget *parent)
    : QDialog(parent), ui(new Ui::GeneralOverview)
{
    ui->setupUi(this);
    origin_window_title = windowTitle();

    button_layout = new QVBoxLayout;

    QGroupBox *group_box = new QGroupBox;
    group_box->setTitle(tr("Effects"));
    group_box->setLayout(button_layout);
    ui->scrollArea->setWidget(group_box);
    ui->skillTextEdit->setProperty("description", true);
    if (ServerInfo.DuringGame && ServerInfo.EnableCheat) {
        ui->changeGeneralButton->show();
        ui->changeGeneral2Button->show();
        connect(ui->changeGeneralButton, SIGNAL(clicked()), this, SLOT(askTransfiguration()));
        connect(ui->changeGeneral2Button, SIGNAL(clicked()), this, SLOT(askTransfiguration()));
    } else {
        ui->changeGeneralButton->hide();
        ui->changeGeneral2Button->hide();
    }
    connect(ui->changeHeroSkinButton, SIGNAL(clicked()), this, SLOT(askChangeSkin()));

    general_search = new GeneralSearch(this);
    connect(ui->searchButton, SIGNAL(clicked()), general_search, SLOT(show()));
    ui->returnButton->hide();
    connect(ui->returnButton, SIGNAL(clicked()), this, SLOT(fillAllGenerals()));
}

void GeneralOverview::fillGenerals(const QList<const General *> &generals, bool init) {
    QList<const General *> copy_generals;
    foreach (const General *general, generals) {
        if (!general->isTotallyHidden())
            copy_generals.append(general);
    }
    if (init) {
        ui->returnButton->hide();
        setWindowTitle(origin_window_title);
        all_generals = copy_generals;
    }

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(copy_generals.length());
    ui->tableWidget->setIconSize(QSize(20, 20));
    QIcon lord_icon("image/system/roles/lord.png");

    for (int i = 0; i < copy_generals.length(); i++) {
        const General *general = copy_generals[i];
        QString general_name = general->objectName();
        QString name, kingdom, gender, max_hp, package;

        name = Sanguosha->translate(general_name);
        kingdom = Sanguosha->translate(general->getKingdom());
        gender = general->isMale() ? tr("Male") : (general->isFemale() ? tr("Female") : tr("NoGender"));
        max_hp = QString::number(general->getMaxHp());
        package = Sanguosha->translate(general->getPackage());

        QString nickname = Sanguosha->translate("#" + general_name);
        if (nickname.startsWith("#") && general_name.contains("_"))
            nickname = Sanguosha->translate("#" +general_name.split("_").last());
        QTableWidgetItem *nickname_item;
        if (!nickname.startsWith("#"))
            nickname_item = new QTableWidgetItem(nickname);
        else
            nickname_item = new QTableWidgetItem(Sanguosha->translate("UnknowNick"));
        nickname_item->setData(Qt::UserRole, general_name);
        nickname_item->setTextAlignment(Qt::AlignCenter);

        if (Sanguosha->isGeneralHidden(general_name)) {
            nickname_item->setBackgroundColor(Qt::gray);
            nickname_item->setToolTip(tr("This general is hidden"));
        }

        QTableWidgetItem *name_item = new QTableWidgetItem(name);
        name_item->setTextAlignment(Qt::AlignCenter);
        name_item->setData(Qt::UserRole, general_name);
        if (general->isLord()) {
            name_item->setIcon(lord_icon);
            name_item->setTextAlignment(Qt::AlignCenter);
        }

        if (Sanguosha->isGeneralHidden(general_name)) {
            name_item->setBackgroundColor(Qt::gray);
            name_item->setToolTip(tr("This general is hidden"));
        }

        QTableWidgetItem *kingdom_item = new QTableWidgetItem(kingdom);
        kingdom_item->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *gender_item = new QTableWidgetItem(gender);
        gender_item->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *max_hp_item = new QTableWidgetItem(max_hp);
        max_hp_item->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *package_item = new QTableWidgetItem(package);
        package_item->setTextAlignment(Qt::AlignCenter);
        if (Config.value("LuaPackages", QString()).toString().split("+").contains(general->getPackage())) {
            package_item->setBackgroundColor(QColor(0x66, 0xCC, 0xFF));
            package_item->setToolTip(tr("This is an Lua extension"));
        }

        ui->tableWidget->setItem(i, 0, nickname_item);
        ui->tableWidget->setItem(i, 1, name_item);
        ui->tableWidget->setItem(i, 2, kingdom_item);
        ui->tableWidget->setItem(i, 3, gender_item);
        ui->tableWidget->setItem(i, 4, max_hp_item);
        ui->tableWidget->setItem(i, 5, package_item);
    }

    ui->tableWidget->setColumnWidth(0, 80);
    ui->tableWidget->setColumnWidth(1, 95);
    ui->tableWidget->setColumnWidth(2, 40);
    ui->tableWidget->setColumnWidth(3, 50);
    ui->tableWidget->setColumnWidth(4, 60);
    ui->tableWidget->setColumnWidth(5, 85);

    ui->tableWidget->setCurrentItem(ui->tableWidget->item(0, 0));
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

bool GeneralOverview::hasSkin(const QString &general_name) {
    int skin_index = Config.value(QString("HeroSkin/%1").arg(general_name), 0).toInt();
    if (skin_index == 0) {
        Config.beginGroup("HeroSkin");
        Config.setValue(general_name, 1);
        Config.endGroup();
        QPixmap pixmap = G_ROOM_SKIN.getCardMainPixmap(general_name);
        Config.beginGroup("HeroSkin");
        Config.remove(general_name);
        Config.endGroup();
        if (pixmap.width() <= 1 && pixmap.height() <= 1)
            return false;
    }
    return true;
}

QString GeneralOverview::getIllustratorInfo(const QString &general_name) {
    int skin_index = Config.value(QString("HeroSkin/%1").arg(general_name), 0).toInt();
    QString suffix = (skin_index > 0) ? QString("_%1").arg(skin_index) : QString();
    QString illustrator_text = Sanguosha->translate(QString("illustrator:%1%2").arg(general_name).arg(suffix));
    if (!illustrator_text.startsWith("illustrator:"))
        return illustrator_text;
    else {
        illustrator_text = Sanguosha->translate("illustrator:" + general_name);
        if (!illustrator_text.startsWith("illustrator:"))
            return illustrator_text;
        else
            return Sanguosha->translate("DefaultIllustrator");
    }
}

void GeneralOverview::addLines(const Skill *skill) {
    QString skill_name = Sanguosha->translate(skill->objectName());
    QStringList sources = skill->getSources();

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

            QString filename = rx.capturedTexts().at(1);
            QString skill_line = Sanguosha->translate("$" + filename);
            button->setDescription(skill_line);

            connect(button, SIGNAL(clicked()), this, SLOT(playAudioEffect()));

            addCopyAction(button);
        }
    }
}

void GeneralOverview::addCopyAction(QCommandLinkButton *button) {
    QAction *action = new QAction(button);
    action->setData(button->description());
    button->addAction(action);
    action->setText(tr("Copy lines"));
    button->setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(action, SIGNAL(triggered()), this, SLOT(copyLines()));
}

void GeneralOverview::copyLines() {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(action->data().toString());
    }
}

void GeneralOverview::on_tableWidget_itemSelectionChanged() {
    int row = ui->tableWidget->currentRow();
    QString general_name = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toString();
    const General *general = Sanguosha->getGeneral(general_name);
    ui->generalPhoto->setPixmap(G_ROOM_SKIN.getCardMainPixmap(general->objectName()));
    ui->changeHeroSkinButton->setVisible(hasSkin(general_name));

    QList<const Skill *> skills = general->getVisibleSkillList();
    foreach (QString skill_name, general->getRelatedSkillNames()) {
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if (skill && skill->isVisible()) skills << skill;
    }

    ui->skillTextEdit->clear();

    resetButtons();

    foreach (const Skill *skill, skills)
        addLines(skill);

    QString last_word = Sanguosha->translate("~" + general->objectName());
    if (last_word.startsWith("~") && general->objectName().contains("_"))
        last_word = Sanguosha->translate(("~") + general->objectName().split("_").last());

    if (!last_word.startsWith("~")) {
        QCommandLinkButton *death_button = new QCommandLinkButton(tr("Death"), last_word);
        button_layout->addWidget(death_button);

        connect(death_button, SIGNAL(clicked()), general, SLOT(lastWord()));

        addCopyAction(death_button);
    }

    if (general_name.contains("caocao")) {
        QCommandLinkButton *win_button = new QCommandLinkButton(tr("Victory"),
                                                                tr("Six dragons lead my chariot, "
                                                                   "I will ride the wind with the greatest speed."
                                                                   "With all of the feudal lords under my command,"
                                                                   "to rule the world with one name!"));

        button_layout->addWidget(win_button);
        addCopyAction(win_button);

        win_button->setObjectName("audio/system/win-cc.ogg");
        connect(win_button, SIGNAL(clicked()), this, SLOT(playAudioEffect()));
    }

    if (general_name == "shenlvbu1" || general_name == "shenlvbu2") {
        QCommandLinkButton *stage_change_button = new QCommandLinkButton(tr("Stage Change"),
                                                                         tr("Trashes, the real fun is just beginning!"));

        button_layout->addWidget(stage_change_button);
        addCopyAction(stage_change_button);

        stage_change_button->setObjectName("audio/system/stagechange.ogg");
        connect(stage_change_button, SIGNAL(clicked()), this, SLOT(playAudioEffect()));
    }

    QString designer_text = Sanguosha->translate("designer:" + general->objectName());
    if (!designer_text.startsWith("designer:"))
        ui->designerLineEdit->setText(designer_text);
    else
        ui->designerLineEdit->setText(tr("Official"));

    QString cv_text = Sanguosha->translate("cv:" + general->objectName());
    if (cv_text.startsWith("cv:"))
        cv_text = Sanguosha->translate("cv:" + general->objectName().split("_").last());
    if (!cv_text.startsWith("cv:"))
        ui->cvLineEdit->setText(cv_text);
    else
        ui->cvLineEdit->setText(tr("Official"));

    ui->illustratorLineEdit->setText(getIllustratorInfo(general->objectName()));

    button_layout->addStretch();
    ui->skillTextEdit->append(general->getSkillDescription());
    ui->changeGeneralButton->setEnabled(Self && Self->getGeneralName() != general->objectName());
    ui->changeGeneral2Button->setEnabled(Self && Self->getGeneral2Name() != general->objectName());
}

void GeneralOverview::playAudioEffect() {
    QObject *button = sender();
    if (button) {
        QString source = button->objectName();
        if (!source.isEmpty())
            Sanguosha->playAudioEffect(source);
    }
}

void GeneralOverview::askTransfiguration() {
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    bool isSecondaryHero = (button && button->objectName() == ui->changeGeneral2Button->objectName());
    if (ServerInfo.EnableCheat && Self) {
        if (isSecondaryHero)
            ui->changeGeneral2Button->setEnabled(false);
        else
            ui->changeGeneralButton->setEnabled(false);
        int row = ui->tableWidget->currentRow();
        QString general_name = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toString();
        ClientInstance->requestCheatChangeGeneral(general_name, isSecondaryHero);
    }
}

void GeneralOverview::on_tableWidget_itemDoubleClicked(QTableWidgetItem *) {
    if (ServerInfo.EnableCheat && Self) {
        askTransfiguration();
    }
}

void GeneralOverview::askChangeSkin() {
    int row = ui->tableWidget->currentRow();
    QString general_name = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toString();

    int n = Config.value(QString("HeroSkin/%1").arg(general_name), 0).toInt();
    n++;
    Config.beginGroup("HeroSkin");
    Config.setValue(general_name, n);
    Config.endGroup();
    QPixmap pixmap = G_ROOM_SKIN.getCardMainPixmap(general_name);
    if (pixmap.width() <= 1 && pixmap.height() <= 1) {
        Config.beginGroup("HeroSkin");
        Config.remove(general_name);
        Config.endGroup();
        if (n > 1)
            pixmap = G_ROOM_SKIN.getCardMainPixmap(general_name);
        else
            return;
    }
    ui->generalPhoto->setPixmap(pixmap);
    ui->illustratorLineEdit->setText(getIllustratorInfo(general_name));
}

void GeneralOverview::startSearch(bool include_hidden, const QString &nickname, const QString &name, const QStringList &genders,
                                  const QStringList &kingdoms, int lower, int upper, const QStringList &packages) {
    QList<const General *> generals;
    foreach (const General *general, all_generals) {
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
        if (!(lower == 0 && upper == 0) && (general->getMaxHp() < lower || general->getMaxHp() > upper))
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
    fillGenerals(all_generals, false);
}
