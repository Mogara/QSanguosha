#ifndef STANDARD_H
#define STANDARD_H

#include "package.h"
#include "card.h"
#include "event.h"

class StandardPackage:public Package{
    Q_OBJECT

public:
    StandardPackage();
    void addCards();
    void addGenerals();
};

class BasicCard:public Card{
public:
    BasicCard(enum Suit suit, int number):Card(suit, number){}
    virtual QString getType() const;
    virtual int getTypeId() const;
};

class TrickCard:public Card{
public:
    TrickCard(enum Suit suit, int number):Card(suit, number){}
    virtual QString getType() const;
    virtual int getTypeId() const;
};

class EquipCard:public Card{
public:
    EquipCard(enum Suit suit, int number):Card(suit, number){}
    virtual QString getType() const;
    virtual int getTypeId() const;
};

class GlobalEffect:public TrickCard{
public:
    GlobalEffect(enum Suit suit, int number):TrickCard(suit, number){}
    virtual QString getSubtype() const;
};

class AOE:public TrickCard{
public:
    AOE(enum Suit suit, int number):TrickCard(suit, number){}
    virtual QString getSubtype() const;
};

class SingleTargetTrick: public TrickCard{
public:
    SingleTargetTrick(enum Suit suit, int number):TrickCard(suit, number){}
    virtual QString getSubtype() const;
};

class DelayedTrick:public TrickCard{
public:
    DelayedTrick(enum Suit suit, int number):TrickCard(suit, number){ }
    virtual QString getSubtype() const;
};

class Weapon:public EquipCard{
public:
    Weapon(enum Suit suit, int number, int range)
        :EquipCard(suit, number), range(range){}
    virtual QString getSubtype() const;

protected:
    int range;
};

class Armor:public EquipCard{
public:
    Armor(enum Suit suit, int number):EquipCard(suit, number){}
    virtual QString getSubtype() const;
};

class Horse:public EquipCard{
public:
    Horse(const QString &name, enum Suit suit, int number, int correct);
    virtual QString getSubtype() const;

private:
    int correct;
};

#endif // STANDARD_H
