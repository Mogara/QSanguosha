#ifndef _ROOM_THREAD_XMODE_H
#define _ROOM_THREAD_XMODE_H

#include <QThread>
#include <QStringList>

class Room;
class ServerPlayer;

class RoomThreadXMode: public QThread {
    Q_OBJECT

public:
    explicit RoomThreadXMode(Room *room);
    void arrange(ServerPlayer *player, const QStringList &arranged);
    void assignRoles(const QString &scheme);

protected:
    virtual void run();

private:
    Room *room;
    ServerPlayer *warm_leader, *cool_leader;
    QStringList general_names;
    QString result;

    void startArrange(QList<ServerPlayer *> &players, QList<QStringList> &to_arrange);
    void assignRoles(const QStringList &roles, const QString &scheme);
};

#endif
