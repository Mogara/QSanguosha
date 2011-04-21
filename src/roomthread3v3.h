#ifndef ROOMTHREAD3V3_H
#define ROOMTHREAD3V3_H

class Room;

#include <QThread>
#include <QSemaphore>

#include "serverplayer.h"

class RoomThread3v3 : public QThread
{
    Q_OBJECT

public:
    explicit RoomThread3v3(Room *room);
    void takeGeneral(ServerPlayer *player, const QString &name);
    void arrange(ServerPlayer *player, const QStringList &arranged);

protected:
    virtual void run();

private:
    Room *room;
    ServerPlayer *warm_leader, *cool_leader;
    QList<const General *> generals;
    QStringList general_names;
    QString result;
    QSemaphore sem;

    void askForTakeGeneral(ServerPlayer *player);
    void startArrange(ServerPlayer *player);
};

#endif // ROOMTHREAD3V3_H
