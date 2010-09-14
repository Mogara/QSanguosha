#ifndef ROOMTHREAD_H
#define ROOMTHREAD_H

class Room;
class TriggerSkill;
class ServerPlayer;
class Card;
class Slash;

#include "player.h"

#include <QThread>
#include <QSemaphore>
#include <QVariant>

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
    bool qinggang;
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
    bool qinggang;

    DamageStruct::Nature nature;
};

Q_DECLARE_METATYPE(SlashEffectStruct)

struct CardUseStruct{
    const Card *card;
    ServerPlayer *from;
    QList<ServerPlayer *> to;
};

Q_DECLARE_METATYPE(CardUseStruct);

struct CardMoveStruct{
    int card_id;
    Player::Place from_place, to_place;
    ServerPlayer *from, *to;
    bool open;

    QString toString(bool card_known = true) const;
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
    bool qinggang;
    bool success;
};

Q_DECLARE_METATYPE(SlashResultStruct);

enum TriggerEvent{
    GameStart,
    PhaseChange,

    Predamage,
    Predamaged,
    Damage,
    Damaged,

    Death,
    JudgeOnEffect,

    SlashEffect,
    Jinked,
    SlashResult,

    CardUsed,
    CardLost,
    CardGot,    

    CardEffect,
    CardEffected,
};

class RoomThread : public QThread{
    Q_OBJECT

public:
    explicit RoomThread(Room *room);
    bool trigger(TriggerEvent event, ServerPlayer *target, QVariant &data);
    bool trigger(TriggerEvent event, ServerPlayer *target);
    void addTriggerSkill(const TriggerSkill *skill);
    void removeTriggerSkill(const TriggerSkill *skill);
    void delay(unsigned long secs);

protected:
    virtual void run();

private:
    Room *room;

    QMap<QString, const TriggerSkill *> trigger_skills;
    QMap<TriggerEvent, QList<const TriggerSkill *> > skill_table;
};

#endif // ROOMTHREAD_H
