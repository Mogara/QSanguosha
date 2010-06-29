#ifndef CARDCLASS_H
#define CARDCLASS_H

#include <QObject>

class Card;

class CardClass : public QObject
{
    Q_OBJECT
public:
    enum Type {Basic, Equip, Trick, UserDefined};
    friend class Card;

    explicit CardClass(const QString &name, enum Type type, int id, QObject *parent);

private:
    enum Type type;
    int id;    
};

#endif // CARDCLASS_H
