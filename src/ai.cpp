#include "ai.h"
#include "serverplayer.h"
#include "engine.h"
#include "standard.h"

AI::AI(ServerPlayer *player)
    :player(player)
{
    room = player->getRoom();
}

TrustAI::TrustAI(ServerPlayer *player)
    :AI(player)
{
}

void TrustAI::activate(CardUseStruct &card_use) const{
    QList<const Card *> cards = player->getHandcards();
    foreach(const Card *card, cards){
        bool use_it = false;

        if(card->inherits("Peach"))
            use_it = player->isWounded();
        else if(card->inherits("EquipCard")){
            const EquipCard *equip = qobject_cast<const EquipCard *>(card);
            switch(equip->location()){
            case EquipCard::WeaponLocation: use_it = !player->getWeapon(); break;
            case EquipCard::ArmorLocation: use_it = !player->getArmor(); break;
            case EquipCard::OffensiveHorseLocation: use_it = !player->getOffensiveHorse(); break;
            case EquipCard::DefensiveHorseLocation: use_it = !player->getDefensiveHorse(); break;
            }
        }else if(card->inherits("TrickCard"))
            use_it = card->targetFixed();

        if(use_it){
            card_use.card = card;
            card_use.from = player;

            return;
        }
    }
}

Card::Suit TrustAI::askForSuit() const{
    return Card::AllSuits[qrand() % 4];
}

QString TrustAI::askForKingdom() const{
    return player->getKingdom();
}

bool TrustAI::askForSkillInvoke(const QString &skill_name) const{
    return false;
}

QString TrustAI::askForChoice(const QString &skill_name, const QString &){
    const Skill *skill = Sanguosha->getSkill(skill_name);
    const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
    return trigger_skill->getDefaultChoice();
}

QList<int> TrustAI::askForDiscard(int discard_num, bool optional, bool include_equip, Card::Suit suit) const{
    QList<int> to_discard;

    if(optional)
        return to_discard;
    else
        return player->forceToDiscard(discard_num, include_equip);
}

int TrustAI::askForNullification(const QString &trick_name, ServerPlayer *from, ServerPlayer *to) const{
    const TrickCard *card = Sanguosha->findChild<const TrickCard *>(trick_name);
    if(to == player && card->isAggressive()){
        QList<const Card *> cards = player->getHandcards();
        if(player->hasSkill("kanpo")){
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

int TrustAI::askForCardChosen(ServerPlayer *who, const QString &flags, const QString &) const{
    QList<const Card *> cards = who->getCards(flags);
    int r = qrand() % cards.length();
    return cards.at(r)->getId();
}

const Card *TrustAI::askForCard(const QString &pattern) const{
    static QRegExp id_rx("\\d+");
    if(pattern.contains("+")){
        QStringList subpatterns = pattern.split("+");
        foreach(QString subpattern, subpatterns){
            const Card *result = askForCard(subpattern);
            if(result)
                return result;
        }
    }

    QList<const Card *> cards = player->getHandcards();
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

QString TrustAI::askForUseCard(const QString &, const QString &) const{
    return ".";
}

int TrustAI::askForAG(const QList<int> &card_ids) const{
    int r = qrand() % card_ids.length();
    return card_ids.at(r);
}

int TrustAI::askForCardShow(ServerPlayer *) const{
    return player->getRandomHandCard();
}

const Card *TrustAI::askForPindian() const{
    QList<const Card *> cards = player->getHandcards();
    const Card *highest = cards.first();
    foreach(const Card *card, cards){
        if(card->getNumber() > highest->getNumber())
            highest = card;
    }

    return highest;
}

ServerPlayer *TrustAI::askForPlayerChosen(const QList<ServerPlayer *> &targets) const{
    int r = qrand() % targets.length();
    return targets.at(r);
}

const Card *TrustAI::askForSinglePeach(ServerPlayer *dying) const{
    if(dying == player){
        QList<const Card *> cards = player->getHandcards();
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

int SmartAI::askForCardShow(ServerPlayer *requestor) const{
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

    cards = player->getHandcards();
    if(lack != Card::NoSuit){
        foreach(const Card *card, cards){
            if(card->getSuit() == lack)
                return card->getId();
        }
    }

    return TrustAI::askForCardShow(requestor);
}

bool SmartAI::askForSkillInvoke(const QString &skill_name) const{
    return always_invoke;
}
