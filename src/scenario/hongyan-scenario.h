#ifndef HONGYANSCENARIO_H
#define HONGYANSCENARIO_H

#include "scenario.h"
#include "standard-skillcards.h"

class LesbianJieyinCard : public JieyinCard{
    Q_OBJECT

public:
    Q_INVOKABLE LesbianJieyinCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class LesbianLijianCard: public LijianCard{
    Q_OBJECT

public:
    Q_INVOKABLE LesbianLijianCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class LesbianLianliCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LesbianLianliCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class LesbianLianliSlashCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LesbianLianliSlashCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HongyanScenario : public Scenario{
    Q_OBJECT

public:
    explicit HongyanScenario();

    virtual bool exposeRoles() const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual void onTagSet(Room *room, const QString &key) const;

private:
    QStringList females;
    QStringList standard_roles;
};

#endif // HONGYANSCENARIO_H
