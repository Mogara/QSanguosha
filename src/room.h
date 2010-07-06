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
    void broadcast(const QString &message, Player *except = NULL);
    int drawCard();
    void drawCards(QList<int> &cards, int count);

    Q_INVOKABLE void pushEvent(const QScriptValue &event);

protected:
    virtual bool event(QEvent *);
    virtual void timerEvent(QTimerEvent *);

private:
    QList<Player*> players;
    int player_count;
    Player *focus;
    QList<int> pile1, pile2;
    QList<int> *draw_pile, *discard_pile;
    int left_seconds;
    int chosen_generals;

    Q_INVOKABLE void setCommand(Player *player, const QStringList &args);
    Q_INVOKABLE void signupCommand(Player *player, const QStringList &args);
    Q_INVOKABLE void chooseCommand(Player *player, const QStringList &args);

private slots:
    void reportDisconnection();
    void processRequest(const QString &request);
    void assignRoles();
    void startGame();

signals:
    void room_message(const QString &);
};

#endif // ROOM_H
