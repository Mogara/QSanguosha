#ifndef SERVERPLAYER_H
#define SERVERPLAYER_H

class Room;
struct CardMoveStruct;
class AI;

#include "player.h"
#include "socket.h"

#include <QMutex>

class ServerPlayer : public Player
{
    Q_OBJECT

public:
    explicit ServerPlayer(Room *room);
    void setSocket(ClientSocket *socket);
    void invoke(const char *method, const QString &arg = ".");
    QString reportHeader() const;
    void sendProperty(const char *property_name);
    void unicast(const QString &message);
    void drawCard(const Card *card);
    Room *getRoom() const;
    void playCardEffect(const Card *card);
    int getRandomHandCardId() const;
    const Card *getRandomHandCard() const;
    void obtainCard(const Card *card);
    void throwAllEquips();
    void throwAllHandCards();
    void throwAllCards();
    void drawCards(int n, bool set_emotion = true);
    bool askForSkillInvoke(const QString &skill_name, const QVariant &data = QVariant());
    QList<int> forceToDiscard(int discard_num, bool include_equip);
    QList<int> handCards() const;
    QList<const Card *> getHandcards() const;
    QList<const Card *> getCards(const QString &flags) const;
    DummyCard *wholeHandCards() const;
    bool isLord() const;
    bool hasNullification() const;
    void kick();
    bool pindian(ServerPlayer *target, const Card *card1 = NULL);

    void gainMark(const QString &mark, int n = 1);
    void loseMark(const QString &mark, int n = 1);

    void setAI(AI *ai);
    AI *getAI() const;

    virtual int aliveCount() const;
    virtual int getHandcardNum() const;
    virtual void removeCard(const Card *card, Place place);
    virtual void addCard(const Card *card, Place place);

private:
    ClientSocket *socket;
    QList<const Card *> handcards;
    Room *room;
    AI *ai;

private slots:
    void getMessage(char *message);
    void castMessage(const QString &message);

signals:
    void disconnected();
    void request_got(const QString &request);
    void message_cast(const QString &message);
};

#endif // SERVERPLAYER_H
