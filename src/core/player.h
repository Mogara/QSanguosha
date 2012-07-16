#ifndef PLAYER_H
#define PLAYER_H

#include "general.h"
#include "card.h"
#include <QObject>
#include <QTcpSocket>

class EquipCard;
class Weapon;
class Armor;
class Horse;
class DelayedTrick;
class DistanceSkill;
class TriggerSkill;

class Player : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString screenname READ screenName WRITE setScreenName)
    Q_PROPERTY(int hp READ getHp WRITE setHp)
    Q_PROPERTY(int maxhp READ getMaxHp WRITE setMaxHp)
    Q_PROPERTY(QString kingdom READ getKingdom WRITE setKingdom)
    Q_PROPERTY(bool wounded READ isWounded STORED false)
    Q_PROPERTY(QString role READ getRole WRITE setRole)
    Q_PROPERTY(QString general READ getGeneralName WRITE setGeneralName)
    Q_PROPERTY(QString general2 READ getGeneral2Name WRITE setGeneral2Name)
    Q_PROPERTY(QString state READ getState WRITE setState)
    Q_PROPERTY(int handcard_num READ getHandcardNum)
    Q_PROPERTY(int seat READ getSeat WRITE setSeat)
    Q_PROPERTY(QString phase READ getPhaseString WRITE setPhaseString)
    Q_PROPERTY(bool faceup READ faceUp WRITE setFaceUp)
    Q_PROPERTY(bool alive READ isAlive WRITE setAlive)
    Q_PROPERTY(QString flags READ getFlags WRITE setFlags)
    Q_PROPERTY(bool chained READ isChained WRITE setChained)
    Q_PROPERTY(bool owner READ isOwner WRITE setOwner)
    Q_PROPERTY(bool ready READ isReady WRITE setReady)
    Q_PROPERTY(int atk READ getAttackRange)
    Q_PROPERTY(General::Gender gender READ getGender)

    Q_PROPERTY(bool kongcheng READ isKongcheng)
    Q_PROPERTY(bool nude READ isNude)
    Q_PROPERTY(bool all_nude READ isAllNude)
    Q_PROPERTY(bool caocao READ isCaoCao)

    Q_ENUMS(Phase)
    Q_ENUMS(Place)
    Q_ENUMS(Role)

