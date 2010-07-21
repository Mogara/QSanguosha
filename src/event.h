#ifndef EVENT_H
#define EVENT_H

#include "player.h"

#include <QEvent>
#include <QObject>

class Event : public QEvent
{
public:
    Event(Player *source, Player *target);
    Player *getSource() const;
    Player *getTarget() const;

private:
    Player *source, *target;
};

#endif // EVENT_H
