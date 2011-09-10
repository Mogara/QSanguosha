#ifndef PACKAGE_H
#define PACKAGE_H

class Skill;
class Card;
class Player;

#include <QObject>
#include <QHash>
#include <QStringList>
#include <QMap>

class CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const = 0;
    virtual bool willThrow() const{
        return true;
    }
};

class Package: public QObject{
    Q_OBJECT

    Q_ENUMS(Type);

public:
    enum Type{
        GeneralPack,
        CardPack,
        MixedPack,
        SpecialPack,
    };

    Package(const QString &name){
        setObjectName(name);
        type = GeneralPack;
    }

    QList<const QMetaObject *> getMetaObjects() const{
        return metaobjects;
    }

    QList<const Skill *> getSkills() const{
        return skills;
    }

    QMap<QString, const CardPattern *> getPatterns() const{
        return patterns;
    }

    QMultiMap<QString, QString> getRelatedSkills() const{
        return related_skills;
    }

    Type getType() const{
        return type;
    }

    template<typename T>
    void addMetaObject(){
        metaobjects << &T::staticMetaObject;
    }

protected:
    QList<const QMetaObject *> metaobjects;
    QList<const Skill *> skills;
    QMap<QString, const CardPattern *> patterns;
    QMultiMap<QString, QString> related_skills;
    Type type;
};

#define ADD_PACKAGE(name) extern "C" { Q_DECL_EXPORT Package *New##name(){ return new name##Package;}  }

#endif // PACKAGE_H
