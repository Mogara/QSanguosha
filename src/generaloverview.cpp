#include "generaloverview.h"
#include "ui_generaloverview.h"
#include "engine.h"

#include <QMessageBox>

GeneralOverview::GeneralOverview(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralOverview)
{
    ui->setupUi(this);

    QList<General*> generals = Sanguosha->findChildren<General*>();
    ui->tableWidget->setRowCount(generals.length());
    ui->tableWidget->setIconSize(QSize(20,20));
    QIcon lord_icon(":/images/roles/lord.png");

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
        name_item->setTextAlignment(Qt::AlignHCenter);
        name_item->setData(Qt::UserRole, general->objectName());
        if(general->isLord()){
            name_item->setIcon(lord_icon);
            name_item->setTextAlignment(Qt::AlignLeft);
        }

        QTableWidgetItem *kingdom_item = new QTableWidgetItem(kingdom);
        kingdom_item->setTextAlignment(Qt::AlignHCenter);

        QTableWidgetItem *gender_item = new QTableWidgetItem(gender);
        gender_item->setTextAlignment(Qt::AlignHCenter);

        QTableWidgetItem *max_hp_item = new QTableWidgetItem(max_hp);
        max_hp_item->setTextAlignment(Qt::AlignHCenter);

        QTableWidgetItem *package_item = new QTableWidgetItem(package);
        package_item->setTextAlignment(Qt::AlignHCenter);

        ui->tableWidget->setItem(i, 0, name_item);
        ui->tableWidget->setItem(i, 1, kingdom_item);
        ui->tableWidget->setItem(i, 2, gender_item);
        ui->tableWidget->setItem(i, 3, max_hp_item);
        ui->tableWidget->setItem(i, 4, package_item);
    }

    ui->tableWidget->setColumnWidth(0, 60);
    ui->tableWidget->setColumnWidth(1, 50);
    ui->tableWidget->setColumnWidth(2, 50);
    ui->tableWidget->setColumnWidth(3, 60);

    ui->tableWidget->setCurrentItem(ui->tableWidget->item(0,0));
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
    QList<const Skill *> skills = general->getSkills();
    ui->skillTextEdit->clear();
    foreach(const Skill *skill, skills){
        QString skill_name = QString("<b>%1</b>").arg(Sanguosha->translate(skill->objectName()));
        QString desc = skill->getDescription();
        desc.replace("\n", "<br/>");
        ui->skillTextEdit->append(QString("<b>%1</b>: %2").arg(skill_name).arg(desc));
    }
}
