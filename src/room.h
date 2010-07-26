#ifndef ROOM_H
#define ROOM_H

#include "serverplayer.h"

#include <QTcpSocket>
#include <QStack>

class Room : public QObject
{
    Q_OBJECT
public:
    explicit Room(QObject *parent, int player_count);
    void addSocket(QTcpSocket *socket);
    bool isFull() const;

protected:
    virtual bool event(QEvent *);
    virtual void timerEvent(QTimerEvent *);

private:
    QList<ServerPlayer*> players, alive_players;
    const int player_count;
    Player *focus;
    QList<int> pile1, pile2;
    QList<int> *draw_pile, *discard_pile;
    int left_seconds;
    int chosen_generals;    
    bool game_started;
    int signup_count;
    QStack<ServerPlayer *> active_records;

    void broadcast(const QString &message, Player *except = NULL);
    int drawCard();
    void drawCards(ServerPlayer *player, int n);

    Q_INVOKABLE void setCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void signupCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void chooseCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void useCardCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void endPhaseCommand(ServerPlayer *player, const QStringList &args);
    Q_INVOKABLE void drawCardsCommand(ServerPlayer *player, const QStringList &args);

    // Q_INVOKABLE void pushEvent(const QString &name, QScriptValue &event);

private slots:
    void reportDisconnection();
    void processRequest(const QString &request);
    void assignRoles();
    void startGame();

signals:
    void room_message(const QString &);
};

#endif // ROOM_H
