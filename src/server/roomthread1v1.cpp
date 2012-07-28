#include "roomthread1v1.h"
#include "room.h"
#include "engine.h"
#include "settings.h"
#include "generalselector.h"

#include <QDateTime>

//@todo: setParent here is illegitimate in QT and is equivalent to calling
// setParent(NULL). Find another way to do it if we really need a parent.
RoomThread1v1::RoomThread1v1(Room *room)
    :room(room)
{}

void RoomThread1v1::run(){

    // initialize the random seed for this thread
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    QSet<QString> banset = Config.value("Banlist/1v1").toStringList().toSet();
    general_names = Sanguosha->getRandomGenerals(10, banset);

    QStringList known_list = general_names.mid(0, 6);
    unknown_list = general_names.mid(6, 4);

    for (int i = 0; i < 4; i++) {
        general_names[i + 6] = QString("x%1").arg(QString::number(i));
    }

    QString unknown_str = "+x0+x1+x2+x3";

    room->broadcastInvoke("fillGenerals", known_list.join("+") + unknown_str);

    ServerPlayer *first = room->getPlayers().at(0), *next = room->getPlayers().at(1);
    askForTakeGeneral(first);

    while(general_names.length() > 1){
        qSwap(first, next);

        askForTakeGeneral(first);
        askForTakeGeneral(first);
    }

    askForTakeGeneral(next);

    startArrange(first);
    startArrange(next);

    room->sem->acquire(2);
}

void RoomThread1v1::askForTakeGeneral(ServerPlayer *player){
    QString name;
    if(general_names.length() == 1)
        name = general_names.first();
    else if(player->getState() != "online"){
        GeneralSelector *selector = GeneralSelector::getInstance();
        name = selector->select1v1(general_names);
    }

    if(name.isNull()){
        player->invoke("askForGeneral1v1");
    }else{
        msleep(1000);
        takeGeneral(player, name);
    }

    room->sem->acquire();
}

void RoomThread1v1::takeGeneral(ServerPlayer *player, const QString &name){
    QString group = player->isLord() ? "warm" : "cool";
    room->broadcastInvoke("takeGeneral", QString("%1:%2").arg(group).arg(name), player);

    QRegExp unknown_rx("x(\\d)");
    QString general_name = name;
    if(unknown_rx.exactMatch(name)){
        int index = unknown_rx.capturedTexts().at(1).toInt();
        general_name = unknown_list.at(index);

        player->invoke("recoverGeneral", QString("%1:%2").arg(index).arg(general_name));
    }

    player->invoke("takeGeneral", QString("%1:%2").arg(group).arg(general_name));

    general_names.removeOne(name);
    player->addToSelected(general_name);

    room->sem->release();
}

void RoomThread1v1::startArrange(ServerPlayer *player){
    if(player->getState() != "online"){        
        GeneralSelector *selector = GeneralSelector::getInstance();
        arrange(player, selector->arrange1v1(player));
    }else{
        player->invoke("startArrange");
    }
}

void RoomThread1v1::arrange(ServerPlayer *player, const QStringList &arranged){
    Q_ASSERT(arranged.length() == 3);

    QStringList left = arranged.mid(1, 2);
    player->tag["1v1Arrange"] = QVariant::fromValue(left);
    player->setGeneralName(arranged.first());

    foreach(QString general, arranged)
        player->invoke("revealGeneral", QString("%1:%2").arg(player->objectName()).arg(general));

    room->sem->release();
}

