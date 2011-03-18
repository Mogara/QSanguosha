%module sgs

%{
#include "structs.h"
#include "engine.h"

#include <QDir>

%}

%include "naturalvar.i"
%include "native.i"
%include "qvariant.i"
%include "list.i"

// ----------------------------------------

class QObject{
public:
	QString objectName();
	void setObjectName(const char *name);
	bool inherits(const char *class_name);
	bool setProperty ( const char * name, const QVariant & value);
	QVariant property ( const char * name ) const;
};

%extend QObject{
	const char *className() const{
		return $self->metaObject()->className();
	}
};

class General : public QObject
{
public:
    explicit General(Package *package, const char *name, const char *kingdom, int max_hp = 4, bool male = true, bool hidden = false);

    // property getters/setters
    int getMaxHp() const;
    QString getKingdom() const;
    bool isMale() const;
    bool isFemale() const;
    bool isLord() const;
    bool isHidden() const;

    void addSkill(Skill* skill);
    bool hasSkill(const char *skill_name) const;

    QString getPixmapPath(const char *category) const;    
    QString getPackage() const;
    QString getSkillDescription() const;
	
    void lastWord() const;
};

class Player: public QObject
{
public:
    enum Phase {Start, Judge, Draw, Play, Discard, Finish, NotActive};
    enum Place {Hand, Equip, Judging, Special, DiscardedPile, DrawPile};
    enum Role {Lord, Loyalist, Rebel, Renegade};

    explicit Player(QObject *parent);

    void setScreenName(const char *screen_name);
    QString screenName() const;

    // property setters/getters
    int getHp() const;
    void setHp(int hp);    
    int getMaxHP() const;
    void setMaxHP(int max_hp);    
    int getLostHp() const;
    bool isWounded() const;

    int getMaxCards() const;    
    int getXueyi() const;
    void setXueyi(int xueyi);

    QString getKingdom() const;
    void setKingdom(const char *kingdom);
    QString getKingdomIcon() const;
    QString getKingdomFrame() const;

    void setRole(const char *role);
    QString getRole() const;    
    Role getRoleEnum() const;

    void setGeneralName(const char *general_name);
    QString getGeneralName() const;    

    void setGeneral2Name(const char *general_name);
    QString getGeneral2Name() const;
    const General *getGeneral2() const;

    void setState(const char *state);
    QString getState() const;

    int getSeat() const;
    void setSeat(int seat);  
    QString getPhaseString() const;
    void setPhaseString(const char *phase_str);
    Phase getPhase() const;
    void setPhase(Phase phase);

    void setAttackRange(int attack_range);
    int getAttackRange() const;
    QString getCorrect() const;
    void setCorrect(const char *correct_str);
    bool inMyAttackRange(const Player *other) const;

    bool isAlive() const;
    bool isDead() const;
    void setAlive(bool alive);

    QString getFlags() const;
    void setFlags(const char *flag);
    bool hasFlag(const char *flag) const;

    bool faceUp() const;
    void setFaceUp(bool face_up);

    virtual int aliveCount() const = 0;
    int distanceTo(const Player *other) const;
    int getGeneralMaxHP() const;
    const General *getAvatarGeneral() const;
    const General *getGeneral() const;

    void acquireSkill(const char *skill_name);
    void loseSkill(const char *skill_name);
    bool hasSkill(const char *skill_name) const;

    void setEquip(const EquipCard *card);
    void removeEquip(const EquipCard *equip);

    QStack<const Card *> getJudgingArea() const;
    void addDelayedTrick(const Card *trick);
    void removeDelayedTrick(const Card *trick);
    QStack<const DelayedTrick *> delayedTricks() const;
    bool containsTrick(const char *trick_name) const;
    const DelayedTrick *topDelayedTrick() const;

    virtual int getHandcardNum() const = 0;
    virtual void removeCard(const Card *card, Place place) = 0;
    virtual void addCard(const Card *card, Place place) = 0;

