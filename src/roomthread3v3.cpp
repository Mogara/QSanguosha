#include "roomthread3v3.h"
#include "room.h"
#include "engine.h"
#include "ai.h"
#include "lua.hpp"
#include "settings.h"

#include <QDateTime>

static bool CompareByPriority(const QString &a, const QString &b){
    static QHash<QString, int> map;
    if(map.isEmpty()){
        map["zhangliao"] = 7;
        map["guojia"] = 7;
        map["liubei"] = 7;
        map["zhugeliang"] = 7;
        map["xunyu"] = 7;
        map["xuhuang"] = 7;
        map["lusu"] = 7;

        map["simayi"] = 6;
        map["huangyueying"] = 6;
        map["sunshangxiang"] = 6;
        map["huatuo"] = 6;
        map["yuanshao"] = 6;
        map["caopi"] = 6;
        map["dongzhuo"] = 6;

        map["zhenji"] = 5;
        map["zhangfei"] = 5;
        map["huanggai"] = 5;
        map["daqiao"] = 5;
        map["zhangjiao"] = 5;
        map["dianwei"] = 5;
        map["taishici"] = 5;
        map["shuangxiong"] = 5;
        map["sunjian"] = 5;

        map["caocao"] = 4;
        map["xuchu"] = 4;
        map["machao"] = 4;
        map["ganning"] = 4;
        map["lubu"] = 4;
        map["xiahouyuan"] = 4;
        map["huangzhong"] = 4;
        map["pangde"] = 4;
        map["zhurong"] = 4;

        map["sunquan"] = 3;
        map["zhouyu"] = 3;
        map["zhoutai"] = 3;
        map["jiaxu"] = 3;
        map["wolong"] = 3;
        map["pangtong"] = 3;

        map["xiahoudun"] = 2;
        map["guanyu"] = 2;
        map["zhaoyun"] = 2;
        map["lumeng"] = 2;
        map["luxun"] = 2;
        map["weiyan"] = 2;
        map["menghuo"] = 2;

        map["caoren"] = 1;
    }

    int p1 = map.value(a, 0);
    int p2 = map.value(b, 0);

    return p1 > p2;
}

static bool CompareByMaxHp(const QString &a, const QString &b){
    const General *g1 = Sanguosha->getGeneral(a);
    const General *g2 = Sanguosha->getGeneral(b);

    return g1->getMaxHp() < g2->getMaxHp();
}

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

    if(Config.value("3v3/UsingExtension", false).toBool())
        general_names = Config.value("3v3/ExtensionGenerals").toStringList();
    else
        general_names = getGeneralsWithoutExtension();

    qShuffle(general_names);
    general_names = general_names.mid(0, 16);
}

QStringList RoomThread3v3::getGeneralsWithoutExtension() const{
    QList<const General *> generals;

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

    QStringList general_names;
    foreach(const General *general, generals)
        general_names << general->objectName();

    return general_names;
}

void RoomThread3v3::run()
{
    // initialize the random seed for this thread
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    room->broadcastInvoke("fillGenerals", general_names.join("+"));
    qSort(general_names.begin(), general_names.end(), CompareByPriority);

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
        name = general_names.first();

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
    if(player->getState() != "online"){
        QStringList arranged = player->getSelected();
        qShuffle(arranged);
        arranged = arranged.mid(0, 3);

        qSort(arranged.begin(), arranged.end(), CompareByMaxHp);
        arranged.swap(0, 1);

        arrange(player, arranged);
    }else{
        player->invoke("startArrange");        
    }
}

void RoomThread3v3::arrange(ServerPlayer *player, const QStringList &arranged){
    Q_ASSERT(arranged.length() == 3);

    if(player->isLord()){
        room->players.at(5)->setGeneralName(arranged.at(0));
        room->players.at(0)->setGeneralName(arranged.at(1));
        room->players.at(1)->setGeneralName(arranged.at(2));
    }else{
        room->players.at(2)->setGeneralName(arranged.at(0));
        room->players.at(3)->setGeneralName(arranged.at(1));
        room->players.at(4)->setGeneralName(arranged.at(2));
    }

    room->sem->release();
}
