#ifndef CARDPATTERN_H
#define CARDPATTERN_H

#include <QObject>

class Card;

class CardPattern: public QObject{
    Q_OBJECT

public:
    CardPattern(const QString &pattern_str);
    QString toString() const;

    virtual bool match(const Card *card) const = 0;
protected:
    QString pattern_str;
};

class NamePattern: public CardPattern{
    Q_OBJECT

public:
    NamePattern(const QString &pattern_str);
    virtual bool match(const Card *card) const;
};

class TypePattern: public CardPattern{
    Q_OBJECT

public:
    TypePattern(const QString &pattern_str);
    virtual bool match(const Card *card) const;
};

class ClassPattern: public CardPattern{
    Q_OBJECT

public:
    ClassPattern(const QString &pattern_str);
    virtual bool match(const Card *card) const;
};

#endif // CARDPATTERN_H