    const Weapon *getWeapon() const;
    const Armor *getArmor() const;
    const Horse *getDefensiveHorse() const;
    const Horse *getOffensiveHorse() const;
    QList<const Card *> getEquips() const;

    bool hasWeapon(const char *weapon_name) const;
    bool hasArmorEffect(const char *armor_name) const;

    bool isKongcheng() const;
    bool isNude() const;
    bool isAllNude() const;

    void addMark(const char *mark);
    void removeMark(const char *mark);
    virtual void setMark(const char *mark, int value);
    int getMark(const char *mark) const;

    void setChained(bool chained);
    bool isChained() const;

    bool canSlash(const Player *other, bool distance_limit = true) const;
    int getCardCount(bool include_equip) const;

    QList<int> &getPile(const char *pile_name);
	
	QVariantMap tag;
};

class ServerPlayer : public Player
{
public:
    void invoke(const char *method, const char *arg = ".");
    void sendProperty(const char *property_name);
    void unicast(const char *message);
    void drawCard(const Card *card);
    Room *getRoom() const;
    void playCardEffect(const Card *card);
    int getRandomHandCard() const;
    void obtainCard(const Card *card);
    void throwAllEquips();
    void throwAllHandCards();
    void throwAllCards();
    void drawCards(int n, bool set_emotion = true);
    bool askForSkillInvoke(const char *skill_name, const QVariant &data = QVariant());
    QList<int> forceToDiscard(int discard_num, bool include_equip);
    QList<int> handCards() const;
    QList<const Card *> getHandcards() const;
    QList<const Card *> getCards(const char *flags) const;
    DummyCard *wholeHandCards() const;
    bool isLord() const;
    bool hasNullification() const;
    bool pindian(ServerPlayer *target, const Card *card1 = NULL);
	void turnOver();

    void gainMark(const char *mark, int n = 1);
    void loseMark(const char *mark, int n = 1);

    virtual int aliveCount() const;
    virtual int getHandcardNum() const;
    virtual void removeCard(const Card *card, Place place);
    virtual void addCard(const Card *card, Place place);
};

class ClientPlayer : public Player
{
public:
    explicit ClientPlayer(Client *client);
    virtual int aliveCount() const;
    virtual int getHandcardNum() const;	
    virtual void removeCard(const Card *card, Place place);
    virtual void addCard(const Card *card, Place place);
    virtual void addKnownHandCard(const Card *card);
    virtual bool isLastHandCard(const Card *card) const; 
};

extern ClientPlayer *Self;

struct DamageStruct{
    DamageStruct();

    enum Nature{
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
};

struct CardEffectStruct{
    CardEffectStruct();

    const Card *card;

    ServerPlayer *from;
    ServerPlayer *to;

    bool multiple;
};

struct SlashEffectStruct{
    SlashEffectStruct();

    const Slash *slash;

    ServerPlayer *from;
    ServerPlayer *to;

    bool drank;

    DamageStruct::Nature nature;
};

struct CardUseStruct{
    CardUseStruct();
    bool isValid() const;
    void parse(const char *str, Room *room);

    const Card *card;
    ServerPlayer *from;
    QList<ServerPlayer *> to;
};

struct CardMoveStruct{
    int card_id;
    Player::Place from_place, to_place;
    ServerPlayer *from, *to;

    QString toString() const;
};

struct DyingStruct{
    DyingStruct();

    ServerPlayer *who; // who is ask for help
    DamageStruct *damage; // if it is NULL that means the dying is caused by losing hp
    int peaches; // peaches that needs
};

enum TriggerEvent{
    GameStart,
    PhaseChange,
    DrawNCards,
    JudgeOnEffect,
    HpRecover,

    Predamage,
    Predamaged,
    DamageDone,
    Damage,
    Damaged,
    DamageComplete,

    Dying,
    AskForPeaches,
    AskForPeachesDone, 
    Death,

    SlashEffect,
    SlashEffected,
    SlashProceed,
    SlashHit,
    SlashMissed,

    CardAsked,
    CardUsed,
    CardResponsed,
    CardDiscarded,
    CardLost,
    CardGot,

