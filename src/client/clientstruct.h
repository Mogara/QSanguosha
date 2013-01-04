#ifndef CLIENTSTRUCT_H
#define CLIENTSTRUCT_H

#include "player.h"

#include <QMap>
#include <QWidget>

struct ServerInfoStruct{
    bool parse(const QString &str);

    QString Name;
    QString GameMode;
    int OperationTimeout;
    QStringList Extensions;
    bool FreeChoose;
    bool Enable2ndGeneral;
    bool EnableScene;
    bool EnableSame;
    bool EnableBasara;
    bool EnableHegemony;
    bool EnableAI;
    bool DisableChat;
    int MaxHPScheme;
    bool isPlay;
};

extern ServerInfoStruct ServerInfo;

class QLabel;
class QListWidget;

class ServerInfoWidget: public QWidget{
    Q_OBJECT

public:
    ServerInfoWidget(bool show_lack = false);
    void fill(const ServerInfoStruct &info, const QString &address);
    void updateLack(int count);
    void clear();

private:
    QLabel *name_label;
    QLabel *address_label;
    QLabel *port_label;
    QLabel *game_mode_label;
    QLabel *player_count_label;
    QLabel *two_general_label;
    QLabel *scene_label;
    QLabel *same_label;
    QLabel *basara_label;
    QLabel *hegemony_label;
    QLabel *max_hp_label;
    QLabel *free_choose_label;
    QLabel *enable_ai_label;
    QLabel *time_limit_label;
    QLabel *lack_label;
    QListWidget *list_widget;
};

struct CardMoveStructForClient{
    int card_id;
    ClientPlayer *from, *to;
    Player::Place from_place, to_place;

    bool parse(const QString &str);
};

#endif // CLIENTSTRUCT_H
