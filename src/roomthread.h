#ifndef ROOMTHREAD_H
#define ROOMTHREAD_H

class Room;
class TriggerSkill;
class ServerPlayer;
class Card;
class Slash;
class GameRule;

#include "player.h"

#include <QThread>
#include <QSemaphore>
#include <QVariant>

#include <csetjmp>

struct TriggerSkillSorter{
    ServerPlayer *target;

    bool operator()(const TriggerSkill *a, const TriggerSkill *b);
    void sort(QList<const TriggerSkill *> &skills);
};

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

Q_DECLARE_METATYPE(DamageStruct);

struct CardEffectStruct{
    const Card *card;

    ServerPlayer *from;
    ServerPlayer *to;
};

Q_DECLARE_METATYPE(CardEffectStruct);

struct SlashEffectStruct{
    SlashEffectStruct();

    const Slash *slash;

    ServerPlayer *from;
    ServerPlayer *to;

    bool drank;

    DamageStruct::Nature nature;
};

Q_DECLARE_METATYPE(SlashEffectStruct)

struct CardUseStruct{
    CardUseStruct();
    bool isValid() const;
    void parse(const QString &str, Room *room);

    const Card *card;
    ServerPlayer *from;
    QList<ServerPlayer *> to;
};

Q_DECLARE_METATYPE(CardUseStruct);

struct CardMoveStruct{
    int card_id;
    Player::Place from_place, to_place;
    ServerPlayer *from, *to;

    QString toString() const;
};

Q_DECLARE_METATYPE(CardMoveStruct);

struct SlashResultStruct{
    SlashResultStruct();
    void fill(const SlashEffectStruct &effect, bool success);

    const Slash *slash;
    ServerPlayer *from;
    ServerPlayer *to;
    DamageStruct::Nature nature;
    bool drank;
    bool success;
};

Q_DECLARE_METATYPE(SlashResultStruct);

typedef const Card *CardStar;

Q_DECLARE_METATYPE(CardStar);

typedef ServerPlayer *PlayerStar;

Q_DECLARE_METATYPE(PlayerStar);

struct DyingStruct{
    DyingStruct();

    DamageStruct *damage; // if it is NULL that means the dying is caused by losing hp
    int peaches; // peaches that needs
};

Q_DECLARE_METATYPE(DyingStruct);

enum TriggerEvent{
    GameStart,
    PhaseChange,
    DrawNCards,
    JudgeOnEffect,

    Predamage,
    Predamaged,
    Damage,
    Damaged,

    Dying,
    Death,

    SlashEffect,
    SlashEffected,
    SlashProceed,
    SlashResult,

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

class RoomThread : public QThread{
    Q_OBJECT

public:
    explicit RoomThread(Room *room);
    void constructTriggerTable(const GameRule *rule);
    bool trigger(TriggerEvent event, ServerPlayer *target, QVariant &data);
    bool trigger(TriggerEvent event, ServerPlayer *target);
    void addTriggerSkill(const TriggerSkill *skill);
    void removeTriggerSkill(const TriggerSkill *skill);
    void removeTriggerSkill(const QString &skill_name);
    void delay(unsigned long msecs = 1000);
    void end();

protected:
    virtual void run();

private:
    Room *room;
    jmp_buf env;

    QMap<TriggerEvent, QList<const TriggerSkill *> > skill_table;
    QMap<const TriggerSkill *, int> refcount;
};

#endif // ROOMTHREAD_H
