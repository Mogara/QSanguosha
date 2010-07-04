#ifndef ROOM_H
#define ROOM_H

#include "player.h"

#include <QScriptValue>
#include <QTcpSocket>

class Room : public QObject
{
    Q_OBJECT
public:
    explicit Room(QObject *parent, int player_count);
    void addSocket(QTcpSocket *socket);
    bool isFull() const;
    void unicast(QTcpSocket *socket, const QString &message);
    void broadcast(const QString &message, QTcpSocket *except = NULL);

    Q_INVOKABLE void pushEvent(const QScriptValue &event);

protected:
    virtual bool event(QEvent *);

private:
    QList<QTcpSocket*> sockets;
    QMap<QTcpSocket*, Player*> players;
    int player_count;
    Player *focus;

    Q_INVOKABLE void setCommand(QTcpSocket *socket, Player *player, const QStringList &args);
    Q_INVOKABLE void signupCommand(QTcpSocket *socket, Player *player, const QStringList &args);

private slots:
    void reportDisconnection();
    void reportMessage(QTcpSocket *socket, const QString &message);
    void getRequest();
    void startGame();

signals:
    void room_message(const QString &);
};

#endif // ROOM_H
