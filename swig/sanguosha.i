%module sgs

%{

#include "structs.h"
#include "engine.h"
#include "client.h"
#include "namespace.h"

#include <QDir>

%}

%include "naturalvar.i"
%include "native.i"
%include "qvariant.i"
%include "list.i"

// ----------------------------------------

namespace BattleArrayType
{
    enum ArrayType {
        Siege,
        Formation
    };
};

class QObject {
public:
    QString objectName();
    void setObjectName(const char *name);
    bool inherits(const char *class_name);
    bool setProperty(const char *name, const QVariant &value);
    QVariant property(const char *name) const;
    void setParent(QObject *parent);
    void deleteLater();
};

class General: public QObject {
public:
    explicit General(Package *package, const char *name, const char *kingdom, int max_hp = 4, bool male = true, bool hidden = false, bool never_shown = false);

    // property getters/setters
    int getDoubleMaxHp() const;
    QString getKingdom() const;
    bool isMale() const;
    bool isFemale() const;
    bool isNeuter() const;
    bool isLord() const;
    bool isHidden() const;
    bool isTotallyHidden() const;

    int getMaxHpHead() const;
    int getMaxHpDeputy() const;

    enum Gender { Sexless, Male, Female, Neuter };
    Gender getGender() const;
    void setGender(Gender gender);

    void addSkill(Skill *skill);
    void addSkill(const char *skill_name);
    bool hasSkill(const char *skill_name) const;
    QList<const Skill *> getSkillList(bool relate_to_place = false, bool head_only = true) const;
    QList<const Skill *> getVisibleSkillList(bool relate_to_place = false, bool head_only = true) const;
    QSet<const Skill *> getVisibleSkills(bool relate_to_place = false, bool head_only = true) const;
    QSet<const TriggerSkill *> getTriggerSkills() const;

    void addRelateSkill(const char *skill_name);
    QStringList getRelatedSkillNames() const;

    QString getPackage() const;
    QString getSkillDescription(bool include_name = false, bool yellow = true) const;

    void addCompanion(const char *name);
    bool isCompanionWith(const char *name) const;

    void setHeadMaxHpAdjustedValue(const int adjusted_value = -1);
    void setDeputyMaxHpAdjustedValue(const int adjusted_value = -1);

    void lastWord() const;
};

class Player: public QObject {
public:
    enum Phase { RoundStart, Start, Judge, Draw, Play, Discard, Finish, NotActive, PhaseNone };
    enum Place { PlaceHand, PlaceEquip, PlaceDelayedTrick, PlaceJudge, PlaceSpecial, DiscardPile, DrawPile, PlaceTable, PlaceUnknown };
    enum Role { Lord, Loyalist, Rebel, Renegade };

    explicit Player(QObject *parent);

    void setScreenName(const char *screen_name);
    QString screenName() const;

    // property setters/getters
    int getHp() const;
    void setHp(int hp);
    int getMaxHp() const;
    void setMaxHp(int max_hp);
    int getLostHp() const;
    bool isWounded() const;
    General::Gender getGender() const;
    virtual void setGender(General::Gender gender);
    bool isMale() const;
    bool isFemale() const;
    bool isNeuter() const;

    bool hasShownRole() const;
    void setShownRole(bool shown);

    int getMaxCards() const;

    QString getKingdom() const;
    void setKingdom(const char *kingdom);

    void setRole(const char *role);
    QString getRole() const;
    Role getRoleEnum() const;

    void setGeneral(const General *general);
    void setGeneralName(const char *general_name);
    QString getGeneralName() const;

    void setGeneral2Name(const char *general_name);
    QString getGeneral2Name() const;
    const General *getGeneral2() const;

    void setState(const char *state);
    QString getState() const;

    int getSeat() const;
    void setSeat(int seat);
    bool isAdjacentTo(const Player *another) const;
    QString getPhaseString() const;
    void setPhaseString(const char *phase_str);
    Phase getPhase() const;
    void setPhase(Phase phase);

    int getAttackRange(bool include_weapon = true) const;
    bool inMyAttackRange(const Player *other) const;

    bool isAlive() const;
    bool isDead() const;
    void setAlive(bool alive);

    QString getFlags() const;
    QStringList getFlagList() const;
    virtual void setFlags(const char *flag);
    bool hasFlag(const char *flag) const;
    void clearFlags();

    bool faceUp() const;
    void setFaceUp(bool face_up);

    virtual int aliveCount() const = 0;
    void setFixedDistance(const Player *player, int distance);
    int distanceTo(const Player *other, int distance_fix = 0) const;
    const General *getAvatarGeneral() const;
    const General *getGeneral() const;

    bool isLord() const;

    void acquireSkill(const char *skill_name);
    void detachSkill(const char *skill_name);
    void detachAllSkills();
    virtual void addSkill(const char *skill_name, bool head_skill = true);
    virtual void loseSkill(const char *skill_name);
    bool hasSkill(const char *skill_name, bool include_lose = false) const;
    bool hasSkills(const char *skill_name, bool include_lose = false) const;
    bool hasInnateSkill(const char *skill_name) const;
    bool hasLordSkill(const char *skill_name, bool include_lose = false) const;
    virtual QString getGameMode() const = 0;

    void setEquip(WrappedCard *equip);
    void removeEquip(WrappedCard *equip);
    bool hasEquip(const Card *card) const;
    bool hasEquip() const;

    QList<const Card *> getJudgingArea() const;
    QList<int> getJudgingAreaID() const; //for marshal
    void addDelayedTrick(const Card *trick);
    void removeDelayedTrick(const Card *trick);
    bool containsTrick(const char *trick_name) const;

    virtual int getHandcardNum() const = 0;
    virtual void removeCard(const Card *card, Place place) = 0;
    virtual void addCard(const Card *card, Place place) = 0;
    virtual QList<const Card *> getHandcards() const = 0;

    WrappedCard *getWeapon() const;
    WrappedCard *getArmor() const;
    WrappedCard *getDefensiveHorse() const;
    WrappedCard *getOffensiveHorse() const;
    QList<const Card *> getEquips() const;
    const EquipCard *getEquip(int index) const;

    bool hasWeapon(const char *weapon_name) const;
    bool hasArmorEffect(const char *armor_name) const;

    bool isKongcheng() const;
    bool isNude() const;
    bool isAllNude() const;

    bool canDiscard(const Player *to, const char *flags) const;
    bool canDiscard(const Player *to, int card_id) const;

    void addMark(const char *mark, int add_num = 1);
    void removeMark(const char *mark, int remove_num = 1);
    virtual void setMark(const char *mark, int value);
    int getMark(const char *mark) const;

