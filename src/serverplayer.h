#ifndef SERVERPLAYER_H
#define SERVERPLAYER_H

class Room;

#include "player.h"

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

    virtual int aliveCount() const;
    virtual int getHandcardNum() const;
    virtual void removeCard(const Card *card, Place place);
    virtual void addCard(const Card *card, Place place);

private:
    QTcpSocket *socket;
    QList<const Card*> handcards;
    Room *room;

private slots:
    void getRequest();

signals:
    void disconnected();
    void request_got(const QString &request);
};

#endif // SERVERPLAYER_H