    CardEffect,
    CardEffected,
    CardFinished
};

class Card: public QObject
{

public:
    // enumeration type
    enum Suit {Spade, Club, Heart, Diamond, NoSuit};
    static const Suit AllSuits[4];
	
	// card types
    enum CardType{
        Skill,
        Basic,
        Trick,
        Equip,
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
	bool isEquipped() const;

    QString getPixmapPath() const;
    QString getIconPath() const;
    QString getPackage() const;    
    QIcon getSuitIcon() const;
    QString getFullName(bool include_suit = false) const;
    QString getLogName() const;
    QString getName() const;
    QString getSkillName() const;   
    void setSkillName(const char *skill_name);
    QString getDescription() const;
    QString getEffectPath() const;

    bool isVirtualCard() const;
    virtual bool match(const char *pattern) const;

    void addSubcard(int card_id);
	void addSubcard(const Card *card);
    QList<int> getSubcards() const;
    void clearSubcards();
    QString subcardString() const;
    void addSubcards(const QList<CardItem *> &card_items);
    int subcardsLength() const;

    virtual QString getType() const = 0;
    virtual QString getSubtype() const = 0;
    virtual CardType getTypeId() const = 0;
    virtual QString toString() const;
    virtual QString getEffectPath(bool is_male) const;
	bool isNDTrick() const;

    // card target selection
    bool targetFixed() const;
    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual bool isAvailable() const;

    // it can be used only once a turn or not
    bool isOnce() const;

    // FIXME: should be pure virtual
    virtual void use(Room *room, ServerPlayer *source,  const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;

    virtual void onMove(const CardMoveStruct &move) const;

    // static functions
    static bool CompareBySuitNumber(const Card *a, const Card *b);
    static bool CompareByType(const Card *a, const Card *b);
    static const Card *Parse(const char *str);
    static Card * Clone(const Card *card);
    static QString Suit2String(Suit suit); 
};

class SkillCard: public Card{
    
};

class DummyCard: public Card{

};

class Package: public QObject{
public:
    Package(const char *name);
};

class Engine: public QObject
{
public:
    void addTranslationEntry(const char *key, const char *value);
    QString translate(const char *to_translate) const;    

    void addPackage(Package *package);
    void addBanPackage(const char *package_name);
    QStringList getBanPackages() const;
    Card *cloneCard(const char *name, Card::Suit suit, int number) const;
    SkillCard *cloneSkillCard(const char *name) const;
    QString getVersion() const;
    QStringList getExtensions() const;
    QStringList getKingdoms() const;
    QString getSetupString() const;    

    QMap<QString, QString> getAvailableModes() const;
    QString getModeName(const char *mode) const;
    int getPlayerCount(const char *mode) const;
    void getRoles(const char *mode, char *roles) const;
    int getRoleIndex() const;

    QStringList getScenarioNames() const;
    void addScenario(Scenario *scenario);
    const Scenario *getScenario(const char *name) const;

    const ChallengeModeSet *getChallengeModeSet() const;
    const ChallengeMode *getChallengeMode(const char *name) const;

    const General *getGeneral(const char *name) const;
    int getGeneralCount(bool include_banned = false) const;
    const Skill *getSkill(const char *skill_name) const;
    const TriggerSkill *getTriggerSkill(const char *skill_name) const;

    int getCardCount() const;
    const Card *getCard(int index) const;

    QStringList getLords() const;
    QStringList getRandomLords() const;
    QStringList getRandomGenerals(int count, const QSet<QString> &ban_set = QSet<QString>()) const;
    QList<int> getRandomCards() const;

    void playAudio(const char *name) const;
    void playEffect(const char *filename) const;
    void playSkillEffect(const char *skill_name, int index) const;
    void playCardEffect(const char *card_name, bool is_male) const;
};

extern Engine *Sanguosha;

class Skill : public QObject
{
public:
    enum Frequency{
        Frequent,
        NotFrequent,
        Compulsory,
        Limited
    };

