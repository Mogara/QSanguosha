/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

  This file is part of QSanguosha-Hegemony.

  This game is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 3.0
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  See the LICENSE file for more details.

  QSanguosha-Rara
*********************************************************************/

%module sgs

%{

#include "structs.h"
#include "engine.h"
#include "client.h"
#include "namespace.h"
#include "standard.h"
#include "roomthread.h"

#include <QDir>

%}

%include "naturalvar.i"
%include "native.i"
%include "qvariant.i"
%include "list.i"
%include "set.i"

// ----------------------------------------

namespace HegemonyMode {
    QString GetMappedRole(const char *kingdom);
    QString GetMappedKingdom(const char *role);

    enum ArrayType {
        Siege,
        Formation
    };
};

namespace MaxCardsType {
    enum MaxCardsCount {
        Max = 1,
        Normal = 0,
        Min = -1,
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
    explicit General(Package *package, const char *name, const char *kingdom, int double_max_hp = 4, bool male = true, bool hidden = false, bool never_shown = false);

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

    void addRelateSkill(const char *skill_name);
    QStringList getRelatedSkillNames() const;

    QString getPackage() const;
    QString getCompanions() const;
    QString getSkillDescription(bool include_name = false, bool inToolTip = true) const;

    void addCompanion(const char *name);
    bool isCompanionWith(const char *name) const;

    void setHeadMaxHpAdjustedValue(int adjusted_value = -1);
    void setDeputyMaxHpAdjustedValue(int adjusted_value = -1);

    void lastWord(const int skinId) const;

};

class Player: public QObject {
public:
    enum Phase { RoundStart, Start, Judge, Draw, Play, Discard, Finish, NotActive, PhaseNone };
    enum Place { PlaceHand, PlaceEquip, PlaceDelayedTrick, PlaceJudge, PlaceSpecial, DiscardPile, DrawPile, PlaceTable, PlaceUnknown, PlaceWuGu, DrawPileBottom };
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

    bool isOwner() const;
    void setOwner(bool owner);

    bool hasShownRole() const;
    void setShownRole(bool shown);

    void setDisableShow(const char *flags, const char *reason);
    void removeDisableShow(const char *reason);
    QStringList disableShow(bool head) const;

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

    QString getFootnoteName() const;

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

    virtual int aliveCount(bool includeRemoved = true) const = 0;
    void setFixedDistance(const Player *player, int distance);
    int originalRightDistanceTo(const Player *other) const;
    int distanceTo(const Player *other, int distance_fix = 0) const;
    const General *getAvatarGeneral() const;
    const General *getGeneral() const;

    bool isLord() const;

    void acquireSkill(const char *skill_name, bool head = true);
    void detachSkill(const char *skill_name);
    void detachAllSkills();
    virtual void addSkill(const char *skill_name, bool head_skill = true);
    virtual void loseSkill(const char *skill_name);
    bool hasSkill(const char *skill_name, bool include_lose = false) const;
    bool hasSkill(const Skill *skill, bool include_lose = false) const;
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
    WrappedCard *getTreasure() const;

    QList<const Card *> getEquips() const;
    const EquipCard *getEquip(int index) const;

    bool hasWeapon(const char *weapon_name) const;
    bool hasArmorEffect(const char *armor_name) const;
    bool hasTreasure(const char *treasure_name) const;

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
    bool canBeChainedBy(const Player *source = NULL) const;

    void setRemoved(bool removed);
    bool isRemoved() const;

    bool isDuanchang(const bool head = true) const;

    bool canSlash(const Player *other, const Card *slash, bool distance_limit = true, int rangefix = 0, const QList<const Player *> &others = QList<const Player *>()) const;
    bool canSlash(const Player *other, bool distance_limit = true, int rangefix = 0, const QList<const Player *> &others = QList<const Player *>()) const;
    int getCardCount(bool include_equip) const;

    QList<int> getPile(const char *pile_name) const;
    QStringList getPileNames() const;
    QString getPileName(int card_id) const;

    bool pileOpen(const char *pile_name, const char *player) const;
    void setPileOpen(const char *pile_name, const char *player);

    void addHistory(const char *name, int times = 1);
    void clearHistory(const char *name = "");
    bool hasUsed(const char *card_class) const;
    int usedTimes(const char *card_class) const;
    int getSlashCount() const;

    bool hasEquipSkill(const char *skill_name) const;
    QList<const Skill *> getSkillList(bool include_equip = false, bool visible_only = true) const;
    QList<const Skill *> getHeadSkillList(bool visible_only = true) const;
    QList<const Skill *> getDeputySkillList(bool visible_only = true) const;
    QList<const Skill *> getVisibleSkillList(bool include_equip = false) const;
    QString getSkillDescription(bool inToolTip = true) const;
    QString getHeadSkillDescription() const;
    QString getDeputySkillDescription() const;

