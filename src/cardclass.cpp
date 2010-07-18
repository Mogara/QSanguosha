#include "cardclass.h"

CardClass::CardClass(const QString &name, const QString &type, const QString &subtype,
                     int id, const QString &pixmap_dir)
    :type(type), subtype(subtype), id(id), pixmap_dir(pixmap_dir)
{
    setObjectName(name);
}

QString CardClass::getPixmapPath() const{
    return QString("%1/%2.png").arg(pixmap_dir).arg(objectName());
}

QScriptValue CardClass::availableFunc() const{
    return available_func;
}

void CardClass::setAvailableFunc(const QScriptValue &func){
    if(func.isFunction())
        available_func = func;
}

bool CardClass::isAvailable(const QScriptValue &env){
    if(available_func.isFunction())
        return available_func.call(env, QScriptValue::UndefinedValue).toBool();
    else
        return false;
}
