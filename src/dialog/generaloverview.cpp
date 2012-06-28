#include "generaloverview.h"
#include "ui_generaloverview.h"
#include "engine.h"

#include <QMessageBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QCommandLinkButton>
#include <QClipboard>

GeneralOverview::GeneralOverview(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralOverview)
{
    ui->setupUi(this);

    button_layout = new QVBoxLayout;

    QGroupBox *group_box = new QGroupBox;
    group_box->setTitle(tr("Effects"));
    group_box->setLayout(button_layout);
    ui->scrollArea->setWidget(group_box);
    ui->skillTextEdit->setProperty("description", true);
}

void GeneralOverview::fillGenerals(const QList<const General *> &generals){
    QList<const General *> copy_generals = generals;
    QMutableListIterator<const General *> itor = copy_generals;
    while(itor.hasNext()){
        if(itor.next()->isTotallyHidden())
            itor.remove();
    }

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(copy_generals.length());
    ui->tableWidget->setIconSize(QSize(20,20));
    QIcon lord_icon("image/system/roles/lord.png");

    int i;
    for(i=0; i<copy_generals.length(); i++){
        const General *general = copy_generals[i];

        QString name, kingdom, gender, max_hp, package;

        name = Sanguosha->translate(general->objectName());

        kingdom = Sanguosha->translate(general->getKingdom());
        gender = general->isMale() ? tr("Male") : tr("Female");
        max_hp = QString::number(general->getMaxHp());
        package = Sanguosha->translate(general->getPackage());

        QString nickname = Sanguosha->translate("#" + general->objectName());
        QTableWidgetItem *nickname_item;
        if(!nickname.startsWith("#"))
            nickname_item = new QTableWidgetItem(nickname);
        else
            nickname_item = new QTableWidgetItem(Sanguosha->translate("UnknowNick"));
        nickname_item->setData(Qt::UserRole, general->objectName());
        nickname_item->setTextAlignment(Qt::AlignCenter);

        if(general->isHidden())
            nickname_item->setBackgroundColor(Qt::gray);

        QTableWidgetItem *name_item = new QTableWidgetItem(name);
        name_item->setTextAlignment(Qt::AlignCenter);
        name_item->setData(Qt::UserRole, general->objectName());
        if(general->isLord()){
            name_item->setIcon(lord_icon);
            name_item->setTextAlignment(Qt::AlignCenter);
        }

        if(general->isHidden())
            name_item->setBackgroundColor(Qt::gray);

        QTableWidgetItem *kingdom_item = new QTableWidgetItem(kingdom);
        kingdom_item->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *gender_item = new QTableWidgetItem(gender);
        gender_item->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *max_hp_item = new QTableWidgetItem(max_hp);
        max_hp_item->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *package_item = new QTableWidgetItem(package);
        package_item->setTextAlignment(Qt::AlignCenter);

        ui->tableWidget->setItem(i, 0, nickname_item);
        ui->tableWidget->setItem(i, 1, name_item);
        ui->tableWidget->setItem(i, 2, kingdom_item);
        ui->tableWidget->setItem(i, 3, gender_item);
        ui->tableWidget->setItem(i, 4, max_hp_item);
        ui->tableWidget->setItem(i, 5, package_item);
    }

    ui->tableWidget->setColumnWidth(0, 80);
    ui->tableWidget->setColumnWidth(1, 80);
    ui->tableWidget->setColumnWidth(2, 40);
    ui->tableWidget->setColumnWidth(3, 50);
    ui->tableWidget->setColumnWidth(4, 60);
    ui->tableWidget->setColumnWidth(5, 60);

    ui->tableWidget->setCurrentItem(ui->tableWidget->item(0,0));
}

void GeneralOverview::resetButtons(){
    QLayoutItem *child;
    while((child = button_layout->takeAt(0))){
        QWidget *widget = child->widget();
        if(widget)
            delete widget;
    }
}

GeneralOverview::~GeneralOverview()
{
    delete ui;
}

void GeneralOverview::addLines(const Skill *skill){
    QString skill_name = Sanguosha->translate(skill->objectName());
    QStringList sources = skill->getSources();

    if(sources.isEmpty()){
        QCommandLinkButton *button = new QCommandLinkButton(skill_name);

        button->setEnabled(false);
        button_layout->addWidget(button);
    }else{
        QRegExp rx(".+/(\\w+\\d?).ogg");
        int i;
        for(i=0; i<sources.length(); i++){
            QString source = sources.at(i);
            if(!rx.exactMatch(source))
                continue;

            QString button_text = skill_name;
            if(sources.length() != 1){
                button_text.append(QString(" (%1)").arg(i+1));
            }

            QCommandLinkButton *button = new QCommandLinkButton(button_text);
            button->setObjectName(source);
            button_layout->addWidget(button);

            QString filename = rx.capturedTexts().at(1);
            QString skill_line = Sanguosha->translate("$" + filename);
            button->setDescription(skill_line);

            connect(button, SIGNAL(clicked()), this, SLOT(playEffect()));

            addCopyAction(button);
        }
    }
}

