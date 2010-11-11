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

class QLabel;
class QListWidget;

class ServerInfoWidget: public QWidget{
    Q_OBJECT

public:
    ServerInfoWidget();
    void fill(const ServerInfoStruct &info, const QString &address);
    void clear();

private:
    QLabel *name_label;
    QLabel *address_label;
    QLabel *port_label;
    QLabel *player_count_label;
    QLabel *two_general_label;
    QLabel *free_choose_label;
    QLabel *scenario_label;
    QLabel *time_limit_label;
    QListWidget *list_widget;
};

struct CardMoveStructForClient{
    int card_id;
    ClientPlayer *from, *to;
    Player::Place from_place, to_place;

    bool parse(const QString &str);
};

#endif // CLIENTSTRUCT_H
