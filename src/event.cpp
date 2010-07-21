#include "event.h"
#include "engine.h"

Event::Event(Player *source, Player *target)
    :QEvent(Sanguosha->getEventType()), source(source), target(target)
{
}

Player *Event::getSource() const{
    return source;
}

Player *Event::getTarget() const{
    return target;
}