    void setChained(bool chained);
    bool isChained() const;

    bool canSlash(const Player *other, const Card *slash, bool distance_limit = true,
                  int rangefix = 0, const QList<const Player *> &others = QList<const Player *>()) const;
    bool canSlash(const Player *other, bool distance_limit = true,
                  int rangefix = 0, const QList<const Player *> &others = QList<const Player *>()) const;
    int getCardCount(bool include_equip) const;

    QList<int> getPile(const char *pile_name) const;
    QStringList getPileNames() const;
    QString getPileName(int card_id) const;

    bool pileOpen(const char *pile_name, const char *player) const;
    void setPileOpen(const char *pile_name, const char *player);

    void addHistory(const char *name, int times = 1);
    void clearHistory();
    bool hasUsed(const char *card_class) const;
    int usedTimes(const char *card_class) const;
    int getSlashCount() const;

    bool hasEquipSkill(const char *skill_name) const;
    QSet<const TriggerSkill *> getTriggerSkills() const;
    QSet<const Skill *> getSkills(bool include_equip = false, bool visible_only = true) const;
    QList<const Skill *> getSkillList(bool include_equip = false, bool visible_only = true) const;
    QList<const Skill *> getHeadSkillList(bool visible_only = true) const;
    QList<const Skill *> getDeputySkillList(bool visible_only = true) const;
    QSet<const Skill *> getVisibleSkills(bool include_equip = false) const;
    QList<const Skill *> getVisibleSkillList(bool include_equip = false) const;
    QString getSkillDescription(bool yellow = true) const;
    QString getHeadSkillDescription() const;
    QString getDeputySkillDescription() const;

    virtual bool isProhibited(const Player *to, const Card *card, const QList<const Player *> &others = QList<const Player *>()) const;
    bool canSlashWithoutCrossbow(const Card *slash = NULL) const;
    virtual bool isLastHandCard(const Card *card, bool contain = false) const = 0;

    bool isJilei(const Card *card) const;
    bool isLocked(const Card *card) const;

    void setCardLimitation(const QString &limit_list, const QString &pattern, bool single_turn = false);
    void removeCardLimitation(const QString &limit_list, const QString &pattern);
    void clearCardLimitation(bool single_turn = false);
    bool isCardLimited(const Card *card, Card::HandlingMethod method, bool isHandcard = false) const;

    // just for convenience
    void addQinggangTag(const Card *card);
    void removeQinggangTag(const Card *card);
    const Player *getLord() const; // a small function put here, simple but useful
    int getPlayerNumWithSameKingdom(const char *_to_calculate = NULL) const;

    void copyFrom(Player *p);

    QList<const Player *> getSiblings() const;
    QList<const Player *> getAliveSiblings() const;

    bool hasShownSkill(const Skill *skill) const;
    void preshowSkill(const char *skill_name);
    bool inHeadSkills(const char *skill_name) const;
    const General *getActualGeneral1() const;
    const General *getActualGeneral2() const;
    QString getActualGeneral1Name() const;
    QString getActualGeneral2Name() const;
    void setActualGeneral1(const General *general);
    void setActualGeneral2(const General *general);
    void setActualGeneral1Name(const char *name);
    void setActualGeneral2Name(const char *name);
    bool hasShownGeneral1() const;
    bool hasShownGeneral2() const;
    void setGeneral1Showed(bool showed);
    void setGeneral2Showed(bool showed);
    bool hasShownOneGeneral() const;
    bool hasShownAllGenerals() const;
    void setSkillPreshowed(const char *skill, const bool preshowed = true);
    void setSkillsPreshowed(const char *falgs = "hd", const bool preshowed = true);
    bool hasPreshowedSkill(const char *name) const;

    bool ownSkill(const char *skill_name) const;
    bool isFriendWith(const Player *player) const;
    bool willBeFriendWith(const Player *player) const;

    void setNext(Player *next);
    void setNext(const char *next);
    Player *getNext() const;
    QString getNextName() const;
    Player *getLast() const;
    Player *getNextAlive(int n = 1) const;
    Player *getLastAlive(int n = 1) const;

};

%extend Player {
    void setTag(const char *key, QVariant &value) {
        $self->tag[key] = value;
    }

    QVariant getTag(const char *key) {
        return $self->tag[key];
    }

    void removeTag(const char *tag_name) {
        $self->tag.remove(tag_name);
    }
};

class ServerPlayer: public Player {
public:
    ServerPlayer(Room *room);

    void invoke(const char *method, const char *arg = ".");
    void drawCard(const Card *card);
    Room *getRoom() const;
    void broadcastSkillInvoke(const Card *card) const;
    void broadcastSkillInvoke(const char *card_name) const;
    int getRandomHandCardId() const;
    const Card *getRandomHandCard() const;
    void obtainCard(const Card *card, bool unhide = true);
    void throwAllEquips();
    void throwAllHandCards();
    void throwAllHandCardsAndEquips();
    void throwAllCards();
    void bury();
    void throwAllMarks(bool visible_only = true);
    void clearOnePrivatePile(const char *pile_name);
    void clearPrivatePiles();
    void drawCards(int n, const char *reason = NULL);
    bool askForSkillInvoke(const char *skill_name, const QVariant &data = QVariant());
    QList<int> forceToDiscard(int discard_num, bool include_equip, bool is_discard = true);
    QList<int> handCards() const;
    virtual QList<const Card *> getHandcards() const;
    QList<const Card *> getCards(const char *flags) const;
    DummyCard *wholeHandCards() const;
    bool hasNullification() const;
    bool pindian(ServerPlayer *target, const char *reason, const Card *card1 = NULL);
    void turnOver();
    void play(QList<Player::Phase> set_phases = QList<Player::Phase>());
    bool changePhase(Player::Phase from, Player::Phase to);

    QList<Player::Phase> &getPhases();
    void skip(Player::Phase phase, bool sendLog = true);
    void skip(bool sendLog = true);
    void insertPhase(Player::Phase phase);
    bool isSkipped(Player::Phase phase);

    void gainMark(const char *mark, int n = 1);
    void loseMark(const char *mark, int n = 1);
    void loseAllMarks(const char *mark_name);

    virtual void addSkill(const char *skill_name, bool head_skill = true);
    virtual void loseSkill(const char *skill_name);
    virtual void setGender(General::Gender gender);

    void setAI(AI *ai);
    AI *getAI() const;
    AI *getSmartAI() const;

    virtual int aliveCount() const;
    virtual int getHandcardNum() const;
    virtual void removeCard(const Card *card, Place place);
    virtual void addCard(const Card *card, Place place);
    virtual bool isLastHandCard(const Card *card, bool contain = false) const;

