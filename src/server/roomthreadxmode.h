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
    //void takeGeneral(ServerPlayer *player, const QString &name);
    void arrange(ServerPlayer *player, const QStringList &arranged);
    void assignRoles(const QString &scheme);

protected:
    virtual void run();

private:
    Room *room;
    ServerPlayer *warm_leader, *cool_leader;
    QStringList general_names;
    QString result;

    //QStringList getGeneralsWithoutExtension() const;
    //void askForTakeGeneral(ServerPlayer *player);
    void startArrange(ServerPlayer *player, const QStringList &to_arrange);
    void assignRoles(const QStringList &roles, const QString &scheme);
};

#endif