    virtual bool isProhibited(const Player *to, const Card *card, const QList<const Player *> &others = QList<const Player *>()) const;
    bool canSlashWithoutCrossbow(const Card *slash = NULL) const;
    virtual bool isLastHandCard(const Card *card, bool contain = false) const = 0;

    bool isJilei(const Card *card) const;
    bool isLocked(const Card *card) const;

    void setCardLimitation(const char *limit_list, const char *pattern, bool single_turn = false);
    void removeCardLimitation(const char *limit_list, const char *pattern);
    void clearCardLimitation(bool single_turn = false);
    bool isCardLimited(const Card *card, Card::HandlingMethod method, bool isHandcard = false) const;

    // just for convenience
    void addQinggangTag(const Card *card);
    void removeQinggangTag(const Card *card);
    const Player *getLord(bool include_death = false) const; // a small function put here, simple but useful

    void copyFrom(Player *p);

    QList<const Player *> getSiblings() const;
    QList<const Player *> getAliveSiblings() const;

    bool hasShownSkill(const Skill *skill) const;
    bool hasShownSkill(const char *skill_name) const;
    bool hasShownSkills(const char *skill_names) const;
    void preshowSkill(const char *skill_name);
    bool inHeadSkills(const char *skill_name) const;
    bool inHeadSkills(const Skill *skill) const;
    bool inDeputySkills(const char *skill_name) const;
    bool inDeputySkills(const Skill *skill) const;
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
    void setSkillPreshowed(const char *skill, bool preshowed = true);
    void setSkillsPreshowed(const char *falgs = "hd", bool preshowed = true);
    bool hasPreshowedSkill(const char *name) const;
    bool hasPreshowedSkill(const Skill *skill) const;
    bool isHidden(const bool &head_general) const;

    bool ownSkill(const char *skill_name) const;
    bool ownSkill(const Skill *skill) const;
    bool isFriendWith(const Player *player) const;
    bool willBeFriendWith(const Player *player) const;

    void setNext(Player *next);
    void setNext(const char *next);
    Player *getNext(bool ignoreRemoved = true) const;
    QString getNextName() const;
    Player *getLast(bool ignoreRemoved = true) const;
    Player *getNextAlive(int n = 1, bool ignoreRemoved = true) const;
    Player *getLastAlive(int n = 1, bool ignoreRemoved = true) const;

    QList<const Player *> getFormation() const;

    virtual QStringList getBigKingdoms(const char *reason, MaxCardsType::MaxCardsCount type = MaxCardsType::Min) const = 0;
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
    explicit ServerPlayer(Room *room);

    QString objectName() const;
    void kick();
    void unicast(const char *message);
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
    int getMaxCards(MaxCardsType::MaxCardsCount type = MaxCardsType::Max) const;
    void drawCards(int n, const char *reason = NULL);
    bool askForSkillInvoke(const char *skill_name, const QVariant &data = QVariant());
    bool askForSkillInvoke(const Skill *skill, const QVariant &data = QVariant());
    QList<int> forceToDiscard(int discard_num, bool include_equip, bool is_discard = true);
    QList<int> handCards() const;
    virtual QList<const Card *> getHandcards() const;
    QList<const Card *> getCards(const char *flags) const;
    DummyCard *wholeHandCards() const;
    bool hasNullification() const;
    PindianStruct *pindianSelect(ServerPlayer *target, const char *reason, const Card *card1 = NULL);
    bool pindian(PindianStruct *pd); //pd is deleted after this function
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

    bool isOnline() const;
    bool isOffline() const;

    virtual int aliveCount(bool includeRemoved = true) const;
    int getPlayerNumWithSameKingdom(const char *reason, const char *_to_calculate = NULL,
                                    MaxCardsType::MaxCardsCount type = MaxCardsType::Max) const;
    virtual int getHandcardNum() const;
    virtual void removeCard(const Card *card, Place place);
    virtual void addCard(const Card *card, Place place);
    virtual bool isLastHandCard(const Card *card, bool contain = false) const;

    void addVictim(ServerPlayer *victim);
    QList<ServerPlayer *> getVictims() const;

    void setNext(ServerPlayer *next);
    ServerPlayer *getNext() const;
    ServerPlayer *getNextAlive(int n = 1) const;

    void startRecord();
    void saveRecord(const char *filename);

    // 3v3 methods
    void addToSelected(const char *general);
    QStringList getSelected() const;
    //QString findReasonable(const QStringList &generals, bool no_unreasonable = false);
    void clearSelected();

