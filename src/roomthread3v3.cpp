#include "roomthread3v3.h"
#include "room.h"
#include "engine.h"
#include "ai.h"

#include <QDateTime>

RoomThread3v3::RoomThread3v3(Room *room)
    :QThread(room), room(room)
{
    foreach(ServerPlayer *player, room->players){
        switch(player->getRoleEnum()){
        case Player::Lord: warm_leader = player; break;
        case Player::Renegade: cool_leader = player; break;
        default:
            break;
        }
    }

    const Package *stdpack = Sanguosha->findChild<const Package *>("standard");
    const Package *windpack = Sanguosha->findChild<const Package *>("wind");

    generals = stdpack->findChildren<const General *>();
    generals << windpack->findChildren<const General *>();

    // remove hidden generals
    QMutableListIterator<const General *> itor(generals);
    while(itor.hasNext()){
        itor.next();

        if(itor.value()->isHidden())
            itor.remove();
    }

    generals << Sanguosha->getGeneral("xiaoqiao");

    Q_ASSERT(generals.length() == 32);

    qShuffle(generals);
    generals = generals.mid(0, 16);

    foreach(const General *general, generals)
        general_names << general->objectName();
}

void RoomThread3v3::run()
{
    // initialize the random seed for this thread
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    room->broadcastInvoke("fillGenerals", general_names.join("+"));

    //QString order = askForChoice(warm_leader, "3v3", "first+next");
    QString order = "first";
    ServerPlayer *first, *next;
    if(order == "first"){
        first = warm_leader;
        next = cool_leader;
    }else{
        first = cool_leader;
        next = warm_leader;
    }

    askForTakeGeneral(first);

    while(general_names.length() > 1){
        qSwap(first, next);

        askForTakeGeneral(first);
        askForTakeGeneral(first);
    }

    askForTakeGeneral(next);

    startArrange(first);
    startArrange(next);

    sem.acquire(2);
}

void RoomThread3v3::askForTakeGeneral(ServerPlayer *player){
    QString name;
    if(general_names.length() == 1)
        name = general_names.first();
    else if(player->getState() != "online"){
        int r = qrand() % general_names.length();
        name = general_names.at(r);
    }

    if(name.isNull()){
        player->invoke("askForGeneral3v3");
    }else{
        msleep(1000);
        takeGeneral(player, name);
    }

    sem.acquire();
}

void RoomThread3v3::takeGeneral(ServerPlayer *player, const QString &name){
    general_names.removeOne(name);
    player->addToSelected(name);

    QString group = player->isLord() ? "warm" : "cool";
    room->broadcastInvoke("takeGeneral", QString("%1:%2").arg(group).arg(name));

    sem.release();
}

void RoomThread3v3::startArrange(ServerPlayer *player){
    if(player->getState() != "online"){
        QStringList arranged = player->getSelected();
        qShuffle(arranged);
        arranged = arranged.mid(0, 3);
        arrange(player, arranged);
    }else{
        player->invoke("startArrange");        
    }
}

void RoomThread3v3::arrange(ServerPlayer *player, const QStringList &arranged){
    Q_ASSERT(arranged.length() == 3);

    if(player->isLord()){
        room->players.at(1)->setGeneralName(arranged.at(0));
        room->players.at(0)->setGeneralName(arranged.at(1));
        room->players.at(5)->setGeneralName(arranged.at(2));
    }else{
        room->players.at(4)->setGeneralName(arranged.at(0));
        room->players.at(3)->setGeneralName(arranged.at(1));
        room->players.at(2)->setGeneralName(arranged.at(2));
    }

    sem.release();
}
