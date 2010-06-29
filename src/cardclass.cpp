#include "cardclass.h"

CardClass::CardClass(const QString &name, enum Type type, int id, QObject *parent)
    :QObject(parent), type(type), id(id)
{
    setObjectName(name);
}
