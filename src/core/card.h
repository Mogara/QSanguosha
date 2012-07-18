#ifndef CARD_H
#define CARD_H

#include <QObject>
#include <QMap>
#include <QIcon>

class Room;
class Player;
class ServerPlayer;
class Client;
class ClientPlayer;
class CardItem;

struct CardEffectStruct;
struct CardMoveStruct;
struct CardUseStruct;

class Card : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString suit READ getSuitString CONSTANT)
    Q_PROPERTY(bool red READ isRed STORED false CONSTANT)
    Q_PROPERTY(bool black READ isBlack STORED false CONSTANT)
    Q_PROPERTY(int id READ getId CONSTANT)
    Q_PROPERTY(int number READ getNumber WRITE setNumber)
    Q_PROPERTY(QString number_string READ getNumberString CONSTANT)
    Q_PROPERTY(QString type READ getType CONSTANT)
    Q_PROPERTY(bool target_fixed READ targetFixed)
    Q_PROPERTY(bool once READ isOnce CONSTANT)
    Q_PROPERTY(bool mute READ isMute CONSTANT)
    Q_PROPERTY(bool equipped READ isEquipped)
    Q_PROPERTY(Color color READ getColor)

    Q_ENUMS(Suit)
    Q_ENUMS(CardType)

public:
    // enumeration type
    enum Suit {Spade, Club, Heart, Diamond, NoSuit};
    enum Color{Red, Black, Colorless};

    static const Suit AllSuits[4];

    // card types
    enum CardType{
        Skill,
        Basic,
        Trick,
        Equip
    };

    // constructor
    Card(Suit suit, int number, bool target_fixed = false);

    // property getters/setters
    QString getSuitString() const;
    bool isRed() const;
    bool isBlack() const;
    int getId() const;
    void setId(int id);
    int getEffectiveId() const;
    QString getEffectIdString() const;

    int getNumber() const;
    void setNumber(int number);
    QString getNumberString() const;

    Suit getSuit() const;
    void setSuit(Suit suit);

    bool sameColorWith(const Card *other) const;
    Color getColor() const;
    QString getFullName(bool include_suit = false) const;
    QString getLogName() const;
    QString getName() const;
    QString getSkillName() const;
    void setSkillName(const QString &skill_name);
    QString getDescription() const;
    
    bool isOnce() const;
    bool isMute() const;
    bool willThrow() const;
    bool canJilei() const;
    bool hasPreAction() const;

    void setFlags(const QString &flag) const;
    bool hasFlag(const QString &flag) const;
    void clearFlags() const;

    QString getPackage() const;
    virtual bool isVirtualCard() const;
    virtual bool isEquipped() const;
    virtual QString getCommonEffectName() const;
    virtual bool match(const QString &pattern) const;

    virtual void addSubcard(int card_id);
    virtual void addSubcard(const Card *card);
    virtual QList<int> getSubcards() const;
    virtual void clearSubcards();
    virtual QString subcardString() const;
    virtual void addSubcards(const QList<const Card *> &cards);
    virtual int subcardsLength() const;

    virtual QString getType() const = 0;
    virtual QString getSubtype() const = 0;
    virtual CardType getTypeId() const = 0;
    virtual QString toString() const;
    virtual bool isNDTrick() const;

    // card target selection
    virtual bool targetFixed() const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    // @todo: the following two functions should be merged into one.
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self,
                              int &maxVotes) const;
    virtual bool isAvailable(const Player *player) const;
    
    virtual const Card *validate(const CardUseStruct *cardUse) const;
    virtual const Card *validateInResposing(ServerPlayer *user, bool &continuable) const;


    virtual void doPreAction(Room *room, const CardUseStruct &card_use) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;

    inline virtual bool isKindOf(const char* cardType) const { return inherits(cardType); }

    // static functions
    static bool CompareByColor(const Card *a, const Card *b);
    static bool CompareBySuitNumber(const Card *a, const Card *b);
    static bool CompareByType(const Card *a, const Card *b);
    static const Card *Parse(const QString &str);
    static Card *Clone(const Card *card);
    static QString Suit2String(Suit suit);
    static QString Number2String(int number);
    static QStringList IdsToStrings(const QList<int> &ids);
    static QList<int> StringsToIds(const QStringList &strings);
    static const int S_UNKNOWN_CARD_ID;
protected:
    QList<int> subcards;
    bool target_fixed;
    bool once;
    QString skill_name;
    bool mute;
    bool will_throw;
    bool can_jilei;
    bool has_preact;
    Suit suit;
    int number;
    int id;

    mutable QStringList flags;
};

class SkillCard: public Card{
    Q_OBJECT

public:
    SkillCard();
    void setUserString(const QString &user_string);

    virtual QString getSubtype() const;
    virtual QString getType() const;
    virtual CardType getTypeId() const;
    virtual QString toString() const;

protected:
    QString user_string;
};

class DummyCard: public SkillCard{
    Q_OBJECT

public:
    DummyCard();

    virtual QString getSubtype() const;
    virtual QString getType() const;
    virtual QString toString() const;
};

#endif // CARD_H
