#include "roomthread1v1.h"
#include "room.h"
#include "engine.h"
#include "settings.h"
#include "generalselector.h"
#include "jsonutils.h"

#include <QDateTime>

using namespace QSanProtocol;
using namespace QSanProtocol::Utils;

RoomThread1v1::RoomThread1v1(Room *room)
    : room(room)
{
    room->getRoomState()->reset();
}

void RoomThread1v1::run() {
    // initialize the random seed for this thread
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
    bool new_rule = (Config.value("1v1/Rule", "Classical").toString() == "2013");
    int total_num = new_rule ? 12 : 10;

    if (!Config.value("1v1/UsingExtension", false).toBool()) {
        const Package *stdpack = Sanguosha->findChild<const Package *>("standard");
        const Package *windpack = Sanguosha->findChild<const Package *>("wind");

        QStringList candidates;
        if (!new_rule) {
            foreach (const General *general, stdpack->findChildren<const General *>())
                candidates << general->objectName();
            foreach (const General *general, windpack->findChildren<const General *>())
                candidates << general->objectName();
        } else {
            candidates << "caocao" << "simayi" << "xiahoudun" << "kof_zhangliao"
                       << "kof_xuchu" << "guojia" << "kof_zhenji" << "kof_xiahouyuan"
                       << "caoren" << "dianwei" << "kof_guanyu" << "zhangfei"
                       << "zhugeliang" << "zhaoyun" << "kof_machao" << "kof_nos_huangyueying"
                       << "kof_huangzhong" << "kof_jiangwei" << "kof_menghuo" << "kof_zhurong"
                       << "sunquan" << "ganning" << "huanggai" << "zhouyu"
                       << "luxun" << "kof_sunshangxiang" << "sunjian" << "xiaoqiao"
                       << "lvbu" << "kof_nos_diaochan" << "yanliangwenchou" << "hejin";
        }
        qShuffle(candidates);
        general_names = candidates.mid(0, total_num);
    } else {
        QSet<QString> banset = Config.value("Banlist/1v1").toStringList().toSet();
        general_names = Sanguosha->getRandomGenerals(total_num, banset);
    }

    if (!new_rule) {
        QStringList known_list = general_names.mid(0, 6);
        unknown_list = general_names.mid(6, 4);

        for (int i = 0; i < 4; i++)
            general_names[i + 6] = QString("x%1").arg(QString::number(i));

        room->doBroadcastNotify(S_COMMAND_FILL_GENERAL, toJsonArray(known_list << "x0" << "x1" << "x2" << "x3"));
    } else {
        room->doBroadcastNotify(S_COMMAND_FILL_GENERAL, toJsonArray(general_names));
    }
    ServerPlayer *first = room->getPlayers().at(0), *next = room->getPlayers().at(1);
    askForTakeGeneral(first);

    while (general_names.length() > 1) {
        qSwap(first, next);

        askForTakeGeneral(first);
        askForTakeGeneral(first);
    }
    askForTakeGeneral(next);

    startArrange(QList<ServerPlayer *>() << first << next);
}

void RoomThread1v1::askForTakeGeneral(ServerPlayer *player) {
    while (room->isPaused()) {}

    QString name;
    if (general_names.length() == 1)
        name = general_names.first();
    else if (player->getState() != "online")
        name = GeneralSelector::getInstance()->select1v1(general_names);

    if (name.isNull()) {
        bool success = room->doRequest(player, S_COMMAND_ASK_GENERAL, Json::Value::null, true);
        Json::Value clientReply = player->getClientReply();
        if (success && clientReply.isString()) {
            name = toQString(clientReply.asCString());
            takeGeneral(player, name);
        } else {
            GeneralSelector *selector = GeneralSelector::getInstance();
            name = selector->select1v1(general_names);
            takeGeneral(player, name);
        }
    } else {
        msleep(Config.AIDelay);
        takeGeneral(player, name);
    }
}

void RoomThread1v1::takeGeneral(ServerPlayer *player, const QString &name) {
    QString group = player->isLord() ? "warm" : "cool";
    room->doBroadcastNotify(room->getOtherPlayers(player, true), S_COMMAND_TAKE_GENERAL, toJsonArray(group, name));

    QRegExp unknown_rx("x(\\d)");
    QString general_name = name;
    if (unknown_rx.exactMatch(name)) {
        int index = unknown_rx.capturedTexts().at(1).toInt();
        general_name = unknown_list.at(index);

        Json::Value arg(Json::arrayValue);
        arg[0] = index;
        arg[1] = toJsonString(general_name);
        room->doNotify(player, S_COMMAND_RECOVER_GENERAL, arg);
    }

    room->doNotify(player, S_COMMAND_TAKE_GENERAL, toJsonArray(group, general_name));

    QString namearg = unknown_rx.exactMatch(name) ? "anjiang" : name;
    foreach (ServerPlayer *p, room->getPlayers()) {
        LogMessage log;
        log.type = "#VsTakeGeneral";
        log.arg = group;
        log.arg2 = (p == player) ? general_name : namearg;
        room->doNotify(p, S_COMMAND_LOG_SKILL, log.toJsonValue());
    }

    general_names.removeOne(name);
    player->addToSelected(general_name);
}

void RoomThread1v1::startArrange(QList<ServerPlayer *> players) {
    while (room->isPaused()) {}
    QList<ServerPlayer *> online = players;
    foreach (ServerPlayer *player, players) {
        if (!player->isOnline()) {
            GeneralSelector *selector = GeneralSelector::getInstance();
            arrange(player, selector->arrange1v1(player));
            online.removeOne(player);
        }
    }
    if (online.isEmpty()) return;

    foreach (ServerPlayer *player, online)
        player->m_commandArgs = Json::Value::null;

    room->doBroadcastRequest(online, S_COMMAND_ARRANGE_GENERAL);

    foreach (ServerPlayer *player, online) {
        Json::Value clientReply = player->getClientReply();
        if (player->m_isClientResponseReady && clientReply.isArray() && clientReply.size() == 3) {
            QStringList arranged;
            tryParse(clientReply, arranged);
            arrange(player, arranged);
        } else {
            GeneralSelector *selector = GeneralSelector::getInstance();
            arrange(player, selector->arrange1v1(player));
        }
    }
}

void RoomThread1v1::arrange(ServerPlayer *player, const QStringList &arranged) {
    Q_ASSERT(arranged.length() == 3);

    QStringList left = arranged.mid(1, 2);
    player->tag["1v1Arrange"] = QVariant::fromValue(left);
    player->setGeneralName(arranged.first());

    foreach (QString general, arranged) {
        room->doNotify(player, S_COMMAND_REVEAL_GENERAL, toJsonArray(player->objectName(), general));
        if (Config.value("1v1/Rule", "Classical").toString() == "2013") break;
    }
}

