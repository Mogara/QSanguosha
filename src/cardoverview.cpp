#include "cardoverview.h"
#include "ui_cardoverview.h"
#include "engine.h"

CardOverview::CardOverview(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CardOverview)
{
    ui->setupUi(this);

    ui->tableWidget->setColumnWidth(0, 80);
    ui->tableWidget->setColumnWidth(1, 60);
    ui->tableWidget->setColumnWidth(2, 30);
    ui->tableWidget->setColumnWidth(3, 60);
    ui->tableWidget->setColumnWidth(4, 60);    
}

void CardOverview::loadFromAll(){
    int i, n = Sanguosha->getCardCount();
    ui->tableWidget->setRowCount(n);
    for(i=0; i<n ;i++){
        Card *card = Sanguosha->getCard(i);
        addCard(i, card);
    }

    if(n>0)
        ui->tableWidget->setCurrentItem(ui->tableWidget->item(0,0));
}

void CardOverview::loadFromList(const QList<const Card*> &list){
    int i, n = list.length();
    ui->tableWidget->setRowCount(n);
    for(i=0; i<n; i++)
        addCard(i, list.at(i));    

    if(n>0)
        ui->tableWidget->setCurrentItem(ui->tableWidget->item(0,0));
}

void CardOverview::addCard(int i, const Card *card){
    QString name = Sanguosha->translate(card->objectName());
    QIcon suit_icon = QIcon(QString(":/images/suit/%1.png").arg(card->getSuitString()));
    QString suit_str = Sanguosha->translate(card->getSuitString());
    QString point = card->getNumberString();
    QString type = Sanguosha->translate(card->getType());
    QString subtype = Sanguosha->translate(card->getSubtype());
    QString package = Sanguosha->translate(card->getPackage());

    QTableWidgetItem *name_item = new QTableWidgetItem(name);
    name_item->setData(Qt::UserRole, card->getID());

    ui->tableWidget->setItem(i, 0, name_item);
    ui->tableWidget->setItem(i, 1, new QTableWidgetItem(suit_icon, suit_str));
    ui->tableWidget->setItem(i, 2, new QTableWidgetItem(point));
    ui->tableWidget->setItem(i, 3, new QTableWidgetItem(type));
    ui->tableWidget->setItem(i, 4, new QTableWidgetItem(subtype));
    ui->tableWidget->setItem(i, 5, new QTableWidgetItem(package));
}

CardOverview::~CardOverview()
{
    delete ui;
}

void CardOverview::on_tableWidget_itemSelectionChanged()
{
    int row = ui->tableWidget->currentRow();
    int card_id = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toInt();
    QString pixmap_path = Sanguosha->getCard(card_id)->getPixmapPath();
    ui->cardLabel->setPixmap(pixmap_path);
}
