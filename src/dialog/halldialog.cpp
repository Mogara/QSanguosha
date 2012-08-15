#include "halldialog.h"
#include "mainwindow.h"
#include "client.h"

#include <QTableWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QMessageBox>

static HallDialog *HallDialogInstance;

HallDialog *HallDialog::getInstance(MainWindow *main_window){
    if(HallDialogInstance == NULL)
        HallDialogInstance = new HallDialog(main_window);

    return HallDialogInstance;
}

HallDialog::HallDialog(MainWindow *main_window)
    :QDialog(main_window), main_window(main_window), room_row(0)
{
    setWindowTitle(tr("Hall"));
    setMinimumSize(434, 432);

    HallDialogInstance = this;

    table = new QTableWidget;
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setColumnCount(3);

    QStringList labels;
    labels << tr("Room ID") << tr("Room Name") << tr("Person");
    table->setHorizontalHeaderLabels(labels);

    QHBoxLayout *hlayout = new QHBoxLayout;
    //QPushButton *prev_button = new QPushButton(tr("Previous"));
    //QPushButton *next_button = new QPushButton(tr("Next"));
    QPushButton *join_button = new QPushButton(tr("Join"));
    QPushButton *create_button = new QPushButton(tr("Create room"));
    QCheckBox *waiting_only = new QCheckBox(tr("Waiting room only"));

    //hlayout->addWidget(prev_button);
    //hlayout->addWidget(next_button);
    hlayout->addStretch();
    hlayout->addWidget(join_button);
    hlayout->addWidget(create_button);
    hlayout->addWidget(waiting_only);

    QVBoxLayout *vlayout = new QVBoxLayout;

    vlayout->addWidget(table);
    vlayout->addLayout(hlayout);
    setLayout(vlayout);

    //connect(prev_button, SIGNAL(clicked()), SLOT(pageUp()));
    //connect(next_button, SIGNAL(clicked()), SLOT(pageDown()));
    connect(join_button, SIGNAL(clicked()), SLOT(join()));
    connect(create_button, SIGNAL(clicked()), SLOT(createRoom()));
    connect(waiting_only, SIGNAL(toggled(bool)), SLOT(toggleDisplay(bool)));

    connect(table, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), SLOT(join()));

    refreshRooms(0);
}

void HallDialog::pageUp(){

}

void HallDialog::pageDown(){

}

void HallDialog::join(){
    int row = table->currentRow();
    if(row == -1)
        return;

    QTableWidgetItem *item = table->item(row, 0);
    if(item == NULL)
        return;

    bool ok;
    int room_id = item->text().toInt(&ok);
    if(!ok)
        return;

    joinRoom(room_id);
}

void HallDialog::createRoom(){
    ClientInstance->request("createRoom .");
    refreshRooms(0);
}

void HallDialog::toggleDisplay(bool only_nonful){

}

void HallDialog::refreshRooms(int page){
    ClientInstance->request("refreshRooms " + QString::number(page));
    current_page = page;
}

void HallDialog::joinRoom(int room_id){
    ClientInstance->request("joinRoom " + QString::number(room_id));
}

void HallDialog::roomBegin(int total, int pagelimit){
    table->clearContents();
    table->setRowCount(total);
    room_row = 0;
}

void HallDialog::room(int room_id, int joined, const QString &setup_string){
    ServerInfoStruct info;
    if(!info.parse(setup_string))
        return;

    QTableWidgetItem *item = new QTableWidgetItem(QString::number(room_id));
    table->setItem(room_row, 0, item);

    item = new QTableWidgetItem(info.Name);
    table->setItem(room_row, 1, item);

    int total = Sanguosha->getPlayerCount(info.GameMode);
    item = new QTableWidgetItem(QString("%1/%2").arg(joined).arg(total));
    table->setItem(room_row, 2, item);

    room_row++;
}

void HallDialog::roomEnd(){

}

void Client::roomBegin(const QString &begin_str){
    QStringList texts = begin_str.split(":");
    int total = texts.at(0).toInt();
    int pagelimit = texts.at(1).toInt();

    if(HallDialogInstance)
        HallDialogInstance->roomBegin(total, pagelimit);
}

void Client::room(const QString &room_str){
    QRegExp rx("^(\\d+):(\\d+):(.+)$");
    if(!rx.exactMatch(room_str)){
        commandFormatWarning(room_str, rx, __FUNCTION__);
        return;
    }

    QStringList texts = rx.capturedTexts();
    int room_id = texts.at(1).toInt();
    int joined = texts.at(2).toInt();
    QString setup_string = texts.at(3);

    if(HallDialogInstance)
        HallDialogInstance->room(room_id, joined, setup_string);
}

void Client::roomEnd(const QString &){
    if(HallDialogInstance)
        HallDialogInstance->roomEnd();
}

void Client::roomCreated(const QString &idstr){
    HallDialogInstance->joinRoom(idstr.toInt());
}

void Client::roomError(const QString &errorStr){
    static QMap<QString, QString> map;
    if(map.isEmpty()){
        map["NO_SUCH_ROOM"] = tr("No such room!");
        map["ROOM_IS_FULL"] = tr("Room is full!");
        map["INVALID_SETUP_STRING"] = tr("Invalid setup string!");
     }

    QString msg = map.value(errorStr, tr("Unknown room error: %1").arg(errorStr));
    QMessageBox::warning(HallDialogInstance, tr("Warning"), msg);
}

void Client::hallEntered(const QString &){
    HallDialogInstance->show();
}
