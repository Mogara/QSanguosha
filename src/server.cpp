#include "server.h"
#include "settings.h"
#include "room.h"
#include "engine.h"
#include "nativesocket.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QFormLayout>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ServerDialog::ServerDialog(QWidget *parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Start server"));

    player_count_spinbox = new QSpinBox;
    player_count_spinbox->setMinimum(2);
    player_count_spinbox->setMaximum(8);
    player_count_spinbox->setValue(Config.PlayerCount);
    player_count_spinbox->setSuffix(tr(" persons"));

    timeout_spinbox = new QSpinBox;
    timeout_spinbox->setMinimum(5);
    timeout_spinbox->setMaximum(30);
    timeout_spinbox->setValue(Config.OperationTimeout);
    timeout_spinbox->setSuffix(tr(" seconds"));

    nolimit_checkbox = new QCheckBox(tr("No limit"));
    nolimit_checkbox->setChecked(false);
    connect(nolimit_checkbox, SIGNAL(toggled(bool)), timeout_spinbox, SLOT(setDisabled(bool)));
    nolimit_checkbox->setChecked(Config.OperationNoLimit);

    QGroupBox *advanced_box = new QGroupBox;
    advanced_box->setTitle(tr("Advanced"));
    QVBoxLayout *advanced_box_layout = new QVBoxLayout;
    advanced_box->setLayout(advanced_box_layout);

    free_choose_checkbox = new QCheckBox(tr("Free choose generals"));
    free_choose_checkbox->setToolTip(tr("Enable this will make the clients choose generals freely"));
    free_choose_checkbox->setChecked(Config.FreeChoose);
    advanced_box_layout->addWidget(free_choose_checkbox);

    forbid_same_ip_checkbox = new QCheckBox(tr("Forbid same IP with multiple connection"));
    forbid_same_ip_checkbox->setChecked(Config.ForbidSIMC);
    advanced_box_layout->addWidget(forbid_same_ip_checkbox);

    QHBoxLayout *button_layout = new QHBoxLayout;
    button_layout->addStretch();

    QPushButton *ok_button = new QPushButton(tr("OK"));
    QPushButton *cancel_button = new QPushButton(tr("Cancel"));

    button_layout->addWidget(ok_button);
    button_layout->addWidget(cancel_button);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

    QGroupBox *extension_box = new QGroupBox;
    extension_box->setTitle(tr("Extension package selection"));
    QGridLayout *grid_layout = new QGridLayout;
    extension_box->setLayout(grid_layout);
    extension_group = new QButtonGroup;
    extension_group->setExclusive(false);

    static QStringList extensions;
    if(extensions.isEmpty())
        extensions << "wind" << "fire" << "thicket" << "maneuvering" << "god" << "yitian";
    QSet<QString> ban_packages = Config.BanPackages.toSet();

    int i;
    for(i=0; i<extensions.length(); i++){
        QString extension = extensions.at(i);
        QCheckBox *checkbox = new QCheckBox;
        checkbox->setObjectName(extension);
        checkbox->setText(Sanguosha->translate(extension));
        checkbox->setChecked(! ban_packages.contains(extension));

        extension_group->addButton(checkbox);

        int row = i / 2;
        int column = i % 2;
        grid_layout->addWidget(checkbox, row, column);
    }

    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Player count"), player_count_spinbox);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(new QLabel(tr("Operation timeout")));
    hlayout->addWidget(timeout_spinbox);
    hlayout->addWidget(nolimit_checkbox);
    layout->addRow(hlayout);
    layout->addRow(advanced_box);
    layout->addRow(extension_box);
    layout->addRow(button_layout);

    setLayout(layout);
}

bool ServerDialog::config(){
    exec();

    if(result() != Accepted)
        return false;

    Config.PlayerCount = player_count_spinbox->value();
    Config.OperationTimeout = timeout_spinbox->value();
    Config.OperationNoLimit = nolimit_checkbox->isChecked();
    Config.FreeChoose = free_choose_checkbox->isChecked();
    Config.ForbidSIMC = forbid_same_ip_checkbox->isChecked();

    Config.setValue("PlayerCount", Config.PlayerCount);
    Config.setValue("OperationTimeout", Config.OperationTimeout);
    Config.setValue("OperationNoLimit", Config.OperationNoLimit);
    Config.setValue("FreeChoose", Config.FreeChoose);
    Config.setValue("ForbidSIMC", Config.ForbidSIMC);

    QSet<QString> ban_packages;
    QList<QAbstractButton *> checkboxes = extension_group->buttons();
    foreach(QAbstractButton *checkbox, checkboxes){
        if(!checkbox->isChecked()){
            QString package_name = checkbox->objectName();
            Sanguosha->addBanPackage(package_name);
            ban_packages.insert(package_name);
        }
    }

    Config.BanPackages = ban_packages.toList();
    Config.setValue("BanPackages", Config.BanPackages);

    return true;
}

Server::Server(QObject *parent)
    :QObject(parent)
{
    server = new NativeServerSocket();
    server->setParent(this);

    connect(server, SIGNAL(new_connection(ClientSocket*)), this, SLOT(processNewConnection(ClientSocket*)));
}

bool Server::listen(){
    return server->listen();
}

void Server::processNewConnection(ClientSocket *socket){
    if(Config.ForbidSIMC){
        QString addr = socket->peerAddress();
        if(addresses.contains(addr)){
            socket->disconnectFromHost();
            emit server_message(tr("Forbid the connection of address %1").arg(addr));
            return;
        }else{
            addresses.insert(addr);
            connect(socket, SIGNAL(disconnected()), this, SLOT(removeAddress()));
        }
    }

    // remove the game over room first
    QMutableListIterator<Room *> itor(rooms);
    while(itor.hasNext()){
        Room *room = itor.next();
        if(room->isFinished()){
            delete room;
            itor.remove();
        }
    }

    // find the free room for the new connection
    Room *free_room = NULL;
    foreach(Room *room, rooms){
        if(!room->isFull()){
            free_room = room;
            break;
        }
    }

    // if no free room is found, create a new room for him
    if(free_room == NULL){
        free_room = new Room(this, Config.PlayerCount);
        rooms << free_room;
        connect(free_room, SIGNAL(room_message(QString)), this, SIGNAL(server_message(QString)));
    }

    free_room->addSocket(socket);

    emit server_message(tr("%1 connected").arg(socket->peerName()));
}

void Server::removeAddress(){
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());

    if(socket){
        addresses.remove(socket->peerAddress());
    }
}
