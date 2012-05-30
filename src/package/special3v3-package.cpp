#include "special3v3-package.h"
#include "skill.h"
#include "standard.h"
#include "server.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"
#include "maneuvering.h"

class Hongyuan:public DrawCardsSkill{
public:
    Hongyuan():DrawCardsSkill("hongyuan"){

    }

    QStringList getTeammateNames(ServerPlayer *zhugejin) const{
        Room *room = zhugejin->getRoom();

        QStringList names;
        foreach(ServerPlayer *other, room->getOtherPlayers(zhugejin)){
            if(AI::GetRelation3v3(zhugejin, other) == AI::Friend)
                names << other->getGeneralName();
        }
        return names;
    }

    virtual int getDrawNum(ServerPlayer *zhugejin, int n) const{
        Room *room = zhugejin->getRoom();        
        if(ServerInfo.GameMode != "06_3v3")
            return n;
        if(room->askForSkillInvoke(zhugejin, objectName())){
            room->playSkillEffect(objectName());
            QStringList names = getTeammateNames(zhugejin);
            room->setTag("HongyuanTargets", QVariant::fromValue(names));
            return n - 1;
        }else
            return n;
    }
};

class HongyuanDraw: public TriggerSkill{
public:
    HongyuanDraw():TriggerSkill("#hongyuan"){
        events << PhaseChange;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
        if(player->getPhase() != Player::Draw)
            return false;
        QStringList names = room->getTag("HongyuanTargets").toStringList();
        room->removeTag("HongyuanTargets");
        if(!names.isEmpty())
            foreach(QString name, names)
                room->findPlayer(name)->drawCards(1);
        return false;
    }
};

HuanshiCard::HuanshiCard(){
    target_fixed = true;
    will_throw = false;
    can_jilei = true;
}

void HuanshiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{

}

class HuanshiViewAsSkill:public OneCardViewAsSkill{
public:
    HuanshiViewAsSkill():OneCardViewAsSkill("huanshi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@huanshi";
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new HuanshiCard;
        card->setSuit(card_item->getFilteredCard()->getSuit());
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Huanshi: public TriggerSkill{
public:
    Huanshi():TriggerSkill("huanshi"){
        view_as_skill = new HuanshiViewAsSkill;

        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->isKongcheng();
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        if(ServerInfo.GameMode != "06_3v3" || AI::GetRelation3v3(player, judge->who) != AI::Friend)
            return false;

        QStringList prompt_list;
        prompt_list << "@huanshi-card" << judge->who->objectName()
                << objectName() << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");

        player->tag["Judge"] = data;
        const Card *card = room->askForCard(player, "@huanshi", prompt, data);

        if(card){
            // the only difference for Guicai & Guidao
            const Card* oldJudge = judge->card;
            judge->card = Sanguosha->getCard(card->getEffectiveId());
            CardsMoveStruct move1(QList<int>(), NULL, Player::DiscardPile,
                CardMoveReason(CardMoveReason::S_REASON_JUDGE, player->objectName(), "huanshi", QString()));
            move1.card_ids.append(card->getEffectiveId());
            
            CardsMoveStruct move2(QList<int>(), player, Player::Hand,
                CardMoveReason(CardMoveReason::S_REASON_OVERRIDE, player->objectName(), "huanshi", QString()));
            move2.card_ids.append(oldJudge->getEffectiveId());

            QList<CardsMoveStruct> moves;
            moves.append(move1);
            moves.append(move2);

            room->moveCardsAtomic(moves, true);   
            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);
        }

        return false;
    }
};

class Mingzhe: public TriggerSkill{
public:
    Mingzhe():TriggerSkill("mingzhe"){
        events << CardUsed << CardResponsed << CardDiscarded;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(ServerInfo.GameMode != "06_3v3" || player->getPhase() != Player::NotActive)
            return false;

        const Card *card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }
        else
            card = data.value<CardStar>();

        int n = 0;
        if(event == CardDiscarded){
            if(card->isVirtualCard()){
                foreach(int card_id, card->getSubcards()){
                    const Card *subcard = Sanguosha->getCard(card_id);
                    if(subcard->isRed())
                        n++;
                }
            }
            else if(card->isRed())
                n++;
        }
        else
            n = card->isRed() ? 1 : 0;

        if(n>0 && player->askForSkillInvoke(objectName(), data))
            player->drawCards(n);

        return false;
    }
};

Special3v3Package::Special3v3Package():Package("Special3v3")
{
    QList<Card *> cards;
    cards << new SupplyShortage(Card::Spade, 1)
          << new SupplyShortage(Card::Club, 12)
          << new Nullification(Card::Heart, 12);

    foreach(Card *card, cards)
        card->setParent(this);

    General *zhugejin = new General(this, "zhugejin", "wu", 3, true);
    zhugejin->addSkill(new Hongyuan);
    zhugejin->addSkill(new HongyuanDraw);
    zhugejin->addSkill(new Huanshi);
    zhugejin->addSkill(new Mingzhe);

    related_skills.insertMulti("hongyuan", "#hongyuan");

    addMetaObject<HuanshiCard>();
}

ADD_PACKAGE(Special3v3)
