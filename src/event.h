#ifndef EVENT_H
#define EVENT_H

#include "player.h"

#include <QEvent>
#include <QObject>
#include <QScriptValue>

class Event : public QEvent
{
public:
    Event(const QScriptValue &value);
    Player *getSource() const;
    Player *getTarget() const;
    QScriptValue getValue() const;

private:
    Player *source, *target;
    QScriptValue value;
};

#endif // EVENT_H
