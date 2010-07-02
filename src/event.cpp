#include "event.h"
#include "engine.h"

Event::Event(const QScriptValue &value)
    :QEvent(Sanguosha->getEventType()), source(NULL), target(NULL), value(value)
{
    QScriptValue source_value = value.property("source");
    QScriptValue target_value = value.property("target");

    if(source_value.isQObject())
       source  = qobject_cast<Player*>(source_value.toQObject());

    if(target_value.isQObject())
        target = qobject_cast<Player*>(target_value.toQObject());

}

Player *Event::getSource() const{
    return source;
}

Player *Event::getTarget() const{
    return target;
}

QScriptValue Event::getValue() const{
    return value;
}
