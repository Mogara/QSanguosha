#ifndef PLAYER_H
#define PLAYER_H

#include "general.h"
#include "card.h"
#include "standard.h"

#include <QObject>
#include <QTcpSocket>

class Player : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int hp READ getHp WRITE setHp)
    Q_PROPERTY(int max_hp READ getMaxHP WRITE setMaxHP)
    Q_PROPERTY(bool wounded READ isWounded STORED false)    
    Q_PROPERTY(QString name READ objectName WRITE setObjectName STORED false)
    Q_PROPERTY(QString role READ getRole WRITE setRole)
    Q_PROPERTY(QString general READ getGeneral WRITE setGeneral)
    Q_PROPERTY(QString state READ getState WRITE setState)
    Q_PROPERTY(int handcard_num READ getHandcardNum)
    Q_PROPERTY(int seat READ getSeat WRITE setSeat)

    // possible phase is start, judge, draw, play, discard, finish
    Q_PROPERTY(QString phase READ getPhase WRITE setPhase)

public:
    explicit Player(QObject *parent);

    // property setters/getters
    int getHp() const;
    void setHp(int hp);
    int getMaxHP() const;
    void setMaxHP(int max_hp);
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
    QString getPhase() const;
    void setPhase(const QString &phase);

    void setCorrect(int src_correct, int dest_correct);
    int distanceTo(const Player *other) const;
    int getGeneralMaxHP() const;
    const General *getAvatarGeneral() const;

    const Card *replaceEquip(const Card *equip);
    void removeEquip(const Card *equip);

    virtual int getHandcardNum() const = 0;
    virtual void removeCard(const Card *card, const QString &location) = 0;
    virtual void addCard(const Card *card, const QString &location) = 0;

    const Weapon *getWeapon() const;
    const Armor *getArmor() const;
    const Horse *getDefensiveHorse() const;
    const Horse *getOffensiveHorse() const;

    void attachSkill(const Skill *skill);
    QList<const Skill *> getSkills() const;

private:
    const General *general;
    int hp, max_hp;
    QString role;
    QString state;
    int seat;
    int src_correct, dest_correct;
    QString phase;
    const Weapon *weapon;
    const Armor *armor;
    const Horse *defensive_horse, *offensive_horse;
    QList<const Skill *> skills;

signals:
    void general_changed();
    void role_changed(const QString &new_role);
    void state_changed(const QString &new_state);
};

#endif // PLAYER_H
