#ifndef PACKAGE_H
#define PACKAGE_H

class Skill;

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

    QList<const Skill *> getSkills() const{
        return skills;
    }

protected:
    QHash<QString,QString> t;
    QList<const QMetaObject *> metaobjects;
    QList<const Skill *> skills;
};

#define ADD_PACKAGE(name) extern "C" { Q_DECL_EXPORT Package *New##name(){ return new name##Package;}  }

#endif // PACKAGE_H
