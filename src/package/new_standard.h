#ifndef NEWSTANDARDPACKAGE_H
#define NEWSTANDARDPACKAGE_H

#include "package.h"
#include "card.h"

class NewLeijiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE NewLeijiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class NewRendeCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE NewRendeCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>

class NewStandardPackage : public Package{
    Q_OBJECT

public:
    NewStandardPackage();
};

#endif // NEWSTANDARDPACKAGE_H
