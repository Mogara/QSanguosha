#ifndef CLIENTSTRUCT_H
#define CLIENTSTRUCT_H

#include "player.h"

#include <QMap>
#include <QWidget>

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

class ServerInfoWidget: public QWidget{
    Q_OBJECT

public:
    ServerInfoWidget(const ServerInfoStruct &info, const QString &address);
};

struct CardMoveStructForClient{
    int card_id;
    ClientPlayer *from, *to;
    Player::Place from_place, to_place;

    bool parse(const QString &str);
};

#endif // CLIENTSTRUCT_H
