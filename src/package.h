#ifndef PACKAGE_H
#define PACKAGE_H

#include <QObject>
#include <QHash>

class Package: public QObject{
    Q_OBJECT

public:
    Package(const QString &name){
        setObjectName(name);
    }

    const QHash<QString,QString> &getTranslation() const{
        return t;
    }

protected:
    QHash<QString,QString> t;
};

#endif // PACKAGE_H
