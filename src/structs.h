#ifndef STRUCTS_H
#define STRUCTS_H

class Room;
class TriggerSkill;
class ServerPlayer;
class Card;
class Slash;
class GameRule;

#include "player.h"

#include <QVariant>

struct DamageStruct{
    DamageStruct();

    enum Nature{
        Normal, // normal slash, duel and most damage caused by skill
        Fire,  // fire slash, fire attack and few damage skill (Yeyan, etc)
        Thunder // lightning, thunder slash, and few damage skill (Leiji, etc)
    };

    ServerPlayer *from;
    ServerPlayer *to;
    const Card *card;
    int damage;
    Nature nature;
    bool chain;
};

struct CardEffectStruct{
    CardEffectStruct();

    const Card *card;

    ServerPlayer *from;
    ServerPlayer *to;

    bool multiple;
};

struct SlashEffectStruct{
    SlashEffectStruct();

    const Slash *slash;

    ServerPlayer *from;
    ServerPlayer *to;

    bool drank;

    DamageStruct::Nature nature;
};

struct CardUseStruct{
    CardUseStruct();
    bool isValid() const;
    void parse(const QString &str, Room *room);

    const Card *card;
    ServerPlayer *from;
    QList<ServerPlayer *> to;
};

struct CardMoveStruct{
    int card_id;
    Player::Place from_place, to_place;
    ServerPlayer *from, *to;

    QString toString() const;
};

struct DyingStruct{
    DyingStruct();

    ServerPlayer *who; // who is ask for help
    DamageStruct *damage; // if it is NULL that means the dying is caused by losing hp
    int peaches; // peaches that needs
};

enum TriggerEvent{
    GameStart,
    PhaseChange,
    DrawNCards,
    JudgeOnEffect,
    HpRecover,

    Predamage,
    Predamaged,
    DamageDone,
    Damage,
    Damaged,
    DamageComplete,

    Dying,
    AskForPeaches,
    Death,

    SlashEffect,
    SlashEffected,
    SlashProceed,
    SlashHit,
    SlashMissed,

    CardAsked,
    CardUsed,
    CardResponsed,
    CardDiscarded,
    CardLost,
    CardGot,

    CardEffect,
    CardEffected,
    CardFinished
};

typedef const Card *CardStar;
typedef ServerPlayer *PlayerStar;

Q_DECLARE_METATYPE(DamageStruct);
Q_DECLARE_METATYPE(CardEffectStruct);
Q_DECLARE_METATYPE(SlashEffectStruct);
Q_DECLARE_METATYPE(CardUseStruct);
Q_DECLARE_METATYPE(CardMoveStruct);
Q_DECLARE_METATYPE(CardStar);
Q_DECLARE_METATYPE(PlayerStar);
Q_DECLARE_METATYPE(DyingStruct);

#endif // STRUCTS_H
