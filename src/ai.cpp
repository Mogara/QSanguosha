#include "ai.h"
#include "serverplayer.h"
#include "engine.h"
#include "standard.h"
#include "settings.h"
#include "maneuvering.h"

extern "C"{

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

}

AI::AI(ServerPlayer *player)
    :self(player)
{
    room = player->getRoom();
}

AI::Relation AI::relationTo(const ServerPlayer *other) const{
    /*static QMap<Player::Role, int> group_map;
    if(group_map.isEmpty()){
        group_map.insert(Player::Lord, 1);
        group_map.insert(Player::Loyalist, 1);
        group_map.insert(Player::Rebel, -1);
        group_map.insert(Player::Renegade, 0);
    }

    Player::Role self_role = self->getRoleEnum();
    Player::Role other_role = other->getRoleEnum();

    int self_group = group_map.value(self_role);
    int other_group = group_map.value(other_role);

    if(self_group == other_group)
        return Friend;
    else if(self_group + other_group == 0)
        return Enemy;
    else if(room->getTag("GameProcess").toString() == "ZN")
        return Enemy;
    else
        return Neutrality;*/

    if(self == other)
        return Friend;

    typedef QPair<Player::Role, Player::Role> RolePair;
    static QMap<RolePair, Relation> map;
    if(map.isEmpty()){
        map[qMakePair(Player::Lord, Player::Lord)] = Friend;
        map[qMakePair(Player::Lord, Player::Rebel)] = Enemy;
        map[qMakePair(Player::Lord, Player::Loyalist)] = Friend;
        map[qMakePair(Player::Lord, Player::Renegade)] = Neutrality;

        map[qMakePair(Player::Loyalist, Player::Loyalist)] = Friend;
        map[qMakePair(Player::Loyalist, Player::Lord)] = Friend;
        map[qMakePair(Player::Loyalist, Player::Rebel)] = Enemy;
        map[qMakePair(Player::Loyalist, Player::Renegade)] =Neutrality;

        map[qMakePair(Player::Rebel, Player::Rebel)] = Friend;
        map[qMakePair(Player::Rebel, Player::Lord)] = Enemy;
        map[qMakePair(Player::Rebel, Player::Loyalist)] = Enemy;
        map[qMakePair(Player::Rebel, Player::Renegade)] =Neutrality;

        map[qMakePair(Player::Renegade, Player::Lord)] = Neutrality;
        map[qMakePair(Player::Renegade, Player::Loyalist)] = Neutrality;
        map[qMakePair(Player::Renegade, Player::Rebel)] = Neutrality;
        map[qMakePair(Player::Renegade, Player::Renegade)] =Neutrality;
    }

    RolePair pair(self->getRoleEnum(), other->getRoleEnum());

    return map.value(pair, Neutrality);
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

void AI::filterEvent(TriggerEvent event, ServerPlayer *player, const QVariant &data){
    // dummy
}

TrustAI::TrustAI(ServerPlayer *player)
    :AI(player)
{
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

    }else if(card->inherits("TrickCard"))
        return true;
    else
        return false;
}

Card::Suit TrustAI::askForSuit(){
    return Card::AllSuits[qrand() % 4];
}

QString TrustAI::askForKingdom(){
    return self->getKingdom();
}

bool TrustAI::askForSkillInvoke(const QString &skill_name, const QVariant &data){
    return false;
}

QString TrustAI::askForChoice(const QString &skill_name, const QString &){
    const Skill *skill = Sanguosha->getSkill(skill_name);
    return skill->getDefaultChoice();
}

QList<int> TrustAI::askForDiscard(const QString &reason, int discard_num, bool optional, bool include_equip){
    QList<int> to_discard;

    if(optional)
        return to_discard;
    else
        return self->forceToDiscard(discard_num, include_equip);
}

int TrustAI::askForNullification(const QString &trick_name, ServerPlayer *from, ServerPlayer *to) {
    const TrickCard *card = Sanguosha->findChild<const TrickCard *>(trick_name);
    if(to == self && card->isAggressive()){
        QList<const Card *> cards = self->getHandcards();
        if(self->hasSkill("kanpo")){
            foreach(const Card *card, cards){
                if(card->isBlack() || card->objectName() == "nullification"){
                    return card->getId();
                }
            }
        }else{
            foreach(const Card *card, cards){
                if(card->objectName() == "nullification")
                    return card->getId();
            }
        }
    }

    return -1;
}

int TrustAI::askForCardChosen(ServerPlayer *who, const QString &flags, const QString &) {
    QList<const Card *> cards = who->getCards(flags);
    int r = qrand() % cards.length();
    return cards.at(r)->getId();
}