    void addVictim(ServerPlayer *victim);
    QList<ServerPlayer *> getVictims() const;

    void setNext(ServerPlayer *next);
    ServerPlayer *getNext() const;
    ServerPlayer *getNextAlive(int n = 1) const;

    // 3v3 methods
    void addToSelected(const char *general);
    QStringList getSelected() const;
    void clearSelected();

    int getGeneralMaxHp() const;
    virtual QString getGameMode() const;

    void addToPile(const char *pile_name, const Card *card, bool open = true);
    void addToPile(const char *pile_name, int card_id, bool open = true);
    void addToPile(const char *pile_name, QList<int> card_ids, bool open = true);
    void addToPile(const char *pile_name, QList<int> card_ids, bool open, CardMoveReason reason);
    void exchangeFreelyFromPrivatePile(const char *skill_name, const char *pile_name, int upperlimit = 1000, bool include_equip = false);
    void gainAnExtraTurn();

    void copyFrom(ServerPlayer *sp);

    // static function
    static bool CompareByActionOrder(ServerPlayer *a, ServerPlayer *b);

    void showGeneral(bool head_general = true);
    void hideGeneral(bool head_general = true);
    void removeGeneral(bool head_general = true);
    void sendSkillsToOthers(bool head_skill = true);
    void disconnectSkillsFromOthers(bool head_skill = true);
    bool askForGeneralShow(bool one = true);
    void notifyPreshow();

    bool inSiegeRelation(const ServerPlayer *skill_owner, const ServerPlayer *victim) const;
    QList<ServerPlayer *> getFormation() const;
    bool inFormationRalation(ServerPlayer *teammate) const;
    void summonFriends(const BattleArrayType::ArrayType type);
};

%extend ServerPlayer {
    void speak(const char *msg) {
        QString str = QByteArray(msg).toBase64();
        $self->getRoom()->speakCommand($self, str);
    }
};

class ClientPlayer: public Player {
public:
    explicit ClientPlayer(Client *client);
    virtual int aliveCount() const;
    virtual int getHandcardNum() const;
    virtual QList<const Card *> getHandcards() const;
    virtual void removeCard(const Card *card, Place place);
    virtual void addCard(const Card *card, Place place);
    virtual void addKnownHandCard(const Card *card);
    virtual bool isLastHandCard(const Card *card, bool contain = false) const;
};

extern ClientPlayer *Self;

class CardMoveReason {
public:
    int m_reason;
    QString m_playerId; // the cause (not the source) of the movement, such as "lusu" when "dimeng", or "zhanghe" when "qiaobian"
    QString m_targetId; // To keep this structure lightweight, currently this is only used for UI purpose.
                        // It will be set to empty if multiple targets are involved. NEVER use it for trigger condition
                        // judgement!!! It will not accurately reflect the real reason.
    QString m_skillName; // skill that triggers movement of the cards, such as "longdang", "dimeng"
    QString m_eventName; // additional arg such as "lebusishu" on top of "S_REASON_JUDGE"

    CardMoveReason();
    CardMoveReason(int moveReason, char *playerId);
    CardMoveReason(int moveReason, char *playerId, char *skillName, char *eventName);
    CardMoveReason(int moveReason, char *playerId, char *targetId, char *skillName, char *eventName);

    static const int S_REASON_UNKNOWN = 0x00;
    static const int S_REASON_USE = 0x01;
    static const int S_REASON_RESPONSE = 0x02;
    static const int S_REASON_DISCARD = 0x03;
    static const int S_REASON_RECAST = 0x04;          // ironchain etc.
    static const int S_REASON_PINDIAN = 0x05;
    static const int S_REASON_DRAW = 0x06;
    static const int S_REASON_GOTCARD = 0x07;
    static const int S_REASON_SHOW = 0x08;
    static const int S_REASON_TRANSFER = 0x09;
    static const int S_REASON_PUT = 0x0A;

    //subcategory of use
    static const int S_REASON_LETUSE = 0x11;           // use a card when self is not current

    //subcategory of response
    static const int S_REASON_RETRIAL = 0x12;

    //subcategory of discard
    static const int S_REASON_RULEDISCARD = 0x13;       //  discard at one's Player::Discard for gamerule
    static const int S_REASON_THROW = 0x23;             /*  gamerule(dying or punish)
                                                            as the cost of some skills   */
    static const int S_REASON_DISMANTLE = 0x33;         //  one throw card of another

    //subcategory of gotcard
    static const int S_REASON_GIVE = 0x17;              // from one hand to another hand
    static const int S_REASON_EXTRACTION = 0x27;        // from another's place to one's hand
    static const int S_REASON_GOTBACK = 0x37;           // from placetable to hand
    static const int S_REASON_RECYCLE = 0x47;           // from discardpile to hand
    static const int S_REASON_ROB = 0x57;               // got a definite card from other's hand
    static const int S_REASON_PREVIEWGIVE = 0x67;       // give cards after previewing, i.e. Yiji & Miji

    //subcategory of show
    static const int S_REASON_TURNOVER = 0x18;          // show n cards  from drawpile
    static const int S_REASON_JUDGE = 0x28;             // show a card  from drawpile for judge
    static const int S_REASON_PREVIEW = 0x38;           // Not done yet, plan for view some cards for self only(guanxing yiji miji)
    static const int S_REASON_DEMONSTRATE = 0x48;       // show a card which copy one to move to table

    //subcategory of transfer
    static const int S_REASON_SWAP = 0x19;              // exchange card for two players
    static const int S_REASON_OVERRIDE = 0x29;          // exchange cards from cards in game
    static const int S_REASON_EXCHANGE_FROM_PILE = 0x39;// exchange cards from cards moved out of game (for qixing only)

    //subcategory of put
    static const int S_REASON_NATURAL_ENTER = 0x1A;     //  a card with no-owner move into discardpile
    static const int S_REASON_REMOVE_FROM_PILE = 0x2A;  //  cards moved out of game go back into discardpile
    static const int S_REASON_JUDGEDONE = 0x3A;         //  judge card move into discardpile
    static const int S_REASON_CHANGE_EQUIP = 0x4A;      //  replace existed equip


    static const int S_MASK_BASIC_REASON = 0x0F;
};

struct DamageStruct {
    DamageStruct();
    DamageStruct(const Card *card, ServerPlayer *from, ServerPlayer *to, int damage = 1, DamageStruct::Nature nature = Normal);
    DamageStruct(const char *reason, ServerPlayer *from, ServerPlayer *to, int damage = 1, DamageStruct::Nature nature = Normal);