    explicit Skill(const char *name, Frequency frequent = NotFrequent);
    bool isLordSkill() const;
    QString getDescription() const;
    QString getDefaultChoice() const;

    void initMediaSource();
    void playEffect(int index = -1) const;
    void setFlag(ServerPlayer *player) const;
    void unsetFlag(ServerPlayer *player) const;
    Frequency getFrequency() const;
};

class TriggerSkill:public Skill{
public:
    TriggerSkill(const char *name);
    const ViewAsSkill *getViewAsSkill() const;
    QList<TriggerEvent> getTriggerEvents() const;

    virtual int getPriority() const;
    virtual bool triggerable(const ServerPlayer *target) const;    
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const = 0;
};

class QThread: public QObject{
};

class RoomThread : public QThread{
public:
    explicit RoomThread(Room *room);
    void constructTriggerTable(const GameRule *rule);
    bool trigger(TriggerEvent event, ServerPlayer *target, QVariant &data);
    bool trigger(TriggerEvent event, ServerPlayer *target);

    void addPlayerSkills(ServerPlayer *player, bool invoke_game_start = false);
    void removePlayerSkills(ServerPlayer *player);

    void addTriggerSkill(const TriggerSkill *skill);
    void removeTriggerSkill(const TriggerSkill *skill);
    void removeTriggerSkill(const QString &skill_name);
    void delay(unsigned long msecs = 1000);
    void end();
};

class Room : public QObject{
public:
    explicit Room(QObject *parent, const char *mode);
    void broadcast(const char *message, ServerPlayer *except = NULL);
    RoomThread *getThread() const;
    void playSkillEffect(const char *skill_name, int index = -1);
    ServerPlayer *getCurrent() const;
    int alivePlayerCount() const;
    QList<ServerPlayer *> getOtherPlayers(ServerPlayer *except) const;
    QList<ServerPlayer *> getAllPlayers() const;
    QList<ServerPlayer *> getAlivePlayers() const;
    void nextPlayer();
    void output(const char *message);
    void killPlayer(ServerPlayer *victim, ServerPlayer *killer = NULL);
    QStringList aliveRoles(ServerPlayer *except = NULL) const;
    void gameOver(const char *winner);
    void slashEffect(const SlashEffectStruct &effect);
    void slashResult(const SlashEffectStruct &effect, bool hit);
    void attachSkillToPlayer(ServerPlayer *player, const char *skill_name);
    void detachSkillFromPlayer(ServerPlayer *player, const char *skill_name);
    bool obtainable(const Card *card, ServerPlayer *player);
    void promptUser(ServerPlayer *to, const char *prompt_str);
    void setPlayerFlag(ServerPlayer *player, const char *flag);
    void setPlayerCorrect(ServerPlayer *player, const char *correct_str);
    void setPlayerProperty(ServerPlayer *player, const char *property_name, const QVariant &value);
    void setPlayerMark(ServerPlayer *player, const char *mark, int value);
    void useCard(const CardUseStruct &card_use);
    void damage(const DamageStruct &data);
    void sendDamageLog(const DamageStruct &data);
    void loseHp(ServerPlayer *victim, int lose = 1);
	void loseMaxHp(ServerPlayer *victim, int lose = 1);
    void applyDamage(ServerPlayer *victim, const DamageStruct &damage);
    void recover(ServerPlayer *player, int recover = 1, bool set_emotion = false);
    void playCardEffect(const char *card_name, bool is_male);
    bool cardEffect(const Card *card, ServerPlayer *from, ServerPlayer *to);
    bool cardEffect(const CardEffectStruct &effect);
    QList<int> getNCards(int n, bool update_pile_number = true);
    ServerPlayer *getLord() const;
    void doGuanxing(ServerPlayer *zhuge);
    void doGongxin(ServerPlayer *shenlumeng, ServerPlayer *target);
    int drawCard(); 
	const Card *peek();
    void takeAG(ServerPlayer *player, int card_id);
    void provide(const Card *card);
    QList<ServerPlayer *> getLieges(const char *kingdom, ServerPlayer *lord) const;
    void sendLog(const LogMessage &log);
    void showCard(ServerPlayer *player, int card_id);   
    void getResult(const char *reply_func, ServerPlayer *reply_player, bool move_focus = true);
    void acquireSkill(ServerPlayer *player, const Skill *skill, bool open = true);
    void acquireSkill(ServerPlayer *player, const char *skill_name, bool open = true);
    void adjustSeats();
    void swapPile();
    int getCardFromPile(const QString card_name);
    ServerPlayer *findPlayer(const char *general_name, bool include_dead = false) const;
    ServerPlayer *findPlayerBySkillName(const char *skill_name, bool include_dead = false) const;
    void installEquip(ServerPlayer *player, const char *equip_name);
    void transfigure(ServerPlayer *player, const char *new_general, bool full_state, bool invoke_start = true);
    lua_State *getLuaState() const;

