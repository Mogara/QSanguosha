#include "ai.h"
#include "serverplayer.h"
#include "engine.h"
#include "standard.h"
#include "settings.h"
#include "maneuvering.h"
#include "lua.hpp"
#include "scenario.h"
#include "aux-skills.h"

AI::AI(ServerPlayer *player)
    :self(player)
{
    room = player->getRoom();
}

typedef QPair<QString, QString> RolePair;

struct RoleMapping: public QMap<RolePair, AI::Relation> {
    void set(const QString &role1, const QString &role2, AI::Relation relation, bool reverse = false){
        insert(qMakePair(role1, role2), relation);
        if(reverse)
            insert(qMakePair(role2, role1), relation);
    }

    AI::Relation get(const QString &role1, const QString &role2){
        return value(qMakePair(role1, role2), AI::Neutrality);
    }
};

AI::Relation AI::GetRelation3v3(const ServerPlayer *a, const ServerPlayer *b){
    QChar c = a->getRole().at(0);
    if(b->getRole().startsWith(c))
        return Friend;
    else
        return Enemy;
}

AI::Relation AI::GetRelationHegemony(const ServerPlayer *a, const ServerPlayer *b){
    const bool aShown = a->getRoom()->getTag(a->objectName()).toStringList().isEmpty();
    const bool bShown = b->getRoom()->getTag(b->objectName()).toStringList().isEmpty();

    const QString aName = aShown ?
                a->getGeneralName() :
                a->getRoom()->getTag(a->objectName()).toStringList().first();
    const QString bName = bShown ?
                b->getGeneralName() :
                b->getRoom()->getTag(b->objectName()).toStringList().first();

    const QString aKingdom = Sanguosha->getGeneral(aName)->getKingdom();
    const QString bKingdom = Sanguosha->getGeneral(bName)->getKingdom();


    qDebug() << aKingdom << bKingdom <<aShown << bShown;

    return aKingdom == bKingdom ? Friend :Enemy;
}

AI::Relation AI::GetRelation(const ServerPlayer *a, const ServerPlayer *b){
    RoleMapping map, map_good, map_bad;
    if(map.isEmpty()){
        map.set("lord", "lord", Friend);
        map.set("lord", "rebel", Enemy);
        map.set("lord", "loyalist", Friend);
        map.set("lord", "renegade", Neutrality);

        map.set("loyalist", "loyalist", Friend);
        map.set("loyalist", "lord", Friend);
        map.set("loyalist", "rebel", Enemy);
        map.set("loyalist", "renegade", Neutrality);

        map.set("rebel", "rebel", Friend);
        map.set("rebel", "lord", Enemy);
        map.set("rebel", "loyalist", Enemy);
        map.set("rebel", "renegade", Neutrality);

        map.set("renegade", "lord", Friend);
        map.set("renegade", "loyalist", Neutrality);
        map.set("renegade", "rebel", Neutrality);
        map.set("renegade", "renegade", Neutrality);

        map_good = map;
        map_good.set("renegade", "loyalist", Enemy, true);
        map_good.set("renegade", "rebel", Friend, true);

        map_bad = map;
        map_bad.set("renegade", "loyalist", Friend, true);
        map_bad.set("renegade", "rebel", Enemy, true);
    }

    if(a->aliveCount() == 2){
        return Enemy;
    }

    Room *room = a->getRoom();
    QString process = room->getTag("GameProcess").toString();
    if(process == "Balance")
        return map.get(a->getRole(), b->getRole());
    else if(process == "LordSuperior")
        return map_good.get(a->getRole(), b->getRole());
    else
        return map_bad.get(a->getRole(), b->getRole());
}

AI::Relation AI::relationTo(const ServerPlayer *other) const{
    if(self == other)
        return Friend;

    const Scenario *scenario = room->getScenario();
    if(scenario)
        return scenario->relationTo(self, other);

    if(room->getMode() == "06_3v3")
        return GetRelation3v3(self, other);
    else if(Config.EnableHegemony)
        return GetRelationHegemony(self, other);

    return GetRelation(self, other);
}