    int getGeneralMaxHp() const;
    virtual QString getGameMode() const;

    QString getIp() const;
    void introduceTo(ServerPlayer *player);
    void marshal(ServerPlayer *player) const;

    void addToPile(const char *pile_name, const Card *card, bool open = true, QList<ServerPlayer *> open_players = QList<ServerPlayer *>());
    void addToPile(const char *pile_name, int card_id, bool open = true, QList<ServerPlayer *> open_players = QList<ServerPlayer *>());
    void addToPile(const char *pile_name, QList<int> card_ids, bool open = true, QList<ServerPlayer *> open_players = QList<ServerPlayer *>());
    void addToPile(const char *pile_name, QList<int> card_ids, bool open, QList<ServerPlayer *> open_players, CardMoveReason reason);
    void gainAnExtraTurn();

    void copyFrom(ServerPlayer *sp);

    // static function
    static bool CompareByActionOrder(ServerPlayer *a, ServerPlayer *b);

    void showGeneral(bool head_general = true, bool trigger_event = true, bool sendLog = true);
    void hideGeneral(bool head_general = true);
    void removeGeneral(bool head_general = true);
    void sendSkillsToOthers(bool head_skill = true);
    void disconnectSkillsFromOthers(bool head_skill = true);
    bool askForGeneralShow(bool one = true, bool refusable = false);
    void notifyPreshow();

    bool inSiegeRelation(const ServerPlayer *skill_owner, const ServerPlayer *victim) const;
    bool inFormationRalation(ServerPlayer *teammate) const;
    void summonFriends(const HegemonyMode::ArrayType type);

    virtual QStringList getBigKingdoms(const char *reason, MaxCardsType::MaxCardsCount type = MaxCardsType::Min) const;
};

%extend ServerPlayer {
    void speak(const char *msg) {
        QString str = QString::fromUtf8(msg);
        $self->getRoom()->speakCommand($self, str);
    }
};

class ClientPlayer: public Player {
public:
    explicit ClientPlayer(Client *client);
    virtual QList<const Card *> getHandcards() const;
    void setCards(const QList<int> &card_ids);
    void setHandcardNum(int n);
    virtual QString getGameMode() const;
    virtual void setFlags(const char *flag);
    virtual int aliveCount(bool includeRemoved = true) const;
    virtual int getHandcardNum() const;
    virtual void removeCard(const Card *card, Place place);
    virtual void addCard(const Card *card, Place place);
    virtual void addKnownHandCard(const Card *card);
    virtual bool isLastHandCard(const Card *card, bool contain = false) const;
    virtual void setMark(const char *mark, int value);

    virtual QStringList getBigKingdoms(const char *reason, MaxCardsType::MaxCardsCount type = MaxCardsType::Min) const;
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
    CardMoveReason(int moveReason, const char *playerId);
    CardMoveReason(int moveReason, const char *playerId, const char *skillName, const char *eventName);
    CardMoveReason(int moveReason, const char *playerId, const char *targetId, const char *skillName, const char *eventName);


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
    static const int S_REASON_TURNOVER = 0x18;          // show n cards from drawpile
    static const int S_REASON_JUDGE = 0x28;             // show a card from drawpile for judge
    static const int S_REASON_PREVIEW = 0x38;           // Not done yet, plan for view some cards for self only(guanxing yiji miji)
    static const int S_REASON_DEMONSTRATE = 0x48;       // show a card which copy one to move to table

    //subcategory of transfer
    static const int S_REASON_SWAP = 0x19;              // exchange card for two players
    static const int S_REASON_OVERRIDE = 0x29;          // exchange cards from cards in game
    static const int S_REASON_EXCHANGE_FROM_PILE = 0x39;// exchange cards from cards moved out of game (for qixing only)

    //subcategory of put
    static const int S_REASON_NATURAL_ENTER = 0x1A;     //  a card with no-owner move into discardpile
    //  e.g. delayed trick enters discardpile
    static const int S_REASON_REMOVE_FROM_PILE = 0x2A;  //  cards moved out of game go back into discardpile
    static const int S_REASON_JUDGEDONE = 0x3A;         //  judge card move into discardpile
    static const int S_REASON_CHANGE_EQUIP = 0x4A;      //  replace existed equip

    static const int S_MASK_BASIC_REASON = 0x0F;
};

struct DamageStruct {
    enum Nature {
        Normal, // normal slash, duel and most damage caused by skill
        Fire,  // fire slash, fire attack and few damage skill (Yeyan, etc)
        Thunder // lightning, thunder slash, and few damage skill (Leiji, etc)
    };

