#ifndef ROOM_H
#define ROOM_H

class PassiveSkill;

#include "serverplayer.h"
#include "cardpattern.h"

#include <QTcpSocket>
#include <QStack>
#include <QQueue>

struct PassiveSkillSorter{
    ServerPlayer *target;

    bool operator()(const PassiveSkill *a, const PassiveSkill *b);
    void sort(QList<const PassiveSkill *> &skills);
};

struct ActiveRecord{
    const char *method;    
    ServerPlayer *target;
    QVariant data;
};

struct DamageStruct{
    enum Nature {
        Normal, // normal slash, duel and most damage caused by skill
        Fire,  // fire slash, fire attack and few damage skill (Yeyan, etc)
        Thunder // lightning, thunder slash, and few damage skill (Leiji, etc)
    };
    DamageStruct();

    ServerPlayer *from;
    ServerPlayer *to;
    const Card *card;
    int damage;
    Nature nature;
};

Q_DECLARE_METATYPE(DamageStruct);

struct CardEffectStruct{
    const Card *card;

    ServerPlayer *from;
    ServerPlayer *to;
};

Q_DECLARE_METATYPE(CardEffectStruct);

struct CardUseStruct{
    const Card *card;
    ServerPlayer *from;
    QList<ServerPlayer *> to;
};

Q_DECLARE_METATYPE(CardUseStruct);

struct CardMoveStruct{
    enum Reason {
        Use,
        Discard,
        Response,
        Dismantle
    };

    Reason reason;
    int card_id;    
    Player::Place from_place, to_place;
    ServerPlayer *from, *to;
};

Q_DECLARE_METATYPE(CardMoveStruct);

class Room : public QObject
{
    Q_OBJECT

    Q_ENUMS(TriggerEvent)

public:
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

        Jink,
        Jinked,
    };

    explicit Room(QObject *parent, int player_count);
    void addSocket(QTcpSocket *socket);
    bool isFull() const;    
    void broadcast(const QString &message, ServerPlayer *except = NULL);
    void throwCard(ServerPlayer *player, const Card *card);
    void throwCard(ServerPlayer *player, int card_id);
    QList<int> *getDiscardPile() const;
    void moveCard(ServerPlayer *src, Player::Place src_place, ServerPlayer *dest, Player::Place dest_place, int card_id);
    void playSkillEffect(const QString &skill_name, int index = -1);

    void enqueueRecord(ActiveRecord *record);

    ServerPlayer *getCurrent() const;
    int alivePlayerCount() const;

    // non-interactive methods
    Q_INVOKABLE void activate(ServerPlayer *target);
    Q_INVOKABLE void nextPhase(ServerPlayer *player);
    Q_INVOKABLE void drawCards(ServerPlayer *player, const QVariant &data);
    Q_INVOKABLE void setPlayerFlag(ServerPlayer *player, const QVariant &flag);
    Q_INVOKABLE void setPlayerProperty(ServerPlayer *player, const QVariant &data);
    Q_INVOKABLE void useCard(ServerPlayer *player, const QVariant &data);



    // interactive methods
    Q_INVOKABLE void askForSkillInvoke(ServerPlayer *player, const QVariant &data);
    Q_INVOKABLE void askForNullification(ServerPlayer *player, const QVariant &data);
    Q_INVOKABLE void askForCardChosen(ServerPlayer *player, const QVariant &data);
    Q_INVOKABLE void requestForCard(ServerPlayer *player, const QVariant &data);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    QList<ServerPlayer*> players, alive_players;
    const int player_count;
    ServerPlayer *current;
    QList<int> pile1, pile2;
    QList<int> *draw_pile, *discard_pile;
    int left_seconds;
    int chosen_generals;
    bool game_started;
    const char *waiting_func;
    int nullificators_count;
    QList<ServerPlayer *> nullificators;
    int signup_count;

    QMap<QString, const PassiveSkill *> passive_skills;
    QStack<ActiveRecord *> stack;
    QQueue<ActiveRecord *> queue;
    QMap<TriggerEvent, QList<const PassiveSkill *> > trigger_table;

    int drawCard();
    void broadcastProperty(ServerPlayer *player, const char *property_name, const QString &value = QString());
    void broadcastInvoke(const char *method, const QString &arg = ".");
    void invokeStackTop();
    void invokePassiveSkills(TriggerEvent event, ServerPlayer *target, const QVariant &data = QVariant());

    // method that may invoke skills
    void changePhase(ServerPlayer *target);
    void predamage(ServerPlayer *target, const DamageStruct &data);
    void damage(ServerPlayer *target, const DamageStruct &data);

    Q_INVOKABLE void setCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void signupCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void chooseCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void useCardCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void judgeCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void invokeSkillCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void replyNullificationCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void chooseCardCommand(ServerPlayer *player, const QStringList &args);

private slots:
    void reportDisconnection();
    void processRequest(const QString &request);
    void assignRoles();
    void startGame();

signals:
    void room_message(const QString &);
};

#endif // ROOM_H
