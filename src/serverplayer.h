#ifndef SERVERPLAYER_H
#define SERVERPLAYER_H

class Room;
struct CardMoveStruct;

#include "player.h"

#include <QMutex>

class ServerPlayer : public Player
{
    Q_OBJECT

public:
    explicit ServerPlayer(Room *room);
    void setSocket(QTcpSocket *socket);    
    void invoke(const char *method, const QString &arg = ".");
    QString reportHeader() const;
    void sendProperty(const char *property_name);
    void unicast(const QString &message);
    void drawCard(const Card *card);
    Room *getRoom() const;
    void playCardEffect(const Card *card);
    int getRandomHandCard() const;
    void leaveTo(ServerPlayer *legatee);
    void obtainCard(const Card *card);
    void throwAllEquips();
    void throwAllCards();
    void drawCards(int n);
    QList<int> handCards() const;
    bool isLord() const;

    virtual int aliveCount() const;
    virtual int getHandcardNum() const;
    virtual void removeCard(const Card *card, Place place);
    virtual void addCard(const Card *card, Place place);

private:
    QTcpSocket *socket;
    QList<const Card *> handcards;
    Room *room;

private slots:
    void getRequest();
    void castMessage(const QString &message);

signals:
    void disconnected();
    void request_got(const QString &request);
    void message_cast(const QString &message);
};

#endif // SERVERPLAYER_H
