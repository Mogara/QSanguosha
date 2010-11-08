#ifndef CLIENTSTRUCT_H
#define CLIENTSTRUCT_H

#include "player.h"

#include <QMap>

struct ServerInfoStruct{
    bool parse(const QString &str);

    QString Name;
    int PlayerCount;
    int OperationTimeout;
    QString Scenario;
    QMap<QString, bool> Extensions;
    bool FreeChoose;
    bool Enable2ndGeneral;
};

extern ServerInfoStruct ServerInfo;

struct CardMoveStructForClient{
    int card_id;
    ClientPlayer *from, *to;
    Player::Place from_place, to_place;

    bool parse(const QString &str);
};

#endif // CLIENTSTRUCT_H
