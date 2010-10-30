#ifndef AI_H
#define AI_H

class Room;
class ServerPlayer;

#include "card.h"
#include "roomthread.h"

#include <QString>
#include <QObject>

class AI: public QObject{
    Q_OBJECT
    Q_ENUMS(Relation)

public:
    AI(ServerPlayer *player);

    static AI *CreateAI(ServerPlayer *player);

    enum Relation { Friend, Enemy, Neutrality };
    Relation relationTo(const ServerPlayer *other) const;

    bool isFriend(const ServerPlayer *other) const;
    bool isEnemy(const ServerPlayer *other) const;

    QList<ServerPlayer *> getEnemies() const;
    QList<ServerPlayer *> getFriends() const;

    virtual void activate(CardUseStruct &card_use) const = 0;
    virtual Card::Suit askForSuit() const = 0;
    virtual QString askForKingdom() const = 0;
    virtual bool askForSkillInvoke(const QString &skill_name, const QVariant &data) const = 0;
    virtual QString askForChoice(const QString &skill_name, const QString &choices) = 0;
    virtual QList<int> askForDiscard(int discard_num, bool optional, bool include_equip, Card::Suit suit) const = 0;
    virtual int askForNullification(const QString &trick_name, ServerPlayer *from, ServerPlayer *to) const = 0;
    virtual int askForCardChosen(ServerPlayer *who, const QString &flags, const QString &reason) const = 0;
    virtual const Card *askForCard(const QString &pattern) const = 0;
    virtual QString askForUseCard(const QString &pattern, const QString &prompt) const = 0;
    virtual int askForAG(const QList<int> &card_ids) const = 0;
    virtual int askForCardShow(ServerPlayer *requestor) const = 0;
    virtual const Card *askForPindian() const = 0;
    virtual ServerPlayer *askForPlayerChosen(const QList<ServerPlayer *> &targets) const = 0;
    virtual const Card *askForSinglePeach(ServerPlayer *dying) const = 0;

protected:
    Room *room;
    ServerPlayer *self;
};

class TrustAI: public AI{
    Q_OBJECT

public:
    TrustAI(ServerPlayer *player);

    virtual void activate(CardUseStruct &card_use) const;
    virtual Card::Suit askForSuit() const;
    virtual QString askForKingdom() const;
    virtual bool askForSkillInvoke(const QString &skill_name, const QVariant &data) const;
    virtual QString askForChoice(const QString &skill_name, const QString &choices);
    virtual QList<int> askForDiscard(int discard_num, bool optional, bool include_equip, Card::Suit suit) const;
    virtual int askForNullification(const QString &trick_name, ServerPlayer *from, ServerPlayer *to) const;
    virtual int askForCardChosen(ServerPlayer *who, const QString &flags, const QString &reason) const;
    virtual const Card *askForCard(const QString &pattern) const;
    virtual QString askForUseCard(const QString &pattern, const QString &prompt) const;
    virtual int askForAG(const QList<int> &card_ids) const;
    virtual int askForCardShow(ServerPlayer *requestor) const;
    virtual const Card *askForPindian() const;
    virtual ServerPlayer *askForPlayerChosen(const QList<ServerPlayer *> &targets) const;
    virtual const Card *askForSinglePeach(ServerPlayer *dying) const;
};

class SmartAI: public TrustAI{
    Q_OBJECT

public:
    SmartAI(ServerPlayer *player, bool always_invoke = false);

    virtual int askForCardShow(ServerPlayer *requestor) const;
    virtual bool askForSkillInvoke(const QString &skill_name, const QVariant &data) const;

private:
    bool always_invoke;
};

#endif // AI_H
