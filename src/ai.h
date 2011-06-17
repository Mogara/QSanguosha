#ifndef AI_H
#define AI_H

class Room;
class ServerPlayer;
class TrickCard;
class ResponseSkill;

struct lua_State;

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
    static Relation GetRelation3v3(const ServerPlayer *a, const ServerPlayer *b);
    static Relation GetRelationBoss(const ServerPlayer *a, const ServerPlayer *b);
    static Relation GetRelation(const ServerPlayer *a, const ServerPlayer *b);
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
    virtual QList<int> askForDiscard(const QString &reason, int discard_num, bool optional, bool include_equip) = 0;
    virtual const Card *askForNullification(const TrickCard *trick, ServerPlayer *from, ServerPlayer *to, bool positive) = 0;
    virtual int askForCardChosen(ServerPlayer *who, const QString &flags, const QString &reason)  = 0;
    virtual const Card *askForCard(const QString &pattern, const QString &prompt)  = 0;
    virtual QString askForUseCard(const QString &pattern, const QString &prompt)  = 0;
    virtual int askForAG(const QList<int> &card_ids, bool refusable, const QString &reason) = 0;
    virtual const Card *askForCardShow(ServerPlayer *requestor, const QString &reason) = 0;
    virtual const Card *askForPindian(ServerPlayer *requestor, const QString &reason) = 0;
    virtual ServerPlayer *askForPlayerChosen(const QList<ServerPlayer *> &targets, const QString &reason) = 0;
    virtual const Card *askForSinglePeach(ServerPlayer *dying) = 0;
    virtual ServerPlayer *askForYiji(const QList<int> &cards, int &card_id) = 0;
    virtual void askForGuanxing(const QList<int> &cards, QList<int> &up, QList<int> &bottom, bool up_only) = 0;
    virtual void filterEvent(TriggerEvent event, ServerPlayer *player, const QVariant &data);

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
    virtual QList<int> askForDiscard(const QString &reason, int discard_num, bool optional, bool include_equip) ;
    virtual const Card *askForNullification(const TrickCard *trick, ServerPlayer *from, ServerPlayer *to, bool positive);
    virtual int askForCardChosen(ServerPlayer *who, const QString &flags, const QString &reason) ;
    virtual const Card *askForCard(const QString &pattern, const QString &prompt);
    virtual QString askForUseCard(const QString &pattern, const QString &prompt) ;
    virtual int askForAG(const QList<int> &card_ids, bool refusable, const QString &reason);
    virtual const Card *askForCardShow(ServerPlayer *requestor, const QString &reason);
    virtual const Card *askForPindian(ServerPlayer *requestor, const QString &reason);
    virtual ServerPlayer *askForPlayerChosen(const QList<ServerPlayer *> &targets, const QString &reason);
    virtual const Card *askForSinglePeach(ServerPlayer *dying) ;
    virtual ServerPlayer *askForYiji(const QList<int> &cards, int &card_id);
    virtual void askForGuanxing(const QList<int> &cards, QList<int> &up, QList<int> &bottom, bool up_only);

    virtual bool useCard(const Card *card);

private:
    ResponseSkill *response_skill;
};

class LuaAI: public TrustAI{
    Q_OBJECT

public:
    LuaAI(ServerPlayer *player);

    virtual const Card *askForCardShow(ServerPlayer *requestor, const QString &reason);
    virtual bool askForSkillInvoke(const QString &skill_name, const QVariant &data);
    virtual void activate(CardUseStruct &card_use);
    virtual QString askForUseCard(const QString &pattern, const QString &prompt);
    virtual QList<int> askForDiscard(const QString &reason, int discard_num, bool optional, bool include_equip);
    virtual const Card *askForNullification(const TrickCard *trick, ServerPlayer *from, ServerPlayer *to, bool positive);
    virtual QString askForChoice(const QString &skill_name, const QString &choices);
    virtual int askForCardChosen(ServerPlayer *who, const QString &flags, const QString &reason);
    virtual const Card *askForCard(const QString &pattern, const QString &prompt);
    virtual ServerPlayer *askForPlayerChosen(const QList<ServerPlayer *> &targets, const QString &reason);
    virtual int askForAG(const QList<int> &card_ids, bool refusable, const QString &reason);

    virtual ServerPlayer *askForYiji(const QList<int> &cards, int &card_id);
    virtual void askForGuanxing(const QList<int> &cards, QList<int> &up, QList<int> &bottom, bool up_only);

    virtual void filterEvent(TriggerEvent event, ServerPlayer *player, const QVariant &data);

    LuaFunction callback;

private:
    void pushCallback(lua_State *L, const char *function_name);
    void pushQIntList(lua_State *L, const QList<int> &list);
    void reportError(lua_State *L);
    bool getTable(lua_State *L, QList<int> &table);
};

#endif // AI_H