    enum Nature {
        Normal, // normal slash, duel and most damage caused by skill
        Fire,  // fire slash, fire attack and few damage skill (Yeyan, etc)
        Thunder // lightning, thunder slash, and few damage skill (Leiji, etc)
    };

    ServerPlayer *from;
    ServerPlayer *to;
    const Card *card;
    int damage;
    Nature nature;
    bool chain;
    bool transfer;
    bool by_user;
    QString reason;

    QString getReason() const;
};

struct CardEffectStruct {
    CardEffectStruct();

    const Card *card;

    ServerPlayer *from;
    ServerPlayer *to;
};

struct SlashEffectStruct {
    SlashEffectStruct();

    int jink_num;

    const Card *slash;
    const Card *jink;

    ServerPlayer *from;
    ServerPlayer *to;

    int drank;

    DamageStruct::Nature nature;
};

struct CardUseStruct {
    enum CardUseReason {
        CARD_USE_REASON_UNKNOWN = 0x00,
        CARD_USE_REASON_PLAY = 0x01,
        CARD_USE_REASON_RESPONSE = 0x02,
        CARD_USE_REASON_RESPONSE_USE = 0x12
    } m_reason;

    CardUseStruct();
    CardUseStruct(const Card *card, ServerPlayer *from, QList<ServerPlayer *> to, bool isOwnerUse = true);
    CardUseStruct(const Card *card, ServerPlayer *from, ServerPlayer *target, bool isOwnerUse = true);
    bool isValid(const char *pattern) const;
    void parse(const char *str, Room *room);
    bool tryParse(const Json::Value &, Room *room);

    const Card *card;
    ServerPlayer *from;
    QList<ServerPlayer *> to;
    bool m_isOwnerUse;
    bool m_addHistory;
};

struct CardsMoveStruct {
    CardsMoveStruct();
    CardsMoveStruct(const QList<int> &ids, Player *from, Player *to, Player::Place from_place, Player::Place to_place, CardMoveReason reason);
    CardsMoveStruct(const QList<int> &ids, Player *to, Player::Place to_place, CardMoveReason reason);
    CardsMoveStruct(int id, Player *from, Player *to, Player::Place from_place, Player::Place to_place, CardMoveReason reason);
    CardsMoveStruct(int id, Player *to, Player::Place to_place, CardMoveReason reason);

    QList<int> card_ids;
    Player::Place from_place, to_place;
    QString from_player_name, to_player_name;
    QString from_pile_name, to_pile_name;
    Player *from, *to;
    CardMoveReason reason;
    bool open;
    bool is_last_handcard;
};

struct CardsMoveOneTimeStruct {
    QList<int> card_ids;
    QList<Player::Place> from_places;
    Player::Place to_place;
    CardMoveReason reason;
    Player *from, *to;
    QStringList from_pile_names;
    QString to_pile_name;

    QList<Player::Place> origin_from_places;
    Player::Place origin_to_place;
    Player *origin_from, *origin_to;
    QStringList origin_from_pile_names;
    QString origin_to_pile_name; //for case of the movement transitted

    QList<bool> open; // helper to prevent sending card_id to unrelevant clients
    bool is_last_handcard;
};

struct DyingStruct {
    DyingStruct();

    ServerPlayer *who; // who is ask for help
    DamageStruct *damage; // if it is NULL that means the dying is caused by losing hp
};

struct DeathStruct {
    DeathStruct();

    ServerPlayer *who; // who is ask for help
    DamageStruct *damage; // if it is NULL that means the dying is caused by losing hp
};

struct RecoverStruct {
    RecoverStruct();

    int recover;
    ServerPlayer *who;
    const Card *card;
};

struct JudgeStruct {
    JudgeStruct();
    bool isGood() const;
    bool isBad() const;
    bool isEffected() const;
    void updateResult();

    bool isGood(const Card *card) const; // For AI

    bool negative;
    bool play_animation;
    ServerPlayer *who;
    const Card *card;
    QString pattern;
    bool good;
    QString reason;
    bool time_consuming;
};

typedef JudgeStruct *JudgeStar;

struct PindianStruct {
    PindianStruct();

    ServerPlayer *from;
    ServerPlayer *to;
    const Card *from_card;
    const Card *to_card;
    int from_number;
    int to_number;
    QString reason;
    bool success;
};

typedef PindianStruct *PindianStar;

struct PhaseChangeStruct {
    PhaseChangeStruct();

    Player::Phase from;
    Player::Phase to;
};

struct CardResponseStruct {
    CardResponseStruct();

    const Card *m_card;
    ServerPlayer *m_who;
    bool m_isUse;
};

enum TriggerEvent {
    NonTrigger,

    GameStart,
    TurnStart,
    EventPhaseStart,
    EventPhaseProceeding,
    EventPhaseEnd,
    EventPhaseChanging,
    EventPhaseSkipping,

    DrawNCards,
    AfterDrawNCards,

    PreHpRecover,
    HpRecover,
    PreHpLost,
    HpChanged,
    MaxHpChanged,
    PostHpReduced,

    EventLoseSkill,
    EventAcquireSkill,

    StartJudge,
    AskForRetrial,
    FinishRetrial,
    FinishJudge,

    PindianVerifying,
    Pindian,

    TurnedOver,
    ChainStateChanged,

    ConfirmDamage,    // confirm the damage's count and damage's nature
    Predamage,        // trigger the certain skill -- jueqing
    DamageForseen,    // the first event in a damage -- kuangfeng dawu
    DamageCaused,     // the moment for -- qianxi..
    DamageInflicted,  // the moment for -- tianxiang..
    PreDamageDone,    // before reducing Hp
    DamageDone,       // it's time to do the damage
    Damage,           // the moment for -- lieren..
    Damaged,          // the moment for -- yiji..
    DamageComplete,   // the moment for trigger iron chain

    Dying,
    QuitDying,
    AskForPeaches,
    AskForPeachesDone,
    Death,
    BuryVictim,
    BeforeGameOverJudge,
    GameOverJudge,
    GameFinished,

    SlashEffected,
    SlashProceed,
    SlashHit,
    SlashMissed,

    JinkEffect,

    CardAsked,
    CardResponded,
    BeforeCardsMove, // sometimes we need to record cards before the move
    CardsMoveOneTime,

    PreCardUsed, // for AI to filter events only.
    CardUsed,
    TargetConfirming,
    TargetConfirmed,
    CardEffect, // for AI to filter events only
    CardEffected,
    PostCardEffected,
    CardFinished,
    TrickCardCanceling,

    ChoiceMade,

    StageChange, // For hulao pass only
    FetchDrawPileCard, // For miniscenarios only