bool AI::isFriend(const ServerPlayer *other) const{
    return relationTo(other) == Friend;
}

bool AI::isEnemy(const ServerPlayer *other) const{
    return relationTo(other) == Enemy;
}

QList<ServerPlayer *> AI::getEnemies() const{
    QList<ServerPlayer *> players = room->getOtherPlayers(self);
    QList<ServerPlayer *> enemies;
    foreach(ServerPlayer *p, players){
        if(isEnemy(p))
            enemies << p;
    }

    return enemies;
}

QList<ServerPlayer *> AI::getFriends() const{
    QList<ServerPlayer *> players = room->getOtherPlayers(self);
    QList<ServerPlayer *> friends;
    foreach(ServerPlayer *p, players){
        if(isFriend(p))
            friends << p;
    }

    return friends;
}

void AI::filterEvent(TriggerEvent , ServerPlayer *, const QVariant &){
    // dummy
}

TrustAI::TrustAI(ServerPlayer *player)
    :AI(player)
{
    response_skill = new ResponseSkill;
    response_skill->setParent(this);
}

void TrustAI::activate(CardUseStruct &card_use){
    QList<const Card *> cards = self->getHandcards();
    foreach(const Card *card, cards){
        if(card->targetFixed()){
            if(useCard(card)){
                card_use.card = card;
                card_use.from = self;

                return;
            }
        }
    }
}

bool TrustAI::useCard(const Card *card){
    if(card->inherits("Peach"))
        return self->isWounded();
    else if(card->inherits("EquipCard")){
        const EquipCard *equip = qobject_cast<const EquipCard *>(card);
        switch(equip->location()){
        case EquipCard::WeaponLocation:{
                const Weapon *weapon = self->getWeapon();
                if(weapon == NULL)
                    return true;

                const Weapon *new_weapon = qobject_cast<const Weapon *>(equip);
                return new_weapon->getRange() > weapon->getRange();
            }
        case EquipCard::ArmorLocation: return !self->getArmor();
        case EquipCard::OffensiveHorseLocation: return !self->getOffensiveHorse();
        case EquipCard::DefensiveHorseLocation: return !self->getDefensiveHorse();
        default:
            return true;
        }

    }else if(card->inherits("ExNihilo"))
        return true;
    else
        return false;
}

Card::Suit TrustAI::askForSuit(const QString &){
    return Card::AllSuits[qrand() % 4];
}

QString TrustAI::askForKingdom(){
    QString role;
    switch(self->getRoleEnum()){
    case Player::Lord:
    case Player::Rebel: role = "wei"; break;
    case Player::Loyalist:
    case Player::Renegade:
        role = room->getLord()->getKingdom(); break;
    }

    return role;
}

bool TrustAI::askForSkillInvoke(const QString &, const QVariant &){
    return false;
}

QString TrustAI::askForChoice(const QString &skill_name, const QString &choice){
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if(skill){
        QString default_choice = skill->getDefaultChoice(self);
        if(choice.contains(default_choice))
            return default_choice;
    }

    QStringList choices = choice.split("+");
    return choices.at(qrand() % choices.length());
}

QList<int> TrustAI::askForDiscard(const QString &, int discard_num, int min_num, bool optional, bool include_equip){
    QList<int> to_discard;

    if(optional)
        return to_discard;
    else
        return self->forceToDiscard(discard_num, include_equip);
}

const Card *TrustAI::askForNullification(const TrickCard *trick, ServerPlayer *, ServerPlayer *to, bool positive){
    if(self == to && trick->isAggressive() && positive){
        QList<const Card *> cards = self->getHandcards();

        foreach(const Card *card, cards){
            if(card->inherits("Nullification"))
                return card;
        }

        if(self->hasSkill("kanpo")){
            foreach(const Card *card, cards){
                if(card->isBlack()){
                    Nullification *ncard = new Nullification(card->getSuit(), card->getNumber());
                    ncard->addSubcard(card);
                    ncard->setSkillName("kanpo");

                    return ncard;
                }
            }
        }
    }

    return NULL;
}

