#include "cardclass.h"

CardClass::CardClass(const QString &name, enum Type type, int id, const QString &pixmap_path)
    :type(type), id(id), pixmap_path(pixmap_path)
{
    setObjectName(name);
}

QString CardClass::getPixmapPath() const{
    return QString("%1/%2.png").arg(pixmap_path).arg(objectName());
}