    DamageStruct();
    DamageStruct(const Card *card, ServerPlayer *from, ServerPlayer *to, int damage = 1, Nature nature = Normal);
    DamageStruct(const char *reason, ServerPlayer *from, ServerPlayer *to, int damage = 1, Nature nature = Normal);

    ServerPlayer *from;
    ServerPlayer *to;
    const Card *card;
    int damage;
    Nature nature;
    bool chain;
    bool transfer;
    bool by_user;
    QString reason;
    QString transfer_reason;
    bool prevented;

    QString getReason() const;
};

struct CardEffectStruct {
    CardEffectStruct();

    const Card *card;

    ServerPlayer *from;
    ServerPlayer *to;

    bool nullified;
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

    bool nullified;
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

    const Card *card;
    ServerPlayer *from;
    QList<ServerPlayer *> to;
    bool m_isOwnerUse;
    bool m_addHistory;
    bool m_isHandcard;
    QStringList nullified_list;
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
    bool open; // helper to prevent sending card_id to unrelevant clients
    bool is_last_handcard;

    Player::Place origin_from_place, origin_to_place;
    Player *origin_from, *origin_to;
    QString origin_from_pile_name, origin_to_pile_name; //for case of the movement transitted

    bool isRelevant(const Player *player);
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

    ServerPlayer *who; // who is dead
    DamageStruct *damage; // if it is NULL that means the dying is caused by losing hp
};

struct RecoverStruct {
    RecoverStruct();

    int recover;
    ServerPlayer *who;
    const Card *card;
};

struct PindianStruct {
    PindianStruct();
    bool isSuccess() const;

    ServerPlayer *from;
    ServerPlayer *to;
    const Card *from_card;
    const Card *to_card;
    int from_number;
    int to_number;
    QString reason;
    bool success;
};

struct JudgeStruct {
    JudgeStruct();
    bool isGood() const;
    bool isBad() const;
    bool isEffected() const;
    void updateResult();

    bool isGood(const Card *card) const; // For AI

    ServerPlayer *who;
    const Card *card;
    QString pattern;
    bool good;
    QString reason;
    bool time_consuming;
    bool negative;
    bool play_animation;
};

struct PhaseChangeStruct {
    PhaseChangeStruct();
    Player::Phase from;
    Player::Phase to;
};

struct CardResponseStruct {
    CardResponseStruct();
    CardResponseStruct(const Card *card);
    CardResponseStruct(const Card *card, ServerPlayer *who);
    CardResponseStruct(const Card *card, bool isUse);
    CardResponseStruct(const Card *card, ServerPlayer *who, bool isUse);

    const Card *m_card;
    ServerPlayer *m_who;
    bool m_isUse;
    bool m_isRetrial;
    bool m_isHandcard;
};

struct PlayerNumStruct {
	PlayerNumStruct();
    PlayerNumStruct(int num, const char *toCalculate);
    PlayerNumStruct(int num, const char *toCalculate, MaxCardsType::MaxCardsCount type);
    PlayerNumStruct(int num, const char *toCalculate, MaxCardsType::MaxCardsCount type, const char *reason);

    MaxCardsType::MaxCardsCount m_type;
    int m_num;
    QString m_toCalculate;
    QString m_reason;
};

struct RoomInfoStruct {
    const QString Name;
    const QString GameMode;
    const QSet<QString> BanPackages;
    const int OperationTimeout;
    const int NullificationCountDown;
    const bool RandomSeat;
    const bool EnableCheat;
    const bool FreeChoose;
    const bool ForbidAddingRobot;
    const bool DisableChat;
    const bool FirstShowingReward;
    const bool RequirePassword;
};

extern RoomInfoStruct ServerInfo;

enum TriggerEvent {
    NonTrigger,

    GameStart,
    TurnStart,
    EventPhaseStart,
    EventPhaseProceeding,
    EventPhaseEnd,
    EventPhaseChanging,
    EventPhaseSkipping,

    ConfirmPlayerNum, // hongfa only

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
    RemoveStateChanged,

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

    PreCardUsed,
    CardUsed,
    TargetChoosing, //distinguish "choose target" and "confirm target"
    TargetConfirming,
    TargetChosen,
    TargetConfirmed,
    CardEffect,
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

    DFDebut,

    NumOfEvents
};

class Card: public QObject {
public:
    // enumeration type
    enum Suit { Spade, Club, Heart, Diamond, NoSuitBlack, NoSuitRed, NoSuit, SuitToBeDecided = -1 };
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
    QString getDescription(bool inToolTip = true) const;

    virtual bool isMute() const;
    virtual bool willThrow() const;
    virtual bool canRecast() const;
    virtual bool hasPreAction() const;
    virtual Card::HandlingMethod getHandlingMethod() const;