int TrustAI::askForCardChosen(ServerPlayer *who, const QString &flags, const QString &) {
    QList<const Card *> cards = who->getCards(flags);
    int r = qrand() % cards.length();
    return cards.at(r)->getId();
}

const Card *TrustAI::askForCard(const QString &pattern, const QString &prompt, const QVariant &data){
    Q_UNUSED(prompt);
    Q_UNUSED(data);

    response_skill->setPattern(pattern);
    QList<const Card *> cards = self->getHandcards();
    foreach(const Card *card, cards){
        if(response_skill->matchPattern(self, card))
            return card;
    }

    return NULL;
}

QString TrustAI::askForUseCard(const QString &, const QString &) {
    return ".";
}

int TrustAI::askForAG(const QList<int> &card_ids, bool refusable, const QString &){
    if(refusable)
        return -1;

    int r = qrand() % card_ids.length();
    return card_ids.at(r);
}

const Card *TrustAI::askForCardShow(ServerPlayer *, const QString &){
    return self->getRandomHandCard();
}

static bool CompareByNumber(const Card *c1, const Card *c2){
    return c1->getNumber() < c2->getNumber();
}

const Card *TrustAI::askForPindian(ServerPlayer *requestor, const QString &reason){
    QList<const Card *> cards = self->getHandcards();
    qSort(cards.begin(), cards.end(), CompareByNumber);

    // zhiba special case
    if(reason == "zhiba" && self->hasLordSkill("sunce_zhiba"))
        return cards.last();

    if(requestor != self && isFriend(requestor))
        return cards.first();
    else
        return cards.last();
}

ServerPlayer *TrustAI::askForPlayerChosen(const QList<ServerPlayer *> &targets, const QString &reason){
    Q_UNUSED(reason);

    int r = qrand() % targets.length();
    return targets.at(r);
}

const Card *TrustAI::askForSinglePeach(ServerPlayer *dying) {
    if(isFriend(dying)){
        QList<const Card *> cards = self->getHandcards();
        foreach(const Card *card, cards){
            if(card->inherits("Peach"))
                return card;

            if(card->inherits("Analeptic") && dying == self)
                return card;
        }

        if(dying == self){
            if(self->hasSkill("jiuchi")){
                foreach(const Card *card, cards){
                    if(card->getSuit() == Card::Spade){
                        Analeptic *analeptic = new Analeptic(Card::Spade, card->getNumber());
                        analeptic->addSubcard(card);
                        analeptic->setSkillName("jiuchi");
                        return analeptic;
                    }
                }
            }

            if(self->hasSkill("jiushi") && self->faceUp()){
                Analeptic *analeptic = new Analeptic(Card::NoSuit, 0);
                analeptic->setSkillName("jiushi");
                return analeptic;
            }
        }

        if(self->hasSkill("jijiu") && self->getPhase() == Player::NotActive){
            cards = self->getCards("he");
            foreach(const Card *card, cards){
                if(card->isRed()){
                    Peach *peach = new Peach(card->getSuit(), card->getNumber());
                    peach->addSubcard(card);
                    peach->setSkillName("jijiu");
                    return peach;
                }
            }
        }
    }

    return NULL;
}

ServerPlayer *TrustAI::askForYiji(const QList<int> &, int &){
    return NULL;
}

void TrustAI::askForGuanxing(const QList<int> &cards, QList<int> &up, QList<int> &bottom, bool up_only){
    Q_UNUSED(bottom);
    Q_UNUSED(up_only);

    up = cards;
    bottom.clear();
}

LuaAI::LuaAI(ServerPlayer *player)
    :TrustAI(player), callback(0)
{

}


