#include "roomthread3v3.h"
#include "room.h"
#include "engine.h"
#include "ai.h"
#include "lua.hpp"
#include "settings.h"
#include "generalselector.h"

#include <QDateTime>

//@todo: setParent here is illegitimate in QT and is equivalent to calling
// setParent(NULL). Find another way to do it if we really need a parent.
RoomThread3v3::RoomThread3v3(Room *room)
    :room(room)
{}

QStringList RoomThread3v3::getGeneralsWithoutExtension() const{
    QList<const General *> generals;

    const Package *stdpack = Sanguosha->findChild<const Package *>("standard");
    const Package *windpack = Sanguosha->findChild<const Package *>("wind");

    generals << stdpack->findChildren<const General *>()
             << windpack->findChildren<const General *>();

    // remove hidden generals
    QMutableListIterator<const General *> itor(generals);
    while(itor.hasNext()){
        itor.next();

        if(itor.value()->isHidden())
            itor.remove();
    }

    generals.removeOne(Sanguosha->getGeneral("yuji"));

    if(Config.value("3v3/UsingNewMode", false).toBool()){
          QStringList list_remove, list_add;
          list_remove << "zhangjiao" << "caoren" << "lvmeng" << "xiahoudun" << "weiyan";
          list_add << "sunjian" << "menghuo" << "xuhuang" << "pangde" << "zhugejin";
          foreach(QString general_name, list_remove)
              generals.removeOne(Sanguosha->getGeneral(general_name));
          foreach(QString general_name, list_add)
              generals << Sanguosha->getGeneral(general_name);
    }

    QStringList general_names;
    foreach(const General *general, generals)
        general_names << general->objectName();

    return general_names;
}

void RoomThread3v3::run()
{
    // initialize the random seed for this thread
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    QString scheme = Config.value("3v3/RoleChoose", "Normal").toString();
    assignRoles(scheme);
    room->adjustSeats();

    foreach(ServerPlayer *player, room->m_players){
        switch(player->getRoleEnum()){
        case Player::Lord: warm_leader = player; break;
        case Player::Renegade: cool_leader = player; break;
        default:
            break;
        }
    }

    if(Config.value("3v3/UsingExtension", false).toBool())
        general_names = Config.value("3v3/ExtensionGenerals").toStringList();
    else
        general_names = getGeneralsWithoutExtension();

    qShuffle(general_names);
    general_names = general_names.mid(0, 16);

    room->broadcastInvoke("fillGenerals", general_names.join("+"));

    QString order = room->askForOrder(warm_leader);
    ServerPlayer *first, *next;
    if(order == "warm"){
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

    room->sem->acquire(2);
}

void RoomThread3v3::askForTakeGeneral(ServerPlayer *player){
    QString name;
    if(general_names.length() == 1 || player->getState() != "online")
        name = GeneralSelector::GetInstance()->select3v3(player, general_names);

    if(name.isNull()){
        player->invoke("askForGeneral3v3");
    }else{
        msleep(1000);
        takeGeneral(player, name);
    }

    room->sem->acquire();
}

void RoomThread3v3::takeGeneral(ServerPlayer *player, const QString &name){
    general_names.removeOne(name);
    player->addToSelected(name);

    QString group = player->isLord() ? "warm" : "cool";
    room->broadcastInvoke("takeGeneral", QString("%1:%2").arg(group).arg(name));

    room->sem->release();
}

void RoomThread3v3::startArrange(ServerPlayer *player){
    if(!player->isOnline()){
        GeneralSelector *selector = GeneralSelector::GetInstance();
        arrange(player, selector->arrange3v3(player));
    }else{
        player->invoke("startArrange");        
    }
}

void RoomThread3v3::arrange(ServerPlayer *player, const QStringList &arranged){
    Q_ASSERT(arranged.length() == 3);

    if(player->isLord()){
        room->m_players.at(5)->setGeneralName(arranged.at(0));
        room->m_players.at(0)->setGeneralName(arranged.at(1));
        room->m_players.at(1)->setGeneralName(arranged.at(2));
        room->setTag(room->m_players.at(5)->objectName(),QStringList(arranged.at(0)));
        room->setTag(room->m_players.at(0)->objectName(),QStringList(arranged.at(1)));
        room->setTag(room->m_players.at(1)->objectName(),QStringList(arranged.at(2)));
    }else{
        room->m_players.at(2)->setGeneralName(arranged.at(0));
        room->m_players.at(3)->setGeneralName(arranged.at(1));
        room->m_players.at(4)->setGeneralName(arranged.at(2));
        room->setTag(room->m_players.at(2)->objectName(),QStringList(arranged.at(0)));
        room->setTag(room->m_players.at(3)->objectName(),QStringList(arranged.at(1)));
        room->setTag(room->m_players.at(4)->objectName(),QStringList(arranged.at(2)));
    }

    room->sem->release();
}

void RoomThread3v3::assignRoles(const QStringList &roles, const QString &scheme){
    QStringList all_roles = roles;
    QStringList roleChoices = all_roles;
    roleChoices.removeDuplicates();
    QList<ServerPlayer *> new_players, abstained;
    for (int i = 0; i < 6; i++)
        new_players << NULL;

    foreach(ServerPlayer *player, room->m_players){
        if(player->isOnline()){
            
            QString role = room->askForRole(player, roleChoices, scheme);
            if(role != "abstain"){
                player->setRole(role);
                all_roles.removeOne(role);

                for(int i = 0; i < 6; i++){
                    if(roles.at(i) == role && new_players.at(i) == NULL){
                        new_players[i] = player;
                        break;
                    }
                }

                continue;
            }
        }

        abstained << player;
    }

    if(!abstained.isEmpty()){
        qShuffle(abstained);

        for(int i = 0; i < 6; i++){
            if(new_players.at(i) == NULL){
                new_players[i] = abstained.takeFirst();
                new_players.at(i)->setRole(roles.at(i));
            }
        }
    }

    room->m_players = new_players;
}

// there are 3 scheme
// Normal: choose team1 or team2
// Random: assign role randomly
// AllRoles: select roles directly
void RoomThread3v3::assignRoles(const QString &scheme){
    QStringList roles;
    roles << "lord" << "loyalist" << "rebel"
            << "renegade"  << "rebel" << "loyalist";

    if(scheme == "Random"){
        // the easiest way        
        qShuffle(room->m_players);

        int i;
        for(i=0; i<roles.length(); i++)
            room->setPlayerProperty(room->m_players.at(i), "role", roles.at(i));
    }else if(scheme == "AllRoles"){
        assignRoles(roles, scheme);
    }else{
        QStringList all_roles;
        all_roles << "leader1" << "guard1" << "guard2"
                    << "leader2" << "guard2" << "guard1";
        assignRoles(all_roles, scheme);

        QMap<QString, QString> map;
        if(qrand() % 2 == 0){
            map["leader1"] = "lord";
            map["guard1"] = "loyalist";
            map["leader2"] = "renegade";
            map["guard2"] = "rebel";
        }else{
            map["leader1"] = "renegade";
            map["guard1"] = "rebel";
            map["leader2"] = "lord";
            map["guard2"] = "loyalist";

            room->m_players.swap(0, 3);
            room->m_players.swap(1, 4);
            room->m_players.swap(2, 5);
        }

        foreach(ServerPlayer *player, room->m_players){
            player->setRole(map[player->getRole()]);
        }
    }

    foreach(ServerPlayer *player, room->m_players)
        room->broadcastProperty(player, "role");
}