    TurnBroken, // For the skill 'DanShou'. Do not use it to trigger events

    GeneralShown, // For Official Hegemony mode
    GeneralHidden, // For Official Hegemony mode
    GeneralRemoved, // For Official Hegemony mode

    NumOfEvents
};

class Card: public QObject {
public:
    // enumeration type
    enum Suit { Spade, Club, Heart, Diamond, NoSuitBlack, NoSuitRed, NoSuit, SuitToBeDecided };
    enum Color { Red, Black, Colorless };
    enum HandlingMethod { MethodNone, MethodUse, MethodResponse, MethodDiscard, MethodRecast, MethodPindian };

    static const Suit AllSuits[4];

    // card types
    enum CardType { TypeSkill, TypeBasic, TypeTrick, TypeEquip };

    // constructor
    Card(Suit suit, int number, bool target_fixed = false);

    // property getters/setters
    QString getSuitString() const;
    bool isRed() const;
    bool isBlack() const;
    int getId() const;
    virtual void setId(int id);
    int getEffectiveId() const;

    int getNumber() const;
    virtual void setNumber(int number);
    QString getNumberString() const;

    Suit getSuit() const;
    virtual void setSuit(Suit suit);

    bool sameColorWith(const Card *other) const;
    Color getColor() const;
    QString getFullName(bool include_suit = false) const;
    QString getLogName() const;
    QString getName() const;
    QString getSkillName(bool removePrefix = true) const;
    virtual void setSkillName(const char *skill_name);
    QString getDescription(bool yellow = true) const;

    virtual bool isMute() const;
    virtual bool willThrow() const;
    virtual bool canRecast() const;
    virtual bool hasPreAction() const;
    virtual Card::HandlingMethod getHandlingMethod() const;

    virtual void setFlags(const char *flag) const;
    bool hasFlag(const char *flag) const;
    virtual void clearFlags() const;

    virtual QString getPackage() const;
    virtual QString getClassName() const;
    virtual bool isVirtualCard() const;
    virtual bool isEquipped() const;
    virtual QString getCommonEffectName() const;
    virtual bool match(const char *pattern) const;

    virtual void addSubcard(int card_id);
    virtual void addSubcard(const Card *card);
    virtual QList<int> getSubcards() const;
    virtual void clearSubcards();
    virtual QString subcardString() const;
    virtual void addSubcards(const QList<const Card *> &cards);
    virtual void addSubcards(const QList<int> &subcards_list);
    virtual int subcardsLength() const;

    virtual QString getType() const = 0;
    virtual QString getSubtype() const = 0;
    virtual CardType getTypeId() const = 0;
    virtual bool isNDTrick() const;

    // card target selection
    virtual bool targetFixed() const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    // @todo: the following two functions should be merged into one.
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self,
                              int &maxVotes) const;
    virtual bool isAvailable(const Player *player) const;

    virtual const Card *getRealCard() const;
    virtual const Card *validate(CardUseStruct &cardUse) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;

    virtual void doPreAction(Room *room, const CardUseStruct &card_use) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;

    virtual QString showSkill() const;
    virtual void setShowSkill(const char *skill_name);

    virtual bool isKindOf(const char *cardType) const;
    virtual QStringList getFlags() const;

    virtual bool isModified() const;
    virtual void onNullified(ServerPlayer *) const;

    // static functions
    static bool CompareByNumber(const Card *a, const Card *b);
    static bool CompareBySuit(const Card *a, const Card *b);
    static bool CompareByType(const Card *a, const Card *b);
    static Card *Clone(const Card *card);
    static QString Suit2String(Suit suit);
    static const int S_UNKNOWN_CARD_ID;

    static const Card *Parse(const char *str);
    virtual QString toString(bool hidden = false) const;

    virtual QString getEffectName() const;

};

%extend Card {
    EquipCard *toEquipCard() {
        return qobject_cast<EquipCard *>($self);
    }

    Weapon *toWeapon() {
        return qobject_cast<Weapon *>($self);
    }

    WrappedCard *toWrapped() {
        return qobject_cast<WrappedCard *>($self);
    }

    TrickCard *toTrick() {
        return qobject_cast<TrickCard *>($self);
    }

    void cardOnUse(Room *room, const CardUseStruct &card_use) const{
         $self->Card::onUse(room, card_use);
    }

    bool cardIsAvailable(const Player *player) const{
        return $self->Card::isAvailable(player);
    }
};

class WrappedCard: public Card {
public:
    void takeOver(Card *card);
    void copyEverythingFrom(Card *card);
    void setModified(bool modified);
};

class SkillCard: public Card {
public:
    SkillCard();
    void setUserString(const char *user_string);
    QString getUserString() const;

    virtual QString getSubtype() const;
    virtual QString getType() const;
    virtual CardType getTypeId() const;
    virtual QString toString(bool hidden = false) const;

protected:
    QString user_string;
};

class DummyCard: public SkillCard {
public:
    DummyCard();
    DummyCard(const QList<int> &subcards);

    virtual QString getSubtype() const;
    virtual QString getType() const;
    virtual QString toString(bool hidden = false) const;
};

class ArraySummonCard: public SkillCard {
public:
    ArraySummonCard(const char *name);
};

class Package: public QObject {
public:
    enum Type { GeneralPack, CardPack, MixedPack, SpecialPack };

    Package(const char *name, Type pack_type = GeneralPack);
    void insertRelatedSkills(const char *main_skill, const char *related_skill);
};

class Engine: public QObject {
public:
    void addTranslationEntry(const char *key, const char *value);
    QString translate(const char *to_translate) const;

    void addPackage(Package *package);
    void addBanPackage(const char *package_name);
    QStringList getBanPackages() const;
    Card *cloneCard(const Card *card) const;
    Card *cloneCard(const char *name, Card::Suit suit = Card::SuitToBeDecided, int number = -1) const;
    SkillCard *cloneSkillCard(const char *name) const;
    QString getVersionNumber() const;
    QString getVersion() const;
    QString getVersionName() const;
    QString getMODName() const;
    QStringList getExtensions() const;
    QStringList getKingdoms() const;

    const CardPattern *getPattern(const char *name) const;
    bool matchExpPattern(const char *pattern, const Player *player, const Card *card) const;
    Card::HandlingMethod getCardHandlingMethod(const char *method_name) const;
    QList<const Skill *> getRelatedSkills(const char *skill_name) const;
    const Skill *getMainSkill(const char *skill_name) const;

    void addPackage(const char *name);

