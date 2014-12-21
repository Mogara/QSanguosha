#ifndef ROOMINFOSTRUCT_H
#define ROOMINFOSTRUCT_H

#include <QSet>
#include <QString>
#include <QVariant>
#include <QSqlRecord>

#include "banpair.h"
#include "protocol.h"

class Settings;

//Configurations required by Room and RoomThread, usually transfered from the room owner to the lobby server
struct RoomConfig {
    RoomConfig(){}
    RoomConfig(const Settings *config);

    bool parse(const QVariant &data);
    QVariant toVariant() const;

    bool isBanned(const QString &first, const QString &second);

    QString GameMode;
    QString ServerName;
    int OperationTimeout;
    int CountDownSeconds;
    int NullificationCountDown;
    int OriginAIDelay;
    int AIDelay;
    int AIDelayAD;
    int LuckCardLimitation;
    int PileSwappingLimitation;
    int HegemonyMaxChoice;
    bool AIChat;
    bool RandomSeat;
    bool EnableCheat;
    bool FreeChoose;
    bool DisableChat;
    bool EnableMinimizeDialog;
    bool RewardTheFirstShowingPlayer;
    bool ForbidAddingRobot;
    bool AlterAIDelayAD;
    bool DisableLua;
    bool SurrenderAtDeath;
    bool EnableLordConvertion;
    QSet<QString> BanPackages;
    QSet<QString> CardConversions;
    QSet<QString> BannedGenerals;
    QSet<BanPair> BannedGeneralPairs;
    QString Password;
};

//Configurations exposed to all the clients, usually transfered from the room server to the clients
struct RoomInfoStruct {
    RoomInfoStruct();
    RoomInfoStruct(const Settings *config);
    RoomInfoStruct(const RoomConfig &config);
    RoomInfoStruct(const QSqlRecord &record);

    bool parse(const QVariant &var);
    QVariant toQVariant() const;
    qlonglong save();

    //Get the timeout allowance for a command. Server countdown is more lenient than the client.
    //@param command: type of command
    //@return countdown for command in milliseconds.
    time_t getCommandTimeout(QSanProtocol::CommandType command, QSanProtocol::ProcessInstanceType instance);

    qlonglong RoomId;
    QString HostAddress;
    int PlayerNum;

    enum State{
        Unknown,
        Waiting,
        Playing,
        Finished
    };
    State RoomState;

    QString Name;
    QString GameMode;
    QSet<QString> BanPackages;
    int OperationTimeout;
    int NullificationCountDown;
    bool RandomSeat;
    bool EnableCheat;
    bool FreeChoose;
    bool ForbidAddingRobot;
    bool DisableChat;
    bool FirstShowingReward;
    bool RequirePassword;
};

#endif // ROOMINFOSTRUCT_H