    virtual void setFlags(const char *flag) const;
    //virtual void setFlags(const QStringList &fs);
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
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self, int &maxVotes) const;
    virtual bool isAvailable(const Player *player) const;

    virtual const Card *getRealCard() const;
    virtual const Card *validate(CardUseStruct &cardUse) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;

    virtual void doPreAction(Room *room, const CardUseStruct &card_use) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;

    virtual QStringList checkTargetModSkillShow(const CardUseStruct &use) const;

    virtual QString showSkill() const;
    virtual void setShowSkill(const char *skill_name);

    virtual bool isKindOf(const char *cardType) const;
    virtual QStringList getFlags() const;

    virtual bool isModified() const;
    virtual void onNullified(ServerPlayer *target) const;

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

    bool isTransferable() const;
    void setTransferable(const bool transferbale);
};

%extend Card {
    EquipCard *toEquipCard() {
        return qobject_cast<EquipCard *>($self);
    }

    Weapon *toWeapon() {
        return qobject_cast<Weapon *>($self);
    }

    Armor *toArmor() {
        return qobject_cast<Armor *>($self);
    }

    Treasure *toTreasure() {
        return qobject_cast<Treasure *>($self);
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
    WrappedCard(Card *card);
    ~WrappedCard();
    virtual void setId(int id);
    virtual void setNumber(int number);
    virtual void setSuit(Suit suit);
    virtual void setSkillName(const char *skillName);
    // Set the internal card to be the new card, update everything related
    // to CardEffect including objectName.
    void takeOver(Card *card);
    void copyEverythingFrom(Card *card);
    void setModified(bool modified);
    // Inherited member functions
    virtual void onNullified(ServerPlayer *target) const;
    virtual bool isModified() const;
    virtual QString getClassName() const;
    virtual const Card *getRealCard() const;
    virtual bool isMute() const;
    virtual bool willThrow() const;
    virtual bool canRecast() const;
    virtual Card::HandlingMethod getHandlingMethod() const;
    virtual bool hasPreAction() const;
    virtual QString getPackage() const;
    virtual bool isVirtualCard() const;
    virtual QString getCommonEffectName() const;
    virtual bool match(const char *pattern) const;
    virtual void setFlags(const char *flag) const;
    virtual QString getType() const;
    virtual QString getSubtype() const;
    virtual CardType getTypeId() const;
    virtual QString toString(bool hidden = false) const;
    virtual bool isNDTrick() const;
    // card target selection
    virtual bool targetFixed() const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    // @todo: the following two functions should be merged into one.
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self, int &maxVotes) const;
    virtual bool isAvailable(const Player *player) const;
    virtual const Card *validate(CardUseStruct &cardUse) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
    virtual void doPreAction(Room *room, const CardUseStruct &cardUse) const;
    virtual void onUse(Room *room, const CardUseStruct &cardUse) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;
    virtual bool isKindOf(const char *cardType) const;
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
    const Card *validate(CardUseStruct &card_use) const;
};

class Package: public QObject {
public:
    enum Type { GeneralPack, CardPack, MixedPack, SpecialPack };

    Package(const char *name, Type pack_type = GeneralPack);
    void insertRelatedSkills(const char *main_skill, const char *related_skill);
};

class Engine: public QObject {
public:
    Engine();
    ~Engine();

    void addTranslationEntry(const char *key, const char *value);
    QString translate(const char *toTranslate) const;
    QString translate(const char *toTranslate, const char *defaultValue) const;

    void addPackage(Package *package);
    void addBanPackage(const char *package_name);
    QStringList getBanPackages() const;
    Card *cloneCard(const Card *card) const;
    Card *cloneCard(const char *name, Card::Suit suit = Card::SuitToBeDecided, int number = -1) const;
    SkillCard *cloneSkillCard(const char *name) const;
    //QSanVersionNumber getVersionNumber() const;
    QString getVersion() const;
    QString getVersionName() const;
    QString getMODName() const;
    QStringList getExtensions() const;
    QStringList getKingdoms() const;
    QStringList getChattingEasyTexts() const;

    QString getModeName(const char *mode) const;
    int getPlayerCount(const char *mode) const;
    QString getRoles(const char *mode) const;
    QStringList getRoleList(const char *mode) const;

    const CardPattern *getPattern(const char *name) const;
    bool matchExpPattern(const char *pattern, const Player *player, const Card *card) const;
    Card::HandlingMethod getCardHandlingMethod(const char *method_name) const;
    QList<const Skill *> getRelatedSkills(const char *skill_name) const;
    const Skill *getMainSkill(const char *skill_name) const;

