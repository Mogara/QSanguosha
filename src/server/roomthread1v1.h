#ifndef ROOMTHREAD1V1_H
#define ROOMTHREAD1V1_H

#include <QThread>
#include <QStringList>

class Room;
class ServerPlayer;

class RoomThread1v1 : public QThread
{
    Q_OBJECT

public:
    explicit RoomThread1v1(Room *room);
    void takeGeneral(ServerPlayer *player, const QString &name);
    void arrange(ServerPlayer *player, const QStringList &arranged);

protected:
    virtual void run();

private:
    Room *room;
    QStringList general_names;
    QStringList unknown_list;

    void askForTakeGeneral(ServerPlayer *player);
    void startArrange(ServerPlayer *player);
};

#endif // ROOMTHREAD1V1_H