    const General *getGeneral(const char *name) const;
    int getGeneralCount(bool include_banned = false) const;
    const Skill *getSkill(const char *skill_name) const;
    const Skill *getSkill(const EquipCard *card) const;
    QStringList getSkillNames() const;
    const TriggerSkill *getTriggerSkill(const char *skill_name) const;
    const ViewAsSkill *getViewAsSkill(const char *skill_name) const;
    QList<const DistanceSkill *> getDistanceSkills() const;
    QList<const MaxCardsSkill *> getMaxCardsSkills() const;
    QList<const TargetModSkill *> getTargetModSkills() const;
    QList<const TriggerSkill *> getGlobalTriggerSkills() const;
    void addSkills(const QList<const Skill *> &skills);

    int getCardCount() const;
    const Card *getEngineCard(int cardId) const;
    // @todo: consider making this const Card *
    Card *getCard(int cardId);
    WrappedCard *getWrappedCard(int cardId);

    QStringList getLords(bool contain_banned = false) const;
    QStringList getRandomLords() const;
    void banRandomGods() const;
    QStringList getRandomGenerals(int count) const;
    QList<int> getRandomCards() const;
    QString getRandomGeneralName() const;
    QStringList getLimitedGeneralNames() const;

    void playSystemAudioEffect(const char *name) const;
    void playAudioEffect(const char *filename) const;
    void playSkillAudioEffect(const char *skill_name, int index) const;

    const ProhibitSkill *isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others = QList<const Player *>()) const;
    int correctDistance(const Player *from, const Player *to) const;
    int correctMaxCards(const Player *target, bool fixed = false) const;
    int correctCardTarget(const TargetModSkill::ModType type, const Player *from, const Card *card) const;

    Room *currentRoom();

    QString getCurrentCardUsePattern();
    CardUseStruct::CardUseReason getCurrentCardUseReason();

    bool isGeneralHidden(const char *general_name) const;
};

extern Engine *Sanguosha;

class Skill: public QObject {
public:
    enum Frequency { Frequent, NotFrequent, Compulsory, Limited, Wake };
    enum Location { Left, Right };

    explicit Skill(const char *name, Frequency frequent = NotFrequent);

    bool isLordSkill() const;
    bool isAttachedLordSkill() const;
    QString getDescription(bool yellow = true) const;
    bool isVisible() const;

    virtual QString getDefaultChoice(ServerPlayer *player) const;
    virtual int getEffectIndex(const ServerPlayer *player, const Card *card) const;

    virtual Location getLocation() const;

    void playAudioEffect(int index = -1) const;
    Frequency getFrequency() const;
    QString getLimitMark() const;

    virtual bool canPreshow() const;
    virtual bool relateToPlace(bool head = true) const;

    void setRelateToPlace(const char *rtp);
};

%extend Skill {
    const TriggerSkill *toTriggerSkill() const{
        return qobject_cast<const TriggerSkill *>($self);
    }
};

class TriggerSkill: public Skill {
public:
    TriggerSkill(const char *name);
    const ViewAsSkill *getViewAsSkill() const;
    QList<TriggerEvent> getTriggerEvents() const;

    virtual int getPriority() const;
    virtual QStringList triggerable(const ServerPlayer *target) const;

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who) const;
    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;
    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;

    bool isGlobal() const;
};

%extend TriggerSkill {
    const BattleArraySkill *toBattleArraySkill() const{
        return qobject_cast<const BattleArraySkill *>($self);
    }

    QStringList TriggerSkillTriggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &ask_who) const{
        return $self->TriggerSkill::triggerable(triggerEvent, room, player, data, ask_who);
    }
};

class QThread: public QObject {
};

struct LogMessage {
    LogMessage();
    QString toString() const;

    QString type;
    ServerPlayer *from;
    QList<ServerPlayer *> to;
    QString card_str;
    QString arg;
    QString arg2;
};

class RoomThread: public QThread {
public:
    explicit RoomThread(Room *room);
    void constructTriggerTable();
    bool trigger(TriggerEvent event, Room *room, ServerPlayer *target, QVariant &data);
    bool trigger(TriggerEvent event, Room *room, ServerPlayer *target);

    void addPlayerSkills(ServerPlayer *player, bool invoke_game_start = false);

    void addTriggerSkill(const TriggerSkill *skill);
    void delay(unsigned long msecs = 1000);
};

class Room: public QThread {
public:
    enum GuanxingType { GuanxingUpOnly = 1, GuanxingBothSides = 0, GuanxingDownOnly = -1 };
    explicit Room(QObject *parent, const char *mode);
    bool isFull() const;
    bool isFinished() const;
    int getLack() const;
    QString getMode() const;
    const Scenario *getScenario() const;
    RoomThread *getThread() const;
    ServerPlayer *getCurrent() const;
    void setCurrent(ServerPlayer *current);
    int alivePlayerCount() const;
    QList<ServerPlayer *> getOtherPlayers(ServerPlayer *except, bool include_dead = false) const;
    QList<ServerPlayer *> getPlayers() const;
    QList<ServerPlayer *> getAllPlayers(bool include_dead = false) const;
    QList<ServerPlayer *> getAlivePlayers() const;
    void output(const char *message);
    void outputEventStack();
    void enterDying(ServerPlayer *player, DamageStruct *reason);
    ServerPlayer *getCurrentDyingPlayer() const;
    void killPlayer(ServerPlayer *victim, DamageStruct *reason = NULL);
    void revivePlayer(ServerPlayer *player);
    QStringList aliveRoles(ServerPlayer *except = NULL) const;
    void gameOver(const char *winner);
    void slashEffect(const SlashEffectStruct &effect);
    void slashResult(const SlashEffectStruct &effect, const Card *jink);
    void attachSkillToPlayer(ServerPlayer *player, const char *skill_name);
    void detachSkillFromPlayer(ServerPlayer *player, const char *skill_name, bool is_equip = false, bool acquire_only = false);
    void handleAcquireDetachSkills(ServerPlayer *player, const char *skill_names, bool acquire_only = false);
    void setPlayerFlag(ServerPlayer *player, const char *flag);
    void setPlayerProperty(ServerPlayer *player, const char *property_name, const QVariant &value);
    void setPlayerMark(ServerPlayer *player, const char *mark, int value);
    void addPlayerMark(ServerPlayer *player, const char *mark, int add_num = 1);
    void removePlayerMark(ServerPlayer *player, const char *mark, int remove_num = 1);
    void setPlayerCardLimitation(ServerPlayer *player, const char *limit_list,
                                 const char *pattern, bool single_turn);
    void removePlayerCardLimitation(ServerPlayer *player, const char *limit_list,
                                    const char *pattern);
    void clearPlayerCardLimitation(ServerPlayer *player, bool single_turn);
    void setCardFlag(const Card *card, const char *flag, ServerPlayer *who = NULL);
    void setCardFlag(int card_id, const char *flag, ServerPlayer *who = NULL);
    void clearCardFlag(const Card *card, ServerPlayer *who = NULL);
    void clearCardFlag(int card_id, ServerPlayer *who = NULL);
    bool useCard(const CardUseStruct &card_use, bool add_history = true);
    void damage(const DamageStruct &data);
    void sendDamageLog(const DamageStruct &data);
    void loseHp(ServerPlayer *victim, int lose = 1);
    void loseMaxHp(ServerPlayer *victim, int lose = 1);
    void applyDamage(ServerPlayer *victim, const DamageStruct &damage);
    void recover(ServerPlayer *player, const RecoverStruct &recover, bool set_emotion = false);
    bool cardEffect(const Card *card, ServerPlayer *from, ServerPlayer *to);
    bool cardEffect(const CardEffectStruct &effect);
    bool isJinkEffected(ServerPlayer *user, const Card *jink);
    void judge(JudgeStruct &judge_struct);
    void sendJudgeResult(const JudgeStar judge);
    QList<int> getNCards(int n, bool update_pile_number = true);
    ServerPlayer *getLord(const char *kingdom = NULL) const;
    void askForGuanxing(ServerPlayer *zhuge, const QList<int> &cards, GuanxingType guanxing_type = GuanxingBothSides);
    int doGongxin(ServerPlayer *shenlvmeng, ServerPlayer *target, QList<int> enabled_ids = QList<int>(), const char *skill_name = "gongxin");
    int drawCard();
    void fillAG(const QList<int> &card_ids, ServerPlayer *who = NULL, const QList<int> &disabled_ids = QList<int>());
    void takeAG(ServerPlayer *player, int card_id, bool move_cards = true);
    void clearAG(ServerPlayer *player = NULL);
    void provide(const Card *card);
    QList<ServerPlayer *> getLieges(const char *kingdom, ServerPlayer *lord) const;
    void sendLog(const LogMessage &log);
    void showCard(ServerPlayer *player, int card_id, ServerPlayer *only_viewer = NULL);
    void showAllCards(ServerPlayer *player, ServerPlayer *to = NULL);
    void retrial(const Card *card, ServerPlayer *player, JudgeStar judge,
                 const char *skill_name, bool exchange = false);