public:
    enum Phase {RoundStart, Start, Judge, Draw, Play, Discard, Finish, NotActive, PhaseNone};
    enum Place {PlaceHand, PlaceEquip, PlaceDelayedTrick, PlaceJudge, PlaceSpecial, DiscardPile, DrawPile, PlaceTable, PlaceUnknown};
    enum Role {Lord, Loyalist, Rebel, Renegade};

    explicit Player(QObject *parent);

    void setScreenName(const QString &screen_name);
    QString screenName() const;

    // property setters/getters
    int getHp() const;
    void setHp(int hp);
    int getMaxHp() const;
    void setMaxHp(int max_hp);
    int getLostHp() const;
    bool isWounded() const;
    General::Gender getGender() const;

    bool isOwner() const;
    void setOwner(bool owner);

    bool isReady() const;
    void setReady(bool ready);

    int getMaxCards() const;

    QString getKingdom() const;
    void setKingdom(const QString &kingdom);

    void setRole(const QString &role);
    QString getRole() const;
    Role getRoleEnum() const;

    void setGeneral(const General *general);
    void setGeneralName(const QString &general_name);
    QString getGeneralName() const;

    void setGeneral2Name(const QString &general_name);
    QString getGeneral2Name() const;
    const General *getGeneral2() const;

    void setState(const QString &state);
    QString getState() const;

    int getSeat() const;
    void setSeat(int seat);
    QString getPhaseString() const;
    void setPhaseString(const QString &phase_str);
    Phase getPhase() const;
    void setPhase(Phase phase);

    int getAttackRange() const;
    bool inMyAttackRange(const Player *other) const;

    bool isAlive() const;
    bool isDead() const;
    void setAlive(bool alive);

    QString getFlags() const;
    virtual void setFlags(const QString &flag);
    bool hasFlag(const QString &flag) const;
    void clearFlags();

    bool faceUp() const;
    void setFaceUp(bool face_up);

    virtual int aliveCount() const = 0;
    void setFixedDistance(const Player *player, int distance);
    int distanceTo(const Player *other, int distance_fix = 0) const;
    const General *getAvatarGeneral() const;
    const General *getGeneral() const;

    bool isLord() const;

    void acquireSkill(const QString &skill_name);
    void loseSkill(const QString &skill_name);
    void loseAllSkills();
    bool hasSkill(const QString &skill_name) const;
    bool hasInnateSkill(const QString &skill_name, bool includeLost = false) const;
    bool hasLordSkill(const QString &skill_name, bool includeLost = false) const;
    bool loseSkills() const;
    virtual QString getGameMode() const = 0;

    void setEquip(const EquipCard *card);
    void removeEquip(const EquipCard *equip);
    bool hasEquip(const Card *card) const;
    bool hasEquip() const;

    QList<const Card *> getJudgingArea() const;
    void addDelayedTrick(const Card *trick);
    void removeDelayedTrick(const Card *trick);
    QList<const DelayedTrick *> delayedTricks() const;
    bool containsTrick(const QString &trick_name) const;
    const DelayedTrick *topDelayedTrick() const;

    virtual int getHandcardNum() const = 0;
    virtual void removeCard(const Card *card, Place place) = 0;
    virtual void addCard(const Card *card, Place place) = 0;

    const Weapon *getWeapon() const;
    const Armor *getArmor() const;
    const Horse *getDefensiveHorse() const;
    const Horse *getOffensiveHorse() const;
    QList<const Card *> getEquips() const;
    const EquipCard *getEquip(int index) const;

    bool hasWeapon(const QString &weapon_name) const;
    bool hasArmorEffect(const QString &armor_name) const;

    bool isKongcheng() const;
    bool isNude() const;
    bool isAllNude() const;

    void addMark(const QString &mark);
    void removeMark(const QString &mark);
    virtual void setMark(const QString &mark, int value);
    int getMark(const QString &mark) const;

    void setChained(bool chained);
    bool isChained() const;

    bool canSlash(const Player *other, bool distance_limit = true, int rangefix = 0) const;
    int getCardCount(bool include_equip) const;

    QList<int> getPile(const QString &pile_name) const;
    QStringList getPileNames() const;
    QString getPileName(int card_id) const;

    void addHistory(const QString &name, int times = 1);
    void clearHistory();
    bool hasUsed(const QString &card_class) const;
    int usedTimes(const QString &card_class) const;
    int getSlashCount() const;

    QSet<const TriggerSkill *> getTriggerSkills() const;
    QSet<const Skill *> getVisibleSkills() const;
    QList<const Skill *> getVisibleSkillList() const;
    QSet<QString> getAcquiredSkills() const;

    virtual bool isProhibited(const Player *to, const Card *card) const;
    bool canSlashWithoutCrossbow() const;
    virtual bool isLastHandCard(const Card *card) const = 0;

    void jilei(const QString &type);
    bool isJilei(const Card *card) const;

    void setCardLocked(const QString &name);
    bool isLocked(const Card *card) const;
    bool hasCardLock(const QString &card_str) const;

    bool isCaoCao() const;
    void copyFrom(Player* p);

    QList<const Player *> getSiblings() const;

    QVariantMap tag;

protected:
    QMap<QString, int> marks;
    QMap<QString, QList<int> > piles;
    QSet<QString> acquired_skills;
    QSet<QString> flags;
    QHash<QString, int> history;

private:
    QString screen_name;
    bool owner;
    bool ready;
    const General *general, *general2;
    int hp, max_hp;
    QString kingdom;
    QString role;
    QString state;
    int seat;
    bool alive;

    Phase phase;
    const Weapon *weapon;
    const Armor *armor;
    const Horse *defensive_horse, *offensive_horse;
    bool face_up;
    bool chained;
    QList<int> judging_area;
    QHash<const Player *, int> fixed_distance;

    QSet<QString> jilei_set;
    QSet<QString> lock_card;

signals:
    void general_changed();
    void general2_changed();
    void role_changed(const QString &new_role);
    void state_changed();
    void hp_changed();
    void kingdom_changed();
    void phase_changed();
    void owner_changed(bool owner);
    void ready_changed(bool ready);
};

#endif // PLAYER_H