    QStringList getModScenarioNames() const;
    void addScenario(Scenario *scenario);
    const Scenario *getScenario(const char *name) const;
    void addPackage(const char *name);

    const General *getGeneral(const char *name) const;
    int getGeneralCount(bool include_banned = false) const;
    const Skill *getSkill(const char *skill_name) const;
    const Skill *getSkill(const EquipCard *card) const;
    QStringList getSkillNames() const;
    const TriggerSkill *getTriggerSkill(const char *skill_name) const;
    const ViewAsSkill *getViewAsSkill(const char *skill_name) const;
    void addSkills(const QList<const Skill *> &skills);

    int getCardCount() const;
    const Card *getEngineCard(int cardId) const;
    // @todo: consider making this const Card *
    Card *getCard(int cardId);
    WrappedCard *getWrappedCard(int cardId);
    QStringList getRandomGenerals(int count) const;
    QList<int> getRandomCards() const;
    QString getRandomGeneralName() const;
    QStringList getLimitedGeneralNames() const;
    QStringList getGeneralNames() const;

    void playSystemAudioEffect(const char *name) const;
    void playAudioEffect(const char *filename) const;
    void playSkillAudioEffect(const char *skill_name, int index) const;

    //const ProhibitSkill *isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others = QList<const Player *>()) const;
    int correctDistance(const Player *from, const Player *to) const;
    int correctMaxCards(const ServerPlayer *target, bool fixed = false, MaxCardsType::MaxCardsCount type = MaxCardsType::Max) const;
    int correctCardTarget(const TargetModSkill::ModType type, const Player *from, const Card *card) const;
    int correctAttackRange(const Player *target, bool include_weapon = true, bool fixed = false) const;

    Room *currentRoom();

    QString getCurrentCardUsePattern();
    CardUseStruct::CardUseReason getCurrentCardUseReason();

    bool isGeneralHidden(const char *general_name) const;
};

%extend Engine {
    QString getVersionNumber() const {
        return $self->getVersionNumber().toString();
    }
};

extern Engine *Sanguosha;

class Skill: public QObject {
public:
    enum Frequency {
        Frequent,
        NotFrequent,
        Compulsory,
        Limited,
        Wake
    };

    explicit Skill(const char *name, Frequency frequent = NotFrequent);
    bool isLordSkill() const;
    bool isAttachedLordSkill() const;
    QString getDescription(bool inToolTip = true) const;
    QString getNotice(int index) const;
    bool isVisible() const;

    virtual int getEffectIndex(const ServerPlayer *player, const Card *card) const;
    virtual QDialog *getDialog() const;

    void initMediaSource();
    void playAudioEffect(int index = -1) const;
    Frequency getFrequency() const;
    QString getLimitMark() const;
    QStringList getSources() const;

    virtual bool canPreshow() const;
    virtual bool relateToPlace(bool head = true) const;

    //for LUA
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
    virtual bool triggerable(const ServerPlayer *target) const;

    //virtual TriggerList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;
    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const;
    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;
    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;

    bool isGlobal() const;

    virtual ~TriggerSkill();
};

