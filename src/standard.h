#ifndef STANDARD_H
#define STANDARD_H

#include "package.h"
#include "card.h"

class StandardPackage:public Package{
    Q_OBJECT

public:
    StandardPackage();
    void addCards();
    void addGenerals();
};

class BasicCard:public Card{
    Q_OBJECT

public:
    BasicCard(Suit suit, int number):Card(suit, number){}
    virtual QString getType() const;
    virtual int getTypeId() const;
};

class TrickCard:public Card{
    Q_OBJECT

public:
    TrickCard(Suit suit, int number):Card(suit, number){}
    virtual QString getType() const;
    virtual int getTypeId() const;
};

class EquipCard:public Card{
    Q_OBJECT

public:
    EquipCard(Suit suit, int number):Card(suit, number, true){}
    virtual QString getType() const;
    virtual int getTypeId() const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class GlobalEffect:public TrickCard{
    Q_OBJECT

public:
    GlobalEffect(Suit suit, int number):TrickCard(suit, number){ target_fixed = true;}
    virtual QString getSubtype() const;
};

class AOE:public TrickCard{
    Q_OBJECT

public:
    AOE(Suit suit, int number):TrickCard(suit, number){ target_fixed = true;}
    virtual QString getSubtype() const;
};

class SingleTargetTrick: public TrickCard{
    Q_OBJECT

public:
    SingleTargetTrick(Suit suit, int number):TrickCard(suit, number){}
    virtual QString getSubtype() const;
};

class DelayedTrick:public TrickCard{
    Q_OBJECT

public:
    DelayedTrick(Suit suit, int number):TrickCard(suit, number){ }
    virtual QString getSubtype() const;
};

class Weapon:public EquipCard{
    Q_OBJECT

public:
    Weapon(Suit suit, int number, int range)
        :EquipCard(suit, number), range(range){}
    virtual QString getSubtype() const;

protected:
    int range;
};

class Armor:public EquipCard{
    Q_OBJECT

public:
    Armor(Suit suit, int number):EquipCard(suit, number){}
    virtual QString getSubtype() const;
};

class Horse:public EquipCard{
    Q_OBJECT

public:
    Horse(const QString &name, Suit suit, int number, int correct);
    virtual QString getSubtype() const;

private:
    int correct;
};

// cards of standard package

class Slash: public BasicCard{
    Q_OBJECT
public:
    Q_INVOKABLE Slash(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual void use(const QList<const ClientPlayer *> &targets) const;
    virtual void use(Room *room, ServerPlayer *source,  const QList<ServerPlayer *> &targets) const;
    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
protected:
    virtual bool isAvailableAtPlay() const;
};

class Jink: public BasicCard{
    Q_OBJECT
public:
    Q_INVOKABLE Jink(Card::Suit suit, int number);
    virtual QString getSubtype() const;
protected:
    virtual bool isAvailableAtPlay() const;
};

class Peach: public BasicCard{
    Q_OBJECT
public:
    Q_INVOKABLE Peach(Card::Suit suit, int number);
    virtual QString getSubtype() const;
protected:
    virtual bool isAvailableAtPlay() const;
};

class Shit:public BasicCard{
    Q_OBJECT
public:
    Q_INVOKABLE Shit(Card::Suit suit, int number);
    virtual QString getSubtype() const;

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class Snatch:public SingleTargetTrick{
    Q_OBJECT

public:
    Snatch(Suit suit, int number);

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
};



// Skill cards

class ZhihengCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhihengCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class RendeCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE RendeCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // STANDARD_H
