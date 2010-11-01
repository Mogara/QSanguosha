#include "ai.h"
#include "serverplayer.h"
#include "engine.h"
#include "standard.h"
#include "settings.h"

AI::AI(ServerPlayer *player)
    :self(player)
{
    room = player->getRoom();
}

AI *AI::CreateAI(ServerPlayer *player){
    switch(Config.AILevel){
    case 0: return new TrustAI(player);
    case 1: return new SmartAI(player);
    case 2:
    default:
        AI *ai = Sanguosha->cloneAI(player);
        if(ai == NULL)
            return new SmartAI(player);
        else
            return ai;
    }
}

AI::Relation AI::relationTo(const ServerPlayer *other) const{
    static QMap<Player::Role, int> group_map;
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
        return Neutrality;
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
        case EquipCard::WeaponLocation: return !self->getWeapon();
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

QList<int> TrustAI::askForDiscard(int discard_num, bool optional, bool include_equip, Card::Suit suit) {
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

int TrustAI::askForAG(const QList<int> &card_ids) {
    int r = qrand() % card_ids.length();
    return card_ids.at(r);
}

int TrustAI::askForCardShow(ServerPlayer *) {
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

ServerPlayer *TrustAI::askForPlayerChosen(const QList<ServerPlayer *> &targets) {
    int r = qrand() % targets.length();
    return targets.at(r);
}

const Card *TrustAI::askForSinglePeach(ServerPlayer *dying) {
    if(dying == self){
        QList<const Card *> cards = self->getHandcards();
        foreach(const Card *card, cards){
            if(card->inherits("Peach") || card->inherits("Analeptic"))
                return card;
        }
    }

    return NULL;
}

SmartAI::SmartAI(ServerPlayer *player, bool always_invoke)
    :TrustAI(player), always_invoke(always_invoke)
{

}

int SmartAI::askForCardShow(ServerPlayer *requestor) {
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
                return card->getId();
        }
    }

    return TrustAI::askForCardShow(requestor);
}

bool SmartAI::askForSkillInvoke(const QString &skill_name, const QVariant &data) {
    return always_invoke;
}

void SmartAI::activate(CardUseStruct &card_use) {
    QList<const Card *> cards = self->getHandcards();
    foreach(const Card *card, cards){
        if(card->targetFixed()){
            if(useCard(card)){
                card_use.card = card;
                card_use.from = self;

                return;
            }
        }else{
            useCard(card, card_use);
        }
    }
}

bool SmartAI::useCard(const Card *card){
    return TrustAI::useCard(card);
}

void SmartAI::useCard(const Card *card, CardUseStruct &card_use){
    // dummy;
}