void GeneralOverview::addCopyAction(QCommandLinkButton *button){
    QAction *action = new QAction(button);
    action->setData(button->description());
    button->addAction(action);
    action->setText(tr("Copy lines"));
    button->setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(action, SIGNAL(triggered()), this, SLOT(copyLines()));
}

void GeneralOverview::copyLines(){
    QAction *action = qobject_cast<QAction *>(sender());
    if(action){
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(action->data().toString());
    }
}

void GeneralOverview::on_tableWidget_itemSelectionChanged()
{
    int row = ui->tableWidget->currentRow();
    QString general_name = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toString();
    const General *general = Sanguosha->getGeneral(general_name);
    ui->generalPhoto->setPixmap(QPixmap(general->getPixmapPath("card")));
    QList<const Skill *> skills = general->getVisibleSkillList();

    foreach(QString skill_name, general->getRelatedSkillNames()){
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if(skill)
            skills << skill;
    }

    ui->skillTextEdit->clear();

    resetButtons();

    foreach(const Skill *skill, skills){
        addLines(skill);
    }

    QString last_word = Sanguosha->translate("~" + general->objectName());
    if(last_word.startsWith("~")){
        QStringList origin_generals = general->objectName().split("_");
        if(origin_generals.length()>1)
            last_word = Sanguosha->translate(("~") +  origin_generals.at(1));
    }

    if(last_word.startsWith("~") && general->objectName().endsWith("f")){
        QString origin_general = general->objectName();
        origin_general.chop(1);
        if(Sanguosha->getGeneral(origin_general))
            last_word = Sanguosha->translate(("~") + origin_general);
    }

    if(!last_word.startsWith("~")){

        QCommandLinkButton *death_button = new QCommandLinkButton(tr("Death"), last_word);
        button_layout->addWidget(death_button);

        connect(death_button, SIGNAL(clicked()), general, SLOT(lastWord()));

        addCopyAction(death_button);
    }

    if(general_name == "caocao" || general_name == "weiwudi" || general_name == "shencaocao"){
        QCommandLinkButton *win_button = new QCommandLinkButton(tr("Victory"), tr(
                "Six dragons lead my chariot, "
                "I will ride the wind with the greatest speed."
                "With all of the feudal lords under my command,"
                "to rule the world with one name!"));

        button_layout->addWidget(win_button);
        addCopyAction(win_button);

        win_button->setObjectName("audio/system/win-cc.ogg");
        connect(win_button, SIGNAL(clicked()), this, SLOT(playEffect()));
    }

    QString designer_text = Sanguosha->translate("designer:" + general->objectName());
    if(!designer_text.startsWith("designer:"))
        ui->designerLineEdit->setText(designer_text);
    else
        ui->designerLineEdit->setText(tr("Official"));

    QString cv_text = Sanguosha->translate("cv:" + general->objectName());
    if(!cv_text.startsWith("cv:"))
        ui->cvLineEdit->setText(cv_text);
    else
        ui->cvLineEdit->setText(tr("Official"));

    QString illustrator_text = Sanguosha->translate("illustrator:" + general->objectName());
    if(!illustrator_text.startsWith("illustrator:"))
        ui->illustratorLineEdit->setText(illustrator_text);
    else
        ui->illustratorLineEdit->setText(Sanguosha->translate("DefaultIllustrator"));

    button_layout->addStretch();
    ui->skillTextEdit->append(general->getSkillDescription());
}

void GeneralOverview::playEffect()
{
    QObject *button = sender();
    if(button){
        QString source = button->objectName();
        if(!source.isEmpty())
            Sanguosha->playEffect(source);
    }
}

#include "clientstruct.h"
#include "client.h"
void GeneralOverview::on_tableWidget_itemDoubleClicked(QTableWidgetItem* item)
{
    if(ServerInfo.FreeChoose && Self){
        int row = ui->tableWidget->currentRow();
        QString general_name = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toString();
        ClientInstance->requestCheatChangeGeneral(general_name);
    }
}
