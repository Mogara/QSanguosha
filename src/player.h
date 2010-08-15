#ifndef PLAYER_H
#define PLAYER_H

#include "general.h"
#include "card.h"

#include <QObject>
#include <QTcpSocket>
#include <QStack>

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
    Q_PROPERTY(QString general READ getGeneral WRITE setGeneral)
    Q_PROPERTY(QString state READ getState WRITE setState)
    Q_PROPERTY(int handcard_num READ getHandcardNum)
    Q_PROPERTY(int seat READ getSeat WRITE setSeat)
    Q_PROPERTY(QString phase READ getPhaseString WRITE setPhaseString)
    Q_PROPERTY(bool face_up READ faceUp)
    Q_PROPERTY(bool alive READ isAlive WRITE setAlive)

    // distance related properties
    Q_PROPERTY(int attack_range READ getAttackRange WRITE setAttackRange)
    Q_PROPERTY(QString correct READ getCorrect WRITE setCorrect)

    Q_ENUMS(Phase)
    Q_ENUMS(Place)

public:
    enum Phase {Start, Judge, Draw, Play, Discard, Finish, NotActive};
    enum Place {Hand, Equip, DelayedTrick, Special, DiscardedPile};

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
    void setGeneral(const QString &general_name);
    QString getGeneral() const;
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

    bool faceUp() const;
    void turnOver();

    virtual int aliveCount() const = 0;
    int distanceTo(const Player *other) const;
    int getGeneralMaxHP() const;
    const General *getAvatarGeneral() const;

    const Card *replaceEquip(const Card *equip);
    void removeEquip(const Card *equip);

    virtual int getHandcardNum() const = 0;
    virtual void removeCard(const Card *card, Place place) = 0;
    virtual void addCard(const Card *card, Place place) = 0;

    const Weapon *getWeapon() const;
    const Armor *getArmor() const;
    const Horse *getDefensiveHorse() const;
    const Horse *getOffensiveHorse() const;

    void attachSkill(const Skill *skill, bool prepend = false);
    void detachSkill(const Skill *skill);
    QList<const Skill *> getSkills() const;

    QStack<const Card *> getJudgingArea() const;

    static void MoveCard(Player *src, Place src_place, Player *dest, Place dest_place, int card_id);

private:
    const General *general;
    int hp, max_hp, max_cards;
    QString role;
    QString state;
    int seat;
    bool alive;

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
    QList<const Skill *> skills;
    bool face_up;
    QStack<const Card *> judging_area;

signals:
    void general_changed();
    void role_changed(const QString &new_role);
    void state_changed(const QString &new_state);
};

#endif // PLAYER_H
