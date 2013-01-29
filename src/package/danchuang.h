#ifndef TEST_H
#define TEST_H

#include "package.h"
#include "card.h"

class DanchuangPackage: public Package{
    Q_OBJECT

public:
    DanchuangPackage();
};

class V5QuanjiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE V5QuanjiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class V5YexinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE V5YexinCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // TEST_H
