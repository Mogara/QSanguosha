#ifndef AI_H
#define AI_H

class Room;
class ServerPlayer;

typedef int LuaFunction;

#include "card.h"
#include "roomthread.h"

#include <QString>
#include <QObject>

class AI: public QObject{
    Q_OBJECT
    Q_ENUMS(Relation);

public:
    AI(ServerPlayer *player);

    enum Relation { Friend, Enemy, Neutrality };
    Relation relationTo(const ServerPlayer *other) const;
    bool isFriend(const ServerPlayer *other) const;
    bool isEnemy(const ServerPlayer *other) const;

    QList<ServerPlayer *> getEnemies() const;
    QList<ServerPlayer *> getFriends() const;

    virtual void activate(CardUseStruct &card_use) = 0;
    virtual Card::Suit askForSuit() = 0;
    virtual QString askForKingdom() = 0;
    virtual bool askForSkillInvoke(const QString &skill_name, const QVariant &data) = 0;
    virtual QString askForChoice(const QString &skill_name, const QString &choices) = 0;
    virtual QList<int> askForDiscard(int discard_num, bool optional, bool include_equip) = 0;
    virtual int askForNullification(const QString &trick_name, ServerPlayer *from, ServerPlayer *to)  = 0;
    virtual int askForCardChosen(ServerPlayer *who, const QString &flags, const QString &reason)  = 0;
    virtual const Card *askForCard(const QString &pattern)  = 0;
    virtual QString askForUseCard(const QString &pattern, const QString &prompt)  = 0;
    virtual int askForAG(const QList<int> &card_ids, bool refsuable) = 0;
    virtual const Card *askForCardShow(ServerPlayer *requestor) = 0;
    virtual const Card *askForPindian() = 0;
    virtual ServerPlayer *askForPlayerChosen(const QList<ServerPlayer *> &targets) = 0;
    virtual const Card *askForSinglePeach(ServerPlayer *dying) = 0;
    virtual ServerPlayer *askForYiji(const QList<int> &cards, int &card_id) = 0;

protected:
    Room *room;
    ServerPlayer *self;
};

class TrustAI: public AI{
    Q_OBJECT

public:
    TrustAI(ServerPlayer *player);

    virtual void activate(CardUseStruct &card_use) ;
    virtual Card::Suit askForSuit() ;
    virtual QString askForKingdom() ;
    virtual bool askForSkillInvoke(const QString &skill_name, const QVariant &data) ;
    virtual QString askForChoice(const QString &skill_name, const QString &choices);
    virtual QList<int> askForDiscard(int discard_num, bool optional, bool include_equip) ;
    virtual int askForNullification(const QString &trick_name, ServerPlayer *from, ServerPlayer *to) ;
    virtual int askForCardChosen(ServerPlayer *who, const QString &flags, const QString &reason) ;
    virtual const Card *askForCard(const QString &pattern) ;
    virtual QString askForUseCard(const QString &pattern, const QString &prompt) ;
    virtual int askForAG(const QList<int> &card_ids, bool refsuable);
    virtual const Card *askForCardShow(ServerPlayer *requestor) ;
    virtual const Card *askForPindian() ;
    virtual ServerPlayer *askForPlayerChosen(const QList<ServerPlayer *> &targets) ;
    virtual const Card *askForSinglePeach(ServerPlayer *dying) ;
    virtual ServerPlayer *askForYiji(const QList<int> &cards, int &card_id);

    virtual bool useCard(const Card *card);
};

class LuaAI: public TrustAI{
    Q_OBJECT

public:
    LuaAI(ServerPlayer *player);

    virtual const Card *askForCardShow(ServerPlayer *requestor);
    virtual bool askForSkillInvoke(const QString &skill_name, const QVariant &data);
    virtual void activate(CardUseStruct &card_use);
    virtual QString askForUseCard(const QString &pattern, const QString &prompt);
    virtual ServerPlayer *askForYiji(const QList<int> &cards, int &card_id);

    LuaFunction callback;
};

#endif // AI_H
