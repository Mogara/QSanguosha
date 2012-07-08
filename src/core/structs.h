#ifndef STRUCTS_H
#define STRUCTS_H

class Room;
class TriggerSkill;
class ServerPlayer;
class Card;
class Slash;
class GameRule;

#include "player.h"

#include <QVariant>
#include <json/json.h>

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
    bool transfer;
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
    const Card *jink;

    ServerPlayer *from;
    ServerPlayer *to;

    bool drank;

    DamageStruct::Nature nature;
};

struct CardUseStruct{
    CardUseStruct();
    bool isValid() const;
    void parse(const QString &str, Room *room);
    bool tryParse(const Json::Value&, Room *room);

    const Card *card;
    ServerPlayer *from;
    QList<ServerPlayer *> to;
};

class CardMoveReason
{
public:
    int m_reason;
    QString m_playerId; // the cause (not the source) of the movement, such as "lusu" when "dimeng", or "zhanghe" when "qiaobian"
    QString m_targetId; // To keep this structure lightweight, currently this is only used for UI purpose.
                        // It will be set to empty if multiple targets are involved. NEVER use it for trigger condition
                        // judgement!!! It will not accurately reflect the real reason.
    QString m_skillName; // skill that triggers movement of the cards, such as "longdang", "dimeng"
    QString m_eventName; // additional arg such as "lebusishu" on top of "S_REASON_JUDGE"
    inline CardMoveReason(){ m_reason = S_REASON_UNKNOWN; }
    inline CardMoveReason(int moveReason, QString playerId)
    {
        m_reason = moveReason;
        m_playerId = playerId;
    }

    inline CardMoveReason(int moveReason, QString playerId, QString skillName, QString eventName)
    {
        m_reason = moveReason;
        m_playerId = playerId;
        m_skillName = skillName;
        m_eventName = eventName;
    }

    inline CardMoveReason(int moveReason, QString playerId, QString targetId, QString skillName, QString eventName)
    {
        m_reason = moveReason;
        m_playerId = playerId;
        m_targetId = targetId;
        m_skillName = skillName;
        m_eventName = eventName;
    }

    bool tryParse(const Json::Value&);
    Json::Value toJsonValue() const;
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
    static const int S_REASON_CHANGE_EQUIP = 0x33;      //  replace existed equip
    static const int S_REASON_DISMANTLE = 0x43;         //  one throw card of another

    //subcategory of gotcard
    static const int S_REASON_GIVE = 0x17;              // from one hand to another hand
    static const int S_REASON_EXTRACTION = 0x27;        // from another's place to one's hand
    static const int S_REASON_GOTBACK = 0x37;           // from placetable to hand
    static const int S_REASON_RECYCLE = 0x47;           // from discardpile to hand
    static const int S_REASON_ROB = 0x57;               // got a definite card from other's hand

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


    static const int S_MASK_BASIC_REASON = 0x0F;
};

struct CardMoveStruct{
    inline CardMoveStruct()
    {
        from_place = Player::PlaceUnknown;
        to_place = Player::PlaceUnknown;
        from = NULL;
        to = NULL;
    }
    int card_id;
    Player::Place from_place, to_place;
    QString from_player_name, to_player_name;
    QString from_pile_name, to_pile_name;
    Player *from, *to;
    CardMoveReason reason;
    bool open;    
    bool tryParse(const Json::Value&);
    Json::Value toJsonValue() const;
    inline bool isRelevant(Player* player)
    {
        return (player != NULL && (from == player || to == player));
    }
    inline bool hasSameSourceAs(const CardMoveStruct &move)
    {
        return (from == move.from) && (from_place == move.from_place) &&
               (from_player_name == move.from_player_name) && (from_pile_name == move.from_pile_name);
    }
    inline bool hasSameDestinationAs(const CardMoveStruct &move)
    {
        return (to == move.to) && (to_place == move.to_place) &&
               (to_player_name == move.to_player_name) && (to_pile_name == move.to_pile_name);
    } 
};

struct CardsMoveOneTimeStruct{
    QList<int> card_ids;
    QList<Player::Place> from_places;
    Player::Place to_place;
    CardMoveReason reason;
    Player *from, *to;
};

struct CardsMoveStruct{
    inline CardsMoveStruct()
    {
        from_place = Player::PlaceUnknown;
        to_place = Player::PlaceUnknown;
        from = NULL;
        to = NULL;
        countAsOneTime = false;
    }

    inline CardsMoveStruct(const QList<int> &ids, Player* from, Player* to, Player::Place to_place, CardMoveReason reason)
    {
        this->card_ids = ids;
        this->from_place = Player::PlaceUnknown;
        this->to_place = to_place;
        this->from = from;
        this->to = to;
        this->reason = reason;
    }
    inline CardsMoveStruct(const QList<int> &ids, Player* to, Player::Place to_place, CardMoveReason reason)
    {
        this->card_ids = ids;
        this->from_place = Player::PlaceUnknown;
        this->to_place = to_place;
        this->from = NULL;
        this->to = to;
        this->reason = reason;
    }

    inline bool hasSameSourceAs(const CardsMoveStruct &move)
    {
        return (from == move.from) && (from_place == move.from_place) &&
               (from_player_name == move.from_player_name) && (from_pile_name == move.from_pile_name);
    }

