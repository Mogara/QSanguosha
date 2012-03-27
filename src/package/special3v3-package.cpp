#include "special3v3-package.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class Hongyuan:public DrawCardsSkill{
public:
    Hongyuan():DrawCardsSkill("hongyuan"){

    }

    QList<ServerPlayer *> getTeammates(ServerPlayer *zhugejin) const{
        Room *room = zhugejin->getRoom();

        QList<ServerPlayer *> teammates;
        foreach(ServerPlayer *other, room->getOtherPlayers(zhugejin)){
            if(AI::GetRelation3v3(zhugejin, other) == AI::Friend)
                teammates << other;
        }
        return teammates;
    }

    virtual int getDrawNum(ServerPlayer *zhugejin, int n) const{
        Room *room = zhugejin->getRoom();        
        if(ServerInfo.GameMode != "06_3v3")
            return n;
        if(room->askForSkillInvoke(zhugejin, objectName())){
            room->playSkillEffect(objectName());
            foreach(ServerPlayer *teammate, getTeammates(zhugejin)){
                if(teammate->objectName() != zhugejin->objectName())
                    teammate->drawCards(1);
            }
            return n - 1;
        }else
            return n;
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

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
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
            room->throwCard(judge->card);

            judge->card = Sanguosha->getCard(card->getEffectiveId());
            room->moveCardTo(judge->card, NULL, Player::Special);

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

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(ServerInfo.GameMode != "06_3v3" || player->getPhase() != Player::NotActive)
            return false;
        bool can_invoke = false;
        int n = 0;
        const Card *card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed){
            CardStar card_star = data.value<CardStar>();
            card = card_star;
        }else if(event == CardDiscarded){
            const Card *cd = data.value<CardStar>();
            card = cd;
        }
        if(card){
            if(card->isVirtualCard()){
                foreach(int id, card->getSubcards()){
                    const Card *cd = Sanguosha->getCard(id);
                    if(cd->isRed()){
                        can_invoke = true;
                        n++;
                    }
                }
            }else if(card->isRed()){
                can_invoke = true;
                n++;
            }

            if(can_invoke && player->askForSkillInvoke(objectName())){
                player->drawCards(n);
            }
        }
        return false;
    }
};

Special3v3Package::Special3v3Package():Package("Special3v3")
{
    General *zhugejin = new General(this, "zhugejin", "wu", 3, true);
    zhugejin->addSkill(new Hongyuan);
    zhugejin->addSkill(new Huanshi);
    zhugejin->addSkill(new Mingzhe);

    addMetaObject<HuanshiCard>();
}

ADD_PACKAGE(Special3v3)
