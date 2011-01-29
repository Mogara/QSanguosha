#include "generaloverview.h"
#include "ui_generaloverview.h"
#include "engine.h"

#include <QMessageBox>
#include <QRadioButton>

GeneralOverview::GeneralOverview(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralOverview)
{
    ui->setupUi(this);    

    button_group = new QButtonGroup;
    button_layout = new QVBoxLayout;
    ui->skillGroupBox->setLayout(button_layout);

    QList<General*> generals = Sanguosha->findChildren<General*>();
    ui->tableWidget->setRowCount(generals.length());
    ui->tableWidget->setIconSize(QSize(20,20));
    QIcon lord_icon("image/system/roles/lord.png");

    int i;
    for(i=0; i<generals.length(); i++){
        General *general = generals[i];
        QString name, kingdom, gender, max_hp, package;

        name = Sanguosha->translate(general->objectName());

        kingdom = Sanguosha->translate(general->getKingdom());
        gender = general->isMale() ? tr("Male") : tr("Female");
        max_hp = QString::number(general->getMaxHp());
        package = Sanguosha->translate(general->getPackage());

        QTableWidgetItem *name_item = new QTableWidgetItem(name);
        name_item->setTextAlignment(Qt::AlignCenter);
        name_item->setData(Qt::UserRole, general->objectName());
        if(general->isLord()){
            name_item->setIcon(lord_icon);
            name_item->setTextAlignment(Qt::AlignCenter);
        }

        QTableWidgetItem *kingdom_item = new QTableWidgetItem(kingdom);
        kingdom_item->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *gender_item = new QTableWidgetItem(gender);
        gender_item->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *max_hp_item = new QTableWidgetItem(max_hp);
        max_hp_item->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *package_item = new QTableWidgetItem(package);
        package_item->setTextAlignment(Qt::AlignCenter);

        ui->tableWidget->setItem(i, 0, name_item);
        ui->tableWidget->setItem(i, 1, kingdom_item);
        ui->tableWidget->setItem(i, 2, gender_item);
        ui->tableWidget->setItem(i, 3, max_hp_item);
        ui->tableWidget->setItem(i, 4, package_item);
    }

    ui->tableWidget->setColumnWidth(0, 80);
    ui->tableWidget->setColumnWidth(1, 50);
    ui->tableWidget->setColumnWidth(2, 50);
    ui->tableWidget->setColumnWidth(3, 60);

    ui->tableWidget->setCurrentItem(ui->tableWidget->item(0,0));
}

void GeneralOverview::resetButtons(){
    QList<QAbstractButton *> buttons = button_group->buttons();
    foreach(QAbstractButton *button, buttons)
        button_group->removeButton(button);

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

void GeneralOverview::on_tableWidget_itemSelectionChanged()
{
    int row = ui->tableWidget->currentRow();
    QString general_name = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toString();
    const General *general = Sanguosha->getGeneral(general_name);
    ui->generalPhoto->setPixmap(QPixmap(general->getPixmapPath("card")));
    QList<const Skill *> skills = general->findChildren<const Skill *>();
    ui->skillTextEdit->clear();

    resetButtons();

    foreach(const Skill *skill, skills){
        if(skill->objectName().startsWith("#"))
            continue;

        QString skill_name = Sanguosha->translate(skill->objectName());
        QRadioButton *button = new QRadioButton(skill_name);
        button->setObjectName(skill->objectName());

        button_layout->addWidget(button);
        button_group->addButton(button);

        QStringList sources = skill->getSources();
        if(sources.isEmpty())
            button->setEnabled(false);
    }

    button_layout->addStretch();

    QList<QAbstractButton *> buttons = button_group->buttons();
    if(!buttons.isEmpty())
        buttons.first()->setChecked(true);

    ui->skillTextEdit->append(general->getSkillDescription());
}

const General *GeneralOverview::currentGeneral(){
    int row = ui->tableWidget->currentRow();
    QString general_name = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toString();
    const General *general = Sanguosha->getGeneral(general_name);

    return general;
}

void GeneralOverview::on_playEffecButton_clicked()
{
    const General *general = currentGeneral();
    QAbstractButton *button = button_group->checkedButton();
    if(button)
        general->playEffect(button->objectName());
}

void GeneralOverview::on_lastWordButton_clicked()
{
    const General *general = currentGeneral();
    general->lastWord();
}