    bool doNotify(ServerPlayer *player, int command, const char *arg);
    bool doBroadcastNotify(int command, const char *arg);
    bool doBroadcastNotify(const QList<ServerPlayer *> &players, int command, const char *arg);

    bool notifyMoveCards(bool isLostPhase, QList<CardsMoveStruct> move, bool forceVisible, QList<ServerPlayer *> players = QList<ServerPlayer *>());
    bool notifyProperty(ServerPlayer *playerToNotify, const ServerPlayer *propertyOwner, const char *propertyName, const char *value = NULL);
    bool notifyUpdateCard(ServerPlayer *player, int cardId, const Card *newCard);
    bool broadcastUpdateCard(const QList<ServerPlayer *> &players, int cardId, const Card *newCard);
    bool notifyResetCard(ServerPlayer *player, int cardId);
    bool broadcastResetCard(const QList<ServerPlayer *> &players, int cardId);

    bool broadcastProperty(ServerPlayer *player, const char *property_name, const char *value = NULL);
    void notifySkillInvoked(ServerPlayer *player, const char *skill_name);
    void broadcastSkillInvoke(const char *skillName);
    void broadcastSkillInvoke(const char *skillName, const char *category);
    void broadcastSkillInvoke(const char *skillName, int type);
    void broadcastSkillInvoke(const char *skillName, bool isMale, int type);
    void doLightbox(const char *lightboxName, int duration = 2000);
    void doAnimate(int type, const char *arg1 = NULL, const char *arg2 = NULL, QList<ServerPlayer *> players = QList<ServerPlayer *>());

    void preparePlayers();
    void changePlayerGeneral(ServerPlayer *player, const char *new_general);
    void changePlayerGeneral2(ServerPlayer *player, const char *new_general);
    void filterCards(ServerPlayer *player, QList<const Card *> cards, bool refilter);

    void acquireSkill(ServerPlayer *player, const Skill *skill, bool open = true);
    void acquireSkill(ServerPlayer *player, const char *skill_name, bool open = true);
    void adjustSeats();
    void swapPile();
    QList<int> getDiscardPile();
    QList<int> &getDrawPile();
    const QList<int> &getDrawPile() const;
    int getCardFromPile(const char *card_name);
    ServerPlayer *findPlayer(const char *general_name, bool include_dead = false) const;
    QList<ServerPlayer *> findPlayersBySkillName(const char *skill_name) const;
    ServerPlayer *findPlayerBySkillName(const char *skill_name) const;
    void installEquip(ServerPlayer *player, const char *equip_name);
    void resetAI(ServerPlayer *player);
    void changeHero(ServerPlayer *player, const char *new_general, bool full_state, bool invoke_start = true,
                    bool isSecondaryHero = false, bool sendLog = true);
    void swapSeat(ServerPlayer *a, ServerPlayer *b);
    void setFixedDistance(Player *from, const Player *to, int distance);
    bool hasWelfare(const ServerPlayer *player) const;
    ServerPlayer *getFront(ServerPlayer *a, ServerPlayer *b) const;
    void signup(ServerPlayer *player, const char *screen_name, const char *avatar, bool is_robot);
    ServerPlayer *getOwner() const;
    void updateStateItem();

    void reconnect(ServerPlayer *player, ClientSocket *socket);
    void marshal(ServerPlayer *player);

    void sortByActionOrder(QList<ServerPlayer *> &players);

