#ifndef ZOMBIE_MODE_H
#define ZOMBIE_MODE_H

#include "scenario.h"
#include "standard-skillcards.h"
#include "maneuvering.h"

class ZombieScenario : public Scenario{
    Q_OBJECT

public:
    explicit ZombieScenario();

    virtual bool exposeRoles() const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual void onTagSet(Room *room, const QString &key) const;

private:
    QStringList females;
};

class GanranEquip: public IronChain{
    Q_OBJECT

public:
    Q_INVOKABLE GanranEquip(Card::Suit suit, int number);
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
};

class PeachingCard: public QingnangCard{
    Q_OBJECT

public:
    Q_INVOKABLE PeachingCard();
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
};


#endif // ZOMBIE_MODE_H
