#ifndef CARD_H
#define CARD_H

#include <QObject>
#include <QMap>
#include <QIcon>

class Room;
class ServerPlayer;
class Client;
class ClientPlayer;
struct CardEffectStruct;

class Card : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString suit READ getSuitString CONSTANT)
    Q_PROPERTY(bool red READ isRed STORED false CONSTANT)
    Q_PROPERTY(bool black READ isBlack STORED false CONSTANT)
    Q_PROPERTY(int id READ getId CONSTANT)
    Q_PROPERTY(int number READ getNumber CONSTANT)    
    Q_PROPERTY(QString number_string READ getNumberString CONSTANT)
    Q_PROPERTY(QString type READ getType CONSTANT)
    Q_PROPERTY(QString pixmap_path READ getPixmapPath)
    Q_PROPERTY(bool target_fixed READ targetFixed)

    Q_ENUMS(Suit)

public:
    // enumeration type
    enum Suit {Spade, Club, Heart, Diamond, NoSuit};

    // constructor
    Card(Suit suit, int number, bool target_fixed = false);

    // property getters, as all properties of card is read only, no setter is defined
    QString getSuitString() const;
    bool isRed() const;
    bool isBlack() const;
    int getId() const;
    void setId(int id);
    int getNumber() const;
    QString getNumberString() const;
    Suit getSuit() const;
    QString getPixmapPath() const;
    QString getIconPath() const;
    QString getPackage() const;
    QIcon getSuitIcon() const;
    QString getFullName(bool include_suit = false) const;
    QString getName() const;

    bool isVirtualCard() const;
    virtual bool match(const QString &pattern) const;

    void addSubcard(int card_id);
    void addSubcards(const QList<int> &card_ids);
    QList<int> getSubcards() const;
    void clearSubcards();
    QString subcardString() const;

    virtual QString getType() const = 0;
    virtual QString getSubtype() const = 0;
    virtual int getTypeId() const = 0;
    virtual QString toString() const;

    // card target selection
    bool targetFixed() const;
    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual bool isAvailable() const;

    // FIXME: should be pure virtual
    virtual void use(Room *room, ServerPlayer *source,  const QList<ServerPlayer *> &targets) const;
    virtual void use(const QList<const ClientPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;

    // static functions
    static bool CompareBySuitNumber(const Card *a, const Card *b);
    static bool CompareByType(const Card *a, const Card *b);

    static const Card *Parse(const QString &str);

protected:
    QList<int> subcards;
    bool target_fixed;

private:
    Suit suit;
    int number;
    int id;    
};

class SkillCard: public Card{
    Q_OBJECT

public:
    SkillCard();
    virtual QString getSubtype() const;    
    virtual QString getType() const;
    virtual int getTypeId() const;
    virtual QString toString() const;
};

#endif // CARD_H