    void addProhibitSkill(const ProhibitSkill *skill);
    const ProhibitSkill *isProhibited(Player *from, Player *to, const Card *card) const;

    void setTag(const char *key, const QVariant &value);
    QVariant getTag(const char *key) const;

    enum TargetType{
        Killer,
        Victim,
        DuelA,
        DuelB,
        Good,
        Bad,
        Normal,
        Recover,
        DrawCard,
    };

    void setEmotion(ServerPlayer *target, TargetType type);

    // related to card transfer
    Player::Place getCardPlace(int card_id) const;
    ServerPlayer *getCardOwner(int card_id) const;
    void setCardMapping(int card_id, ServerPlayer *owner, Player::Place place);

    void drawCards(ServerPlayer *player, int n);
    void obtainCard(ServerPlayer *target, const Card *card);
    void obtainCard(ServerPlayer *target, int card_id);

    void throwCard(const Card *card);
    void throwCard(int card_id);
    int throwSpecialCard();
    void moveCardTo(const Card *card, ServerPlayer *to, Player::Place place, bool open = true);

    // interactive methods
    void activate(ServerPlayer *player, CardUseStruct &card_use);
    Card::Suit askForSuit(ServerPlayer *player);
    QString askForKingdom(ServerPlayer *player);
    bool askForSkillInvoke(ServerPlayer *player, const char *skill_name, const QVariant &data = QVariant());
    QString askForChoice(ServerPlayer *player, const char *skill_name, const char *choices);
    bool askForDiscard(ServerPlayer *target, const char *reason, int discard_num, bool optional = false, bool include_equip = false);
	bool askForNullification(const char *trick_name, ServerPlayer *from, ServerPlayer *to);
    bool isCanceled(const CardEffectStruct &effect);
    int askForCardChosen(ServerPlayer *player, ServerPlayer *who, const char *flags, const char *reason);
    const Card *askForCard(ServerPlayer *player, const char *pattern, const char *prompt);
    bool askForUseCard(ServerPlayer *player, const char *pattern, const char *prompt);
    int askForAG(ServerPlayer *player, const QList<int> &card_ids, bool refusable = false);
    const Card *askForCardShow(ServerPlayer *player, ServerPlayer *requestor);
    bool askForYiji(ServerPlayer *guojia, QList<int> &cards);
    const Card *askForPindian(ServerPlayer *player, const char *ask_str);    
    ServerPlayer *askForPlayerChosen(ServerPlayer *player, const QList<ServerPlayer *> &targets, const char *reason);

    void askForPeaches(const DyingStruct &dying, const QList<ServerPlayer *> &players);
    int askForPeach(ServerPlayer *player, ServerPlayer *dying, int peaches);
    bool askForSinglePeach(ServerPlayer *player, ServerPlayer *dying, int peaches);

    void broadcastProperty(ServerPlayer *player, const char *property_name, const char *value = QString());
    void broadcastInvoke(const char *method, const char *arg = ".", ServerPlayer *except = NULL);
};

%include "luaskills.i"
%include "card.i"
%include "ai.i"

