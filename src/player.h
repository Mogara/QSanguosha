#ifndef PLAYER_H
#define PLAYER_H

#include "general.h"
#include "card.h"

#include <QObject>
#include <QTcpSocket>
#include <QStack>

class EquipCard;
class Weapon;
class Armor;
class Horse;

class Player : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int hp READ getHp WRITE setHp)
    Q_PROPERTY(int max_hp READ getMaxHP WRITE setMaxHP)
    Q_PROPERTY(int max_cards READ getMaxCards WRITE setMaxCards)
    Q_PROPERTY(bool wounded READ isWounded STORED false)    
    Q_PROPERTY(QString name READ objectName WRITE setObjectName STORED false)
    Q_PROPERTY(QString role READ getRole WRITE setRole)
    Q_PROPERTY(QString general READ getGeneralName WRITE setGeneralName)
    Q_PROPERTY(QString state READ getState WRITE setState)
    Q_PROPERTY(int handcard_num READ getHandcardNum)
    Q_PROPERTY(int seat READ getSeat WRITE setSeat)
    Q_PROPERTY(QString phase READ getPhaseString WRITE setPhaseString)
    Q_PROPERTY(bool face_up READ faceUp)
    Q_PROPERTY(bool alive READ isAlive WRITE setAlive)
    Q_PROPERTY(QString flags READ getFlags WRITE setFlags)

    // distance related properties
    Q_PROPERTY(int attack_range READ getAttackRange WRITE setAttackRange)
    Q_PROPERTY(QString correct READ getCorrect WRITE setCorrect)

    Q_PROPERTY(bool kongcheng READ isKongcheng)
    Q_PROPERTY(bool nude READ isNude)

    Q_ENUMS(Phase)
    Q_ENUMS(Place)

public:
    enum Phase {Start, Judge, Draw, Play, Discard, Finish, NotActive};
    enum Place {Hand, Equip, DelayedTrick, Special, DiscardedPile, DrawPile};

    explicit Player(QObject *parent);

    // property setters/getters
    int getHp() const;
    void setHp(int hp);
    int getMaxHP() const;
    void setMaxHP(int max_hp);
    int getMaxCards() const;
    void setMaxCards(int max_cards);
    bool isWounded() const;   
    void setRole(const QString &role);
    QString getRole() const;
    void setGeneralName(const QString &general_name);
    QString getGeneralName() const;
    void setState(const QString &state);
    QString getState() const;
    int getSeat() const;
    void setSeat(int seat);
    bool isFocus() const;
    void setFocus(bool focus);    
    QString getPhaseString() const;
    void setPhaseString(const QString &phase_str);
    Phase getPhase() const;
    void setPhase(Phase phase);
    Phase getNextPhase() const;

    void setAttackRange(int attack_range);
    int getAttackRange() const;
    QString getCorrect() const;
    void setCorrect(const QString &correct_str);

    bool isAlive() const;
    void setAlive(bool alive);

    QString getFlags() const;
    void setFlags(const QString &flag);
    bool hasFlag(const QString &flag) const;

    bool faceUp() const;
    void turnOver();

    virtual int aliveCount() const = 0;
    int distanceTo(const Player *other) const;
    int getGeneralMaxHP() const;
    const General *getAvatarGeneral() const;
    const General *getGeneral() const;
    bool hasSkill(const QString &skill_name) const;

    const EquipCard *getEquip(const QString &subtype) const;
    void setEquip(const EquipCard *card);
    void removeEquip(const EquipCard *equip);

    virtual int getHandcardNum() const = 0;
    virtual void removeCard(const Card *card, Place place) = 0;
    virtual void addCard(const Card *card, Place place) = 0;

    const Weapon *getWeapon() const;
    const Armor *getArmor() const;
    const Horse *getDefensiveHorse() const;
    const Horse *getOffensiveHorse() const;

    QStack<const Card *> getJudgingArea() const;
    bool isKongcheng() const;
    bool isNude() const;

private:
    const General *general;
    int hp, max_hp, max_cards;
    QString role;
    QString state;
    int seat;
    bool alive;
    QSet<QString> flags;

    struct CorrectStruct{
        int equip_src;
        int equip_dest;
        int skill_src;
        int skill_dest;
    };
    struct CorrectStruct correct;
    int attack_range;

    Phase phase;
    const Weapon *weapon;
    const Armor *armor;
    const Horse *defensive_horse, *offensive_horse;
    bool face_up;
    QStack<const Card *> judging_area;

signals:
    void general_changed();
    void role_changed(const QString &new_role);
    void state_changed();
};

#endif // PLAYER_H
