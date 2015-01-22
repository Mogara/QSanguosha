/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#ifndef _PACKAGE_H
#define _PACKAGE_H

#include <QObject>
#include <QHash>
#include <QStringList>
#include <QMap>

class Card;
class Player;
class Skill;

class CardPattern {
public:
    virtual bool match(const Player *player, const Card *card) const = 0;
    virtual bool willThrow() const{
        return true;
    }
    virtual QString getPatternString() const{
        return QString();
    }

    virtual ~CardPattern() {}
};

class Package : public QObject {
    Q_OBJECT
    Q_ENUMS(Type)

public:
    enum Type { GeneralPack, CardPack, MixedPack, SpecialPack };

    Package(const QString &name, Type pack_type = GeneralPack) {
        setObjectName(name);
        type = pack_type;
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

    QMultiMap<QString, QString> getConvertPairs() const{
        return convert_pairs;
    }

    Type getType() const{
        return type;
    }

    template<typename T>
    void addMetaObject() {
        metaobjects << &T::staticMetaObject;
    }

    inline void insertRelatedSkills(const QString &main_skill, const QString &related_skill) {
        related_skills.insertMulti(main_skill, related_skill);
    }

    void insertRelatedSkills(const QString &main_skill, int n, ...);

    inline void insertConvertPairs(const QString &from, const QString &to) {
        convert_pairs.insertMulti(from, to);
    }

    virtual ~Package();

protected:
    QList<const QMetaObject *> metaobjects;
    QList<const Skill *> skills;
    QMap<QString, const CardPattern *> patterns;
    QMultiMap<QString, QString> related_skills;
    QMultiMap<QString, QString> convert_pairs;
    Type type;
};

typedef QHash<QString, Package *> PackageHash;

class PackageAdder {
public:
    PackageAdder(const QString &name, Package *pack) {
        packages()[name] = pack;
    }

    static PackageHash &packages(void);
};

#define ADD_PACKAGE(name) static PackageAdder name##PackageAdder(#name, new name##Package);

#endif

