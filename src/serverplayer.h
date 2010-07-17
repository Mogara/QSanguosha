#ifndef SERVERPLAYER_H
#define SERVERPLAYER_H

#include "player.h"

class ServerPlayer : public Player
{
    Q_OBJECT

public:
    explicit ServerPlayer(QObject *parent);
    void setSocket(QTcpSocket *socket);
    void unicast(const QString &message);
    QString reportHeader() const;
    void sendProperty(const char *property_name);    
    void drawCard(const Card *card);

    virtual int getHandcardNum() const;
    virtual void removeCard(const Card *card, const QString &location);
    virtual void addCard(const Card *card, const QString &location);

private:
    QTcpSocket *socket;
    QList<const Card*> handcards;

private slots:
    void getRequest();

signals:
    void disconnected();
    void request_got(const QString &request);
};

#endif // SERVERPLAYER_H