const Card *TrustAI::askForCard(const QString &pattern) {
    static QRegExp id_rx("\\d+");

    if(pattern.contains("+")){
        QStringList subpatterns = pattern.split("+");
        foreach(QString subpattern, subpatterns){
            const Card *result = askForCard(subpattern);
            if(result)
                return result;
        }
    }

    QList<const Card *> cards = self->getHandcards();
    if(id_rx.exactMatch(pattern)){
        int card_id = pattern.toInt();
        foreach(const Card *card, cards)
            if(card->getId() == card_id)
                return card;
    }else{
        foreach(const Card *card, cards)
            if(card->match(pattern))
                return card;
    }

    return NULL;
}

QString TrustAI::askForUseCard(const QString &, const QString &) {
    return ".";
}

int TrustAI::askForAG(const QList<int> &card_ids, bool refsuable){
    if(refsuable)
        return -1;

    int r = qrand() % card_ids.length();
    return card_ids.at(r);
}

const Card *TrustAI::askForCardShow(ServerPlayer *) {
    return self->getRandomHandCard();
}

const Card *TrustAI::askForPindian() {
    QList<const Card *> cards = self->getHandcards();
    const Card *highest = cards.first();
    foreach(const Card *card, cards){
        if(card->getNumber() > highest->getNumber())
            highest = card;
    }

    return highest;
}

ServerPlayer *TrustAI::askForPlayerChosen(const QList<ServerPlayer *> &targets, const QString &reason){
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

        if(self->hasSkill("jiuchi") && dying == self){
            foreach(const Card *card, cards){
                if(card->getSuit() == Card::Spade){
                    Analeptic *analeptic = new Analeptic(Card::Spade, card->getNumber());
                    analeptic->addSubcard(card);
                    analeptic->setSkillName("jiuchi");
                    return analeptic;
                }
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

LuaAI::LuaAI(ServerPlayer *player)
    :TrustAI(player), callback(0)
{

}

const Card *LuaAI::askForCardShow(ServerPlayer *requestor) {
    QList<const Card *> cards = requestor->getHandcards();
    Card::Suit lack = Card::NoSuit;
    int i;
    for(i=0; i<4; i++){
        Card::Suit suit = Card::AllSuits[i];
        bool found = false;
        foreach(const Card *card, cards){
            if(card->getSuit() == suit){
                found = true;
                break;
            }
        }

        if(!found){
            lack = suit;
            break;
        }
    }

    cards = self->getHandcards();
    if(lack != Card::NoSuit){
        foreach(const Card *card, cards){
            if(card->getSuit() == lack)
                return card;
        }
    }

    return TrustAI::askForCardShow(requestor);
}

QString LuaAI::askForUseCard(const QString &pattern, const QString &prompt){
    if(callback == 0)
        return TrustAI::askForUseCard(pattern, prompt);

    lua_State *L = room->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);

    lua_pushstring(L, __func__);

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

QList<int> LuaAI::askForDiscard(const QString &reason, int discard_num, bool optional, bool include_equip){
    if(callback == 0)
        return TrustAI::askForDiscard(reason, discard_num, optional, include_equip);

    lua_State *L = room->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);

    lua_pushstring(L, __func__);

    lua_pushstring(L, reason.toAscii());

    lua_pushinteger(L, discard_num);

    lua_pushboolean(L, optional);

    lua_pushboolean(L, include_equip);

    int error = lua_pcall(L, 5, 1, 0);
    if(error){
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);
        return TrustAI::askForDiscard(reason, discard_num, optional, include_equip);
    }

    QList<int> result;

    if(lua_istable(L, -1)){
        size_t len = lua_objlen(L, -1);
        size_t i;
        for(i=0; i<len; i++){
            lua_rawgeti(L, -1, i+1);
            result << lua_tointeger(L, -1);
            lua_pop(L, 1);
        }
    }else
        result = TrustAI::askForDiscard(reason, discard_num, optional, include_equip);

    lua_pop(L, 1);

    return result;
}

QString LuaAI::askForChoice(const QString &skill_name, const QString &choices){
    Q_ASSERT(callback);

    lua_State *L = room->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);

    lua_pushstring(L, __func__);

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

const Card *LuaAI::askForCard(const QString &pattern){
    Q_ASSERT(callback);

    lua_State *L = room->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);

    lua_pushstring(L, __func__);

    lua_pushstring(L, pattern.toAscii());

    int error = lua_pcall(L, 2, 1, 0);
    const char *result = lua_tostring(L, -1);
    lua_pop(L, 1);
    if(error){
        room->output(result);
        return TrustAI::askForCard(pattern);
    }

    if(result == NULL)
        return TrustAI::askForCard(pattern);

    return Card::Parse(result);
}