QString LuaAI::askForUseCard(const QString &pattern, const QString &prompt){
    if(callback == 0)
        return TrustAI::askForUseCard(pattern, prompt);

    lua_State *L = room->getLuaState();

    pushCallback(L, __FUNCTION__);
    lua_pushstring(L, pattern.toAscii());
    lua_pushstring(L, prompt.toAscii());

    int error = lua_pcall(L, 3, 1, 0);
    const char *result = lua_tostring(L, -1);
    lua_pop(L, 1);

    if(error){
        const char *error_msg = result;
        room->output(error_msg);
        return ".";
    }

    return result;
}

QList<int> LuaAI::askForDiscard(const QString &reason, int discard_num, int min_num, bool optional, bool include_equip){
    lua_State *L = room->getLuaState();

    pushCallback(L, __FUNCTION__);
    lua_pushstring(L, reason.toAscii());
    lua_pushinteger(L, discard_num);
    lua_pushinteger(L, min_num);
    lua_pushboolean(L, optional);
    lua_pushboolean(L, include_equip);

    int error = lua_pcall(L, 6, 1, 0);
    if(error){
        reportError(L);
        return TrustAI::askForDiscard(reason, discard_num, min_num, optional, include_equip);
    }

    QList<int> result;
    if(getTable(L, result))
        return result;
    else
        return TrustAI::askForDiscard(reason, discard_num, min_num, optional, include_equip);
}

bool LuaAI::getTable(lua_State *L, QList<int> &table){
    if(!lua_istable(L, -1)){
        lua_pop(L, 1);
        return false;
    }

    size_t len = lua_objlen(L, -1);
    size_t i;
    for(i=0; i<len; i++){
        lua_rawgeti(L, -1, i+1);
        table << lua_tointeger(L, -1);
        lua_pop(L, 1);
    }

    lua_pop(L, 1);

    return true;
}

QString LuaAI::askForChoice(const QString &skill_name, const QString &choices){

    lua_State *L = room->getLuaState();

    pushCallback(L, __FUNCTION__);
    lua_pushstring(L, skill_name.toAscii());
    lua_pushstring(L, choices.toAscii());

    int error = lua_pcall(L, 3, 1, 0);
    const char *result = lua_tostring(L, -1);
    lua_pop(L, 1);
    if(error){
        room->output(result);
        return TrustAI::askForChoice(skill_name, choices);
    }

    return result;
}

int LuaAI::askForAG(const QList<int> &card_ids, bool refusable, const QString &reason){
    lua_State *L = room->getLuaState();

    pushCallback(L, __FUNCTION__);
    pushQIntList(L, card_ids);
    lua_pushboolean(L, refusable);
    lua_pushstring(L, reason.toAscii());

    int error = lua_pcall(L, 4, 1, 0);
    if(error){
        reportError(L);
        return TrustAI::askForAG(card_ids, refusable, reason);
    }

    int card_id = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return card_id;
}

void LuaAI::pushCallback(lua_State *L, const char *function_name){
    Q_ASSERT(callback);

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);
    lua_pushstring(L, function_name);
}

void LuaAI::pushQIntList(lua_State *L, const QList<int> &list){
    lua_createtable(L, list.length(), 0);

    int i;
    for(i=0; i<list.length(); i++){
        lua_pushinteger(L, list.at(i));
        lua_rawseti(L, -2, i+1);
    }
}

void LuaAI::reportError(lua_State *L){
    const char *error_msg = lua_tostring(L, -1);
    room->output(error_msg);
    lua_pop(L, 1);
}

void LuaAI::askForGuanxing(const QList<int> &cards, QList<int> &up, QList<int> &bottom, bool up_only){
    lua_State *L = room->getLuaState();

    pushCallback(L, __FUNCTION__);
    pushQIntList(L, cards);
    lua_pushboolean(L, up_only);

    int error = lua_pcall(L, 3, 2, 0);
    if(error){
        reportError(L);
        return TrustAI::askForGuanxing(cards, up, bottom, up_only);
    }

    getTable(L, bottom);
    getTable(L, up);
}
