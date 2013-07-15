#include "generaloverview.h"
#include "ui_generaloverview.h"
#include "engine.h"
#include "client.h"
#include "settings.h"

#include <QMessageBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QCommandLinkButton>
#include <QClipboard>

#include "generalmodel.h"

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
}

void GeneralOverview::fillGenerals(const QList<const General *> &generals){
    QList<const General *> copy_generals = generals;
    QMutableListIterator<const General *> itor = copy_generals;
    while(itor.hasNext()){
        if(itor.next()->isTotallyHidden())
            itor.remove();
    }

    GeneralModel *model = new GeneralModel(copy_generals);
    model->setParent(this);
    ui->tableView->setModel(model);
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
        QRegExp rx(".+/(\\w+\\d?).(\\w+)");
        for(int i = 0; i < sources.length(); i++){
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

void GeneralOverview::on_tableView_clicked(const QModelIndex &index)
{
    QString general_name = ui->tableView->model()->data(index, Qt::UserRole).toString();
    const General *general = Sanguosha->getGeneral(general_name);

    QString category = QString();
    int style = Config.value("UI/GStyle", Config.S_STYLE_INDEX).toInt();
    if(style == 1)
        category = "card2";
    else if(style == 2)
        category = "card3";
    else
        category = "card";
    ui->generalPhoto->setPixmap(QPixmap(general->getPixmapPath(category)));

    if(ServerInfo.isPlay && Config.value("Cheat/FreeChange", false).toBool())
        ui->changeGeneralButton->show();
    else
        ui->changeGeneralButton->hide();

    QList<const Skill *> skills = general->getVisibleSkillList();

    foreach(QString skill_name, general->getRelatedSkillNames()){
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if(skill)
            skills << skill;
    }

    ui->skillTextEdit->clear();

    resetButtons();

    foreach(const Skill *skill, skills)
        addLines(skill);

    QString last_word = Sanguosha->translate("~" + general_name);
    if(last_word.startsWith("~")){
        QStringList origin_generals = general_name.split("_");
        if(origin_generals.length()>1)
            last_word = Sanguosha->translate(("~") +  origin_generals.at(1));
    }

    if(last_word.startsWith("~") && general_name.endsWith("f")){
        QString origin_general = general_name;
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

    if(general->isLord() || general->isCaoCao()){
        QString win_word = general->getWinword();
        if(!win_word.startsWith("`")){
            QCommandLinkButton *win_button = new QCommandLinkButton(tr("Victory"), win_word);
            button_layout->addWidget(win_button);
            connect(win_button, SIGNAL(clicked()), general, SLOT(winWord()));
            addCopyAction(win_button);
        }
    }

    QString designer_text = Sanguosha->translate("designer:" + general_name);
    if(!designer_text.startsWith("designer:"))
        ui->designerLineEdit->setText(designer_text);
    else
        ui->designerLineEdit->setText(tr("Official"));

    QString cv_text = Sanguosha->translate("cv:" + general_name);
    if(!cv_text.startsWith("cv:"))
        ui->cvLineEdit->setText(cv_text);
    else
        ui->cvLineEdit->setText(tr("Official"));

    QString illustrator_text = Sanguosha->translate("illustrator:" + general_name);
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

void GeneralOverview::on_tableView_doubleClicked(const QModelIndex &index)
{
    if(Self == NULL)
        return;

    if(!Config.value("Cheat/FreeChange", false).toBool())
        return;

    QString general_name = ui->tableView->model()->data(index, Qt::UserRole).toString();
    if(general_name != Self->getGeneralName()){
        ClientInstance->changeGeneral(general_name);
        ui->changeGeneralButton->setEnabled(false);
    }
}