    inline bool hasSameDestinationAs(const CardsMoveStruct &move)
    {
        return (to == move.to) && (to_place == move.to_place) &&
               (to_player_name == move.to_player_name) && (to_pile_name == move.to_pile_name);
    }

    QList<int> card_ids;
    Player::Place from_place, to_place;
    QString from_player_name, to_player_name;
    QString from_pile_name, to_pile_name;
    Player *from, *to;
    CardMoveReason reason;
    bool open; // helper to prevent sending card_id to unrelevant clients
    bool countAsOneTime; // helper to identify distinct move counted as one time
    bool tryParse(const Json::Value&);
    Json::Value toJsonValue() const;
    inline bool isRelevant(const Player* player)
    {
        return (player != NULL && (from == player || to == player));
    }
    QList<CardMoveStruct> flatten();    
};

struct DyingStruct{
    DyingStruct();

    ServerPlayer *who; // who is ask for help
    DamageStruct *damage; // if it is NULL that means the dying is caused by losing hp
};

struct RecoverStruct{
    RecoverStruct();

    int recover;
    ServerPlayer *who;
    const Card *card;
};

struct PindianStruct{
    PindianStruct();
    bool isSuccess() const;

    ServerPlayer *from;
    ServerPlayer *to;
    const Card *from_card;
    const Card *to_card;
    QString reason;
};

class JudgeStructPattern{
private:
    QString pattern;
    bool isRegex;

public:
    JudgeStructPattern();
    JudgeStructPattern &operator=(const QRegExp &rx);
    JudgeStructPattern &operator=(const QString &str);
    bool match(const Player *player, const Card *card) const;
};

struct JudgeStruct{
    JudgeStruct();
    bool isGood(const Card *card = NULL) const;
    bool isBad() const;

    ServerPlayer *who;
    const Card *card;
    JudgeStructPattern pattern;
    bool good;
    QString reason;
    bool time_consuming;
};

struct PhaseChangeStruct{
    PhaseChangeStruct();
    Player::Phase from;
    Player::Phase to;
};

enum TriggerEvent{
    NonTrigger,

    GameStart,
    TurnStart,
    EventPhaseStart,
    EventPhaseEnd,
    EventPhaseChanging,
    DrawNCards,
    HpRecover,
    HpLost,
    HpChanged,
    MaxHpChanged,

    StartJudge,
    AskForRetrial,
    FinishJudge,

    Pindian,
    TurnedOver,

    Predamage,        // assure the num of damage,check all buff(ancaletic luoyi...)
    DamageForseen,       // Damage begun! The first moment when you enter "Real Damage Process" (kuangfeng dawu)
    DamageCaused,    // The moment you just damage to someone(qianxi kylinbow ice_sword...)
    DamageInflicted,   // The moment you are just damaged by someone,but can still change(silverlion jilei tianxiang!)
    DamageDone,       // Always trigger,from this one,the damage is assertain,hp changed.
    Damage,
    Damaged,
    DamageComplete,

    Dying,
    AskForPeaches,
    AskForPeachesDone,
    Death,
    GameOverJudge,
    GameFinished,

    SlashEffect,
    SlashEffected,
    SlashProceed,
    SlashHit,
    SlashMissed,

    CardAsked,
    CardResponsed,
    CardDiscarded,
    CardsMoveOneTime,
    CardDrawing,
    CardDrawnDone,

    CardUsed,
    TargetConfirming,
    TargetConfirmed,
    CardEffect,
    CardEffected,
    CardFinished,

    ChoiceMade,

    // For hulao pass only
    StageChange,

    NumOfEvents
};

typedef const Card *CardStar;
typedef ServerPlayer *PlayerStar;
typedef JudgeStruct *JudgeStar;
typedef DamageStruct *DamageStar;
typedef PindianStruct *PindianStar;
typedef const CardMoveStruct *CardMoveStar;
typedef const CardsMoveOneTimeStruct *CardsMoveOneTimeStar;
typedef const CardsMoveStruct *CardsMoveStar;

Q_DECLARE_METATYPE(DamageStruct)
Q_DECLARE_METATYPE(CardEffectStruct)
Q_DECLARE_METATYPE(SlashEffectStruct)
Q_DECLARE_METATYPE(CardUseStruct)
Q_DECLARE_METATYPE(CardsMoveStruct)
Q_DECLARE_METATYPE(CardsMoveStar)
Q_DECLARE_METATYPE(CardsMoveOneTimeStar)
Q_DECLARE_METATYPE(CardMoveStruct)
Q_DECLARE_METATYPE(CardMoveStar)
Q_DECLARE_METATYPE(CardStar)
Q_DECLARE_METATYPE(PlayerStar)
Q_DECLARE_METATYPE(DyingStruct)
Q_DECLARE_METATYPE(RecoverStruct)
Q_DECLARE_METATYPE(JudgeStar)
Q_DECLARE_METATYPE(DamageStar)
Q_DECLARE_METATYPE(PindianStar)
Q_DECLARE_METATYPE(PhaseChangeStruct)
#endif // STRUCTS_H
