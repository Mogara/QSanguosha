#ifndef ROOMINFOSTRUCT_H
#define ROOMINFOSTRUCT_H

#include <QSet>
#include <QString>
#include <QVariant>

#include "banpair.h"

class Settings;

struct RoomConfig{
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
    bool OperationNoLimit;
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

#endif // ROOMINFOSTRUCT_H
