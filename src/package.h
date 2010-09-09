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

    QList<const QMetaObject *> getMetaObjects() const{
        return metaobjects;
    }

protected:
    QHash<QString,QString> t;
    QList<const QMetaObject *> metaobjects;
};

#define ADD_PACKAGE(name) extern "C" { Q_DECL_EXPORT Package *New##name(){ return new name##Package;}  }

#endif // PACKAGE_H
