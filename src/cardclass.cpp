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
