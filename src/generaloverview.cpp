#include "generaloverview.h"
#include "ui_generaloverview.h"
#include "engine.h"

GeneralOverview::GeneralOverview(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralOverview)
{
    ui->setupUi(this);

    QObject *generals_obj = Sanguosha->findChild<QObject*>("generals");
    QList<General*> generals = generals_obj->findChildren<General*>();
    ui->tableWidget->setRowCount(generals.length());
    int i;
    for(i=0; i<generals.length(); i++){
        General *general = generals[i];
        QString name, kingdom, gender, max_hp, package;

        name = Sanguosha->translate(general->objectName());
        kingdom = Sanguosha->translate(general->property("kingdom").toString());
        gender = general->property("male").toBool() ? tr("Male") : tr("Female");
        max_hp = general->property("max_hp").toString();
        // FIXME: package

        QTableWidgetItem *name_item = new QTableWidgetItem(name);
        name_item->setTextAlignment(Qt::AlignHCenter);
        name_item->setData(Qt::UserRole, general->objectName());

        QTableWidgetItem *kingdom_item = new QTableWidgetItem(kingdom);
        kingdom_item->setTextAlignment(Qt::AlignHCenter);

        QTableWidgetItem *gender_item = new QTableWidgetItem(gender);
        gender_item->setTextAlignment(Qt::AlignHCenter);

        QTableWidgetItem *max_hp_item = new QTableWidgetItem(max_hp);
        max_hp_item->setTextAlignment(Qt::AlignHCenter);

        ui->tableWidget->setItem(i, 0, name_item);
        ui->tableWidget->setItem(i, 1, kingdom_item);
        ui->tableWidget->setItem(i, 2, gender_item);
        ui->tableWidget->setItem(i, 3, max_hp_item);
    }

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
    General *general = Sanguosha->getGeneral(general_name);
    ui->generalPhoto->setPixmap(QPixmap(general->getPixmapPath("card")));
}