%extend TriggerSkill {
    const BattleArraySkill *toBattleArraySkill() const{
        return qobject_cast<const BattleArraySkill *>($self);
    }

    TriggerList TriggerSkillTriggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        return $self->TriggerSkill::triggerable(triggerEvent, room, player, data);
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

struct RoomConfig {
    bool isBanned(const QString &first, const QString &second);

    QString GameMode;
    QString ServerName;
    int OperationTimeout;
    int CountDownSeconds;
    int NullificationCountDown;
    int OriginAIDelay;
    int AIDelay;
    int AIDelayAD;
    int LuckCardLimitation;
    int PileSwappingLimitation;
    int HegemonyMaxChoice;
    bool AIChat;
    bool RandomSeat;
    bool EnableCheat;
    bool FreeChoose;
    bool DisableChat;
    bool EnableMinimizeDialog;
    bool RewardTheFirstShowingPlayer;
    bool ForbidAddingRobot;
    bool AlterAIDelayAD;
    bool DisableLua;
    bool SurrenderAtDeath;
    bool EnableLordConvertion;
};

class Room: public QThread {
public:
    enum GuanxingType { GuanxingUpOnly = 1, GuanxingBothSides = 0, GuanxingDownOnly = -1 };

    explicit Room(QObject *parent);
    ~Room();
    bool isFull() const;
    bool isFinished() const;
    bool canPause(ServerPlayer *p) const;
    void tryPause();
    const RoomConfig &getConfig() const;
    QString getMode() const;
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
    void setPlayerCardLimitation(ServerPlayer *player, const char *limit_list, const char *pattern, bool single_turn);
    void removePlayerCardLimitation(ServerPlayer *player, const char *limit_list, const char *pattern);
    void clearPlayerCardLimitation(ServerPlayer *player, bool single_turn);
    void setPlayerDisableShow(ServerPlayer *player, const char *flags, const char *reason);
    void removePlayerDisableShow(ServerPlayer *player, const char *reason);
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
    bool cardEffect(const Card *card, ServerPlayer *from, ServerPlayer *to, bool multiple = false);
    bool cardEffect(const CardEffectStruct &effect);
    bool isJinkEffected(ServerPlayer *user, const Card *jink);
    void judge(JudgeStruct &judge_struct);
    void sendJudgeResult(const JudgeStruct *judge);
    QList<int> getNCards(int n, bool update_pile_number = true);
    ServerPlayer *getLord(const char *kingdom, bool include_death = false) const;
    void askForGuanxing(ServerPlayer *zhuge, const QList<int> &cards, GuanxingType guanxing_type = GuanxingBothSides);
    int doGongxin(ServerPlayer *shenlvmeng, ServerPlayer *target, QList<int> enabled_ids = QList<int>(), const char *skill_name = "shangyi");
    int drawCard();
    void fillAG(const QList<int> &card_ids, ServerPlayer *who = NULL, const QList<int> &disabled_ids = QList<int>());
    void takeAG(ServerPlayer *player, int card_id, bool move_cards = true);
    void clearAG(ServerPlayer *player = NULL);
    void provide(const Card *card);
    QList<ServerPlayer *> getLieges(const char *kingdom, ServerPlayer *lord) const;
    void sendLog(const LogMessage &log, QList<ServerPlayer *> players = QList<ServerPlayer *>());
    void sendLog(const LogMessage &log, ServerPlayer *player);
    void sendCompulsoryTriggerLog(ServerPlayer *player, const char *skill_name, bool notify_skill = true);
    void showCard(ServerPlayer *player, int card_id, ServerPlayer *only_viewer = NULL);
    void showAllCards(ServerPlayer *player, ServerPlayer *to = NULL);
    void retrial(const Card *card, ServerPlayer *player, JudgeStruct *judge, const char *skill_name, bool exchange = false);


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
    void broadcastSkillInvoke(const char *skillName, const ServerPlayer *player = NULL);
    void broadcastSkillInvoke(const char *skillName, const char *category);
    void broadcastSkillInvoke(const char *skillName, int type,
                              const ServerPlayer *player = NULL);
    void broadcastSkillInvoke(const char *skillName, bool isMale, int type);
    void doLightbox(const char *lightboxName, int duration = 2000);
    void doSuperLightbox(const char *heroName, const char *skillName);

    inline void doAnimate(int type, const char *arg1 = NULL, const char *arg2 = NULL, QList<ServerPlayer *> players = QList<ServerPlayer *>());

    void preparePlayers();
    void changePlayerGeneral(ServerPlayer *player, const char *new_general);
    void changePlayerGeneral2(ServerPlayer *player, const char *new_general);
    void filterCards(ServerPlayer *player, QList<const Card *> cards, bool refilter);

    void acquireSkill(ServerPlayer *player, const Skill *skill, bool open = true, bool head = true);
    void acquireSkill(ServerPlayer *player, const char *skill_name, bool open = true, bool head = true);
    void adjustSeats();
    void swapPile();
    QList<int> getDiscardPile();
    inline QList<int> &getDrawPile();
    inline const QList<int> &getDrawPile() const;
    int getCardFromPile(const char *card_name);
    ServerPlayer *findPlayer(const char *general_name, bool include_dead = false) const;
    QList<ServerPlayer *> findPlayersBySkillName(const char *skill_name) const;
    ServerPlayer *findPlayerBySkillName(const char *skill_name) const;
    void installEquip(ServerPlayer *player, const char *equip_name);
    void resetAI(ServerPlayer *player);
    //void changeHero(ServerPlayer *player, const char *new_general, bool full_state, bool invoke_start = true, bool isSecondaryHero = false, bool sendLog = true);
    void swapSeat(ServerPlayer *a, ServerPlayer *b);
    void setFixedDistance(Player *from, const Player *to, int distance);
    ServerPlayer *getFront(ServerPlayer *a, ServerPlayer *b) const;
    void signup(ServerPlayer *player, const char *screen_name, const char *avatar, bool is_robot);
    ServerPlayer *getOwner() const;
    void updateStateItem();

    void reconnect(ServerPlayer *player, AbstractClientSocket *socket);
    void marshal(ServerPlayer *player);

    void sortByActionOrder(QList<ServerPlayer *> &players);

    void setTag(const char *key, const QVariant &value);
    QVariant getTag(const char *key) const;
    void removeTag(const char *key);

    void setEmotion(ServerPlayer *target, const char *emotion);

    Player::Place getCardPlace(int card_id) const;
    QList<int> getCardIdsOnTable(const Card *) const;
    QList<int> getCardIdsOnTable(const QList<int> &card_ids) const;
    ServerPlayer *getCardOwner(int card_id) const;
    void setCardMapping(int card_id, ServerPlayer *owner, Player::Place place);

    void drawCards(ServerPlayer *player, int n, const char *reason = NULL);
    void drawCards(QList<ServerPlayer *> players, int n, const char *reason = NULL);
    void drawCards(QList<ServerPlayer *> players, QList<int> n_list, const char *reason = NULL);
    void obtainCard(ServerPlayer *target, const Card *card, bool unhide = true);
    void obtainCard(ServerPlayer *target, int card_id, bool unhide = true);
    void obtainCard(ServerPlayer *target, const Card *card, const CardMoveReason &reason, bool unhide = true);

    void throwCard(int card_id, ServerPlayer *who, ServerPlayer *thrower = NULL, const char *skill_name = "");
    void throwCard(const Card *card, ServerPlayer *who, ServerPlayer *thrower = NULL, const char *skill_name = "");
    void throwCard(const Card *card, const CardMoveReason &reason, ServerPlayer *who, ServerPlayer *thrower = NULL, const char *skill_name = "");

    void moveCardTo(const Card *card, ServerPlayer *dstPlayer, Player::Place dstPlace, bool forceMoveVisible = false);
    void moveCardTo(const Card *card, ServerPlayer *dstPlayer, Player::Place dstPlace, const CardMoveReason &reason,
        bool forceMoveVisible = false);
    void moveCardTo(const Card *card, ServerPlayer *srcPlayer, ServerPlayer *dstPlayer, Player::Place dstPlace,
        const CardMoveReason &reason, bool forceMoveVisible = false);
    void moveCardTo(const Card *card, ServerPlayer *srcPlayer, ServerPlayer *dstPlayer, Player::Place dstPlace,
        const char *pileName, const CardMoveReason &reason, bool forceMoveVisible = false);
    void moveCardsAtomic(QList<CardsMoveStruct> cards_move, bool forceMoveVisible);
    void moveCardsAtomic(CardsMoveStruct cards_move, bool forceMoveVisible);
    void moveCards(CardsMoveStruct cards_move, bool forceMoveVisible, bool ignoreChanges = true);
    void moveCards(QList<CardsMoveStruct> cards_moves, bool forceMoveVisible, bool ignoreChanges = true);

    // interactive methods
    void activate(ServerPlayer *player, CardUseStruct &card_use);
    void askForLuckCard();
    Card::Suit askForSuit(ServerPlayer *player, const char *reason);
    QString askForKingdom(ServerPlayer *player);
    bool askForSkillInvoke(ServerPlayer *player, const char *skill_name, const QVariant &data = QVariant());
    QString askForChoice(ServerPlayer *player, const char *skill_name, const char *choices, const QVariant &data = QVariant());
    bool askForDiscard(ServerPlayer *target, const char *reason, int discard_num, int min_num,bool optional = false, bool include_equip = false, const char *prompt = NULL, bool notify_skill = false);
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
    QString askForGeneral(ServerPlayer *player, const char *generals, const char *default_choice = NULL, bool single_result = true, const char *skill_name = NULL, const QVariant &data = QVariant());
    const Card *askForSinglePeach(ServerPlayer *player, ServerPlayer *dying);
    void addPlayerHistory(ServerPlayer *player, const char *key, int times = 1);

    inline Card *getCard(int cardId) const;
    inline void resetCard(int cardId);
    void updateCardsOnLose(const CardsMoveStruct &move);
    void updateCardsOnGet(const CardsMoveStruct &move);

    // these 2 functions puts here, for convenience
    static void cancelTarget(CardUseStruct &use, const char *name);
    static void cancelTarget(CardUseStruct &use, ServerPlayer *player);
};

%extend Room {
    ServerPlayer *nextPlayer(ServerPlayer *player) const{
        Q_UNUSED($self);
        return qobject_cast<ServerPlayer *>(player->getNextAlive());
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

    int err = luaL_dostring(L, script.toLatin1());
    if (err){
        QString err_str = lua_tostring(L, -1);
        lua_pop(L, 1);
        output(err_str);
        qWarning("%s", err_str.toLatin1().constData());
    }
}

%}

%include "card.i"
%include "luaskills.i"
%include "ai.i"
