#include "event.h"
#include "engine.h"

Event::Event(const QString &name, Player *source, Player *target)
    :QEvent(Sanguosha->getEventType()), name(name), source(source), target(target), room(NULL)
{
}

QString Event::getName() const{
    return name;
}

Player *Event::getSource() const{
    return source;
}

Player *Event::getTarget() const{
    return target;
}

Event *Event::Parse(Room *room, const QString &str){
    return NULL;
}