    const ProhibitSkill *isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others = QList<const Player *>()) const;

    void setTag(const char *key, const QVariant &value);
    QVariant getTag(const char *key) const;
    void removeTag(const char *key);

    void setEmotion(ServerPlayer *target, const char *emotion);

    Player::Place getCardPlace(int card_id) const;
    ServerPlayer *getCardOwner(int card_id) const;
    void setCardMapping(int card_id, ServerPlayer *owner, Player::Place place);

    void drawCards(ServerPlayer *player, int n, const char *reason = NULL);
    void drawCards(QList<ServerPlayer *> players, int n, const char *reason = NULL);
    void drawCards(QList<ServerPlayer *> players, QList<int> n_list, const char *reason = NULL);
    void obtainCard(ServerPlayer *target, const Card *card, bool unhide = true);
    void obtainCard(ServerPlayer *target, int card_id, bool unhide = true);
    void obtainCard(ServerPlayer *target, const Card *card,  const CardMoveReason &reason, bool unhide = true);

    void throwCard(int card_id, ServerPlayer *who, ServerPlayer *thrower = NULL);
    void throwCard(const Card *card, ServerPlayer *who, ServerPlayer *thrower = NULL);
    void throwCard(const Card *card, const CardMoveReason &reason, ServerPlayer *who, ServerPlayer *thrower = NULL);

    void moveCardTo(const Card *card, ServerPlayer *dstPlayer, Player::Place dstPlace, bool forceMoveVisible = false);
    void moveCardTo(const Card *card, ServerPlayer *dstPlayer, Player::Place dstPlace, const CardMoveReason &reason,
                    bool forceMoveVisible = false);
    void moveCardTo(const Card *card, ServerPlayer *srcPlayer, ServerPlayer *dstPlayer, Player::Place dstPlace,
                    const CardMoveReason &reason, bool forceMoveVisible = false);
    void moveCardTo(const Card *card, ServerPlayer *srcPlayer, ServerPlayer *dstPlayer, Player::Place dstPlace,
                    const char *pileName, const CardMoveReason &reason, bool forceMoveVisible = false);
    void moveCardsAtomic(QList<CardsMoveStruct> cards_move, bool forceMoveVisible);
    void moveCardsAtomic(CardsMoveStruct cards_move, bool forceMoveVisible);
    void moveCardsToEndOfDrawpile(QList<int> card_ids);
    void moveCards(CardsMoveStruct cards_move, bool forceMoveVisible, bool ignoreChanges = true);
    void moveCards(QList<CardsMoveStruct> cards_moves, bool forceMoveVisible, bool ignoreChanges = true);

    // interactive methods
    void activate(ServerPlayer *player, CardUseStruct &card_use);
    void askForLuckCard();
    Card::Suit askForSuit(ServerPlayer *player, const char *reason);
    QString askForKingdom(ServerPlayer *player);
    bool askForSkillInvoke(ServerPlayer *player, const char *skill_name, const QVariant &data = QVariant());
    QString askForChoice(ServerPlayer *player, const char *skill_name, const char *choices, const QVariant &data = QVariant());
    bool askForDiscard(ServerPlayer *target, const char *reason, int discard_num, int min_num,
                       bool optional = false, bool include_equip = false, const char *prompt = NULL);
    const Card *askForExchange(ServerPlayer *player, const char *reason, int discard_num, bool include_equip = false,
                               const char *prompt = NULL, bool optional = false);
    bool askForNullification(const Card *trick, ServerPlayer *from, ServerPlayer *to, bool positive);
    bool isCanceled(const CardEffectStruct &effect);
    int askForCardChosen(ServerPlayer *player, ServerPlayer *who, const char *flags, const char *reason,
                         bool handcard_visible = false, Card::HandlingMethod method = Card::MethodNone, const QList<int> &disabled_ids = QList<int>());
    const Card *askForCard(ServerPlayer *player, const char *pattern, const char *prompt, const QVariant &data, const char *skill_name);
    const Card *askForCard(ServerPlayer *player, const char *pattern, const char *prompt, const QVariant &data = QVariant(),
                           Card::HandlingMethod method = Card::MethodDiscard, ServerPlayer *to = NULL, bool isRetrial = false,
                           const char *skill_name = NULL, bool isProvision = false);
    const Card *askForUseCard(ServerPlayer *player, const char *pattern, const char *prompt, int notice_index = -1,
                       Card::HandlingMethod method = Card::MethodUse, bool addHistory = true);
    const Card *askForUseSlashTo(ServerPlayer *slasher, ServerPlayer *victim, const char *prompt,
                                    bool distance_limit = true, bool disable_extra = false, bool addHistory = false);
    const Card *askForUseSlashTo(ServerPlayer *slasher, QList<ServerPlayer *> victims, const char *prompt,
                                     bool distance_limit = true, bool disable_extra = false, bool addHistory = false);
    int askForAG(ServerPlayer *player, const QList<int> &card_ids, bool refusable, const char *reason);
    const Card *askForCardShow(ServerPlayer *player, ServerPlayer *requestor, const char *reason);
    bool askForYiji(ServerPlayer *guojia, QList<int> &cards, const char *skill_name = NULL,
                    bool is_preview = false, bool visible = false, bool optional = true, int max_num = -1,
                    QList<ServerPlayer *> players = QList<ServerPlayer *>(), CardMoveReason reason = CardMoveReason(),
                    const char *prompt = NULL, bool notify_skill = false);
    const Card *askForPindian(ServerPlayer *player, ServerPlayer *from, ServerPlayer *to, const char *reason);
    QList<const Card *> askForPindianRace(ServerPlayer *from, ServerPlayer *to, const char *reason);
    ServerPlayer *askForPlayerChosen(ServerPlayer *player, const QList<ServerPlayer *> &targets, const char *reason,
                                     const char *prompt = NULL, bool optional = false, bool notify_skill = false);
    QString askForGeneral(ServerPlayer *player, const char *generals, const char *default_choice = NULL, bool single_result = true);
    const Card *askForSinglePeach(ServerPlayer *player, ServerPlayer *dying);
    void addPlayerHistory(ServerPlayer *player, const char *key, int times = 1);

    void broadcastInvoke(const char *method, const char *arg = ".", ServerPlayer *except = NULL);
    RoomState *getRoomState();
    Card *getCard(int cardId) const;
    void resetCard(int cardId);
    void updateCardsOnLose(const CardsMoveStruct &move);
    void updateCardsOnGet(const CardsMoveStruct &move);
};

%extend Room {
    ServerPlayer *nextPlayer() const{
        return qobject_cast<ServerPlayer *>($self->getCurrent()->getNextAlive());
    }
    void output(const char *msg) {
        if(Config.value("DebugOutput", false).toBool())
            $self->output(msg);
    }
    void outputEventStack() {
        if(Config.value("DebugOutput", false).toBool())
            $self->outputEventStack();
    }
    void writeToConsole(const char *msg) {
        $self->output(msg);
        qWarning("%s", msg);
    }
    void throwEvent(const TriggerEvent event) {
        Q_UNUSED($self);
        throw event;
    }
};

%{

void Room::doScript(const QString &script) {
    SWIG_NewPointerObj(L, this, SWIGTYPE_p_Room, 0);
    lua_setglobal(L, "R");

    SWIG_NewPointerObj(L, current, SWIGTYPE_p_ServerPlayer, 0);
    lua_setglobal(L, "P");

    luaL_dostring(L, script.toAscii());
}

%}

%include "card.i"
%include "luaskills.i"
%include "ai.i"
