#include "cardclass.h"

CardClass::CardClass(const QString &name, QObject *parent)
    :QObject(parent)
{
    setObjectName(name);
}
