#ifndef ROOMTHREAD_H
#define ROOMTHREAD_H

class Room;
class PassiveSkill;
class ServerPlayer;
class Card;

#include "player.h"

#include <QThread>
#include <QSemaphore>
#include <QVariant>

struct PassiveSkillSorter{
    ServerPlayer *target;

    bool operator()(const PassiveSkill *a, const PassiveSkill *b);
    void sort(QList<const PassiveSkill *> &skills);
};

enum DamageNature{
    Normal, // normal slash, duel and most damage caused by skill
    Fire,  // fire slash, fire attack and few damage skill (Yeyan, etc)
    Thunder // lightning, thunder slash, and few damage skill (Leiji, etc)
};

struct DamageStruct{
    DamageStruct();

    ServerPlayer *from;
    ServerPlayer *to;
    const Card *card;
    int damage;
    DamageNature nature;
};

Q_DECLARE_METATYPE(DamageStruct);

struct CardEffectStruct{
    const Card *card;

    ServerPlayer *from;
    ServerPlayer *to;
};

Q_DECLARE_METATYPE(CardEffectStruct);

struct SlashEffectStruct{
    const Card *slash;

    DamageNature nature;
};

Q_DECLARE_METATYPE(SlashEffectStruct);

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

    QString toString() const;
};

Q_DECLARE_METATYPE(CardMoveStruct);

enum TriggerEvent{
    GameStart,
    PhaseChange,

    Predamage,
    Predamaged,
    Damage,
    Damaged,

    Dying,
    Death,

    Judge,
    JudgeOnEffect,

    CardUsed,
    CardMove,
    CardEffect,
};

class RoomThread : public QThread{
    Q_OBJECT

public:
    explicit RoomThread(Room *room);
    bool invokePassiveSkills(TriggerEvent event, ServerPlayer *target, const QVariant &data = QVariant());

protected:
    virtual void run();

private:
    Room *room;

    QMap<QString, const PassiveSkill *> passive_skills;
    QMap<TriggerEvent, QList<const PassiveSkill *> > trigger_table;
};

#endif // ROOMTHREAD_H
