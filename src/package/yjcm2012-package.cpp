#include "yjcm2012-package.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"

ZhenlieCard::ZhenlieCard(){
    target_fixed = true;
    will_throw = false;
}

void ZhenlieCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->showCard(source, getSubcards().first());
}

class ZhenlieViewAsSkill:public OneCardViewAsSkill{
public:
    ZhenlieViewAsSkill():OneCardViewAsSkill(""){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@zhenlie";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        int card_id = Self->getMark("zhenliecard");
        const Card *card = Sanguosha->getCard(card_id);
        return to_select->getFilteredCard()->getSuit() != card->getSuit();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ZhenlieCard *card = new ZhenlieCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Zhenlie: public TriggerSkill{
public:
    Zhenlie():TriggerSkill("zhenlie"){
        events << AskForRetrial;
        view_as_skill = new ZhenlieViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if(!TriggerSkill::triggerable(target))
            return false;
        return !target->isKongcheng();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@zhenlie-card" << judge->who->objectName()
                << "" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");
        room->setPlayerMark(player, "zhenliecard", judge->card->getEffectiveId());
        const Card *card = room->askForCard(player, "@zhenlie", prompt, data);

        if(card){
            int card_id = room->drawCard();
            room->getThread()->delay();

            judge->card = Sanguosha->getCard(card_id);
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

class Miji: public PhaseChangeSkill{
public:
    Miji():PhaseChangeSkill("miji"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *wangyi) const{
        if(!wangyi->isWounded())
            return false;
        if(wangyi->getPhase() == Player::Start || wangyi->getPhase() == Player::Finish){
            if(!wangyi->askForSkillInvoke(objectName()))
                return false;
            Room *room = wangyi->getRoom();
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club|spade):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = wangyi;

            room->judge(judge);

            if(judge.isGood()){
                room->playSkillEffect(objectName());
                int x = wangyi->getLostHp();
                wangyi->drawCards(x);
                ServerPlayer *target = room->askForPlayerChosen(wangyi, room->getAllPlayers(), objectName());

                QList<const Card *> miji_cards = wangyi->getHandcards().mid(wangyi->getHandcardNum() - x);
                foreach(const Card *card, miji_cards)
                    room->moveCardTo(card, target, Player::Hand, false);
            }
        }
        return false;
    }
};

class Fuji: public TriggerSkill{
public:
    Fuji():TriggerSkill("fuji"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();

        if(player->distanceTo(damage.to) == 1 && damage.card && damage.card->inherits("Slash")){
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.to << damage.to;
            log.arg = objectName();
            room->sendLog(log);
            room->loseMaxHp(damage.to, damage.damage);
            return true;
        }
        return false;
    }
};

YJCM2012Package::YJCM2012Package():Package("YJCM2012"){

    General *wangyi = new General(this, "wangyi", "wei", 3, false);
    wangyi->addSkill(new Zhenlie);
    wangyi->addSkill(new Miji);
/*
    General *xunyou = new General(this, "xunyou", "wei", 3);

    General *caozhang = new General(this, "caozhang", "wei");
*/
    General *madai = new General(this, "madai", "shu");
    madai->addSkill(new Fuji);
/*
    General *liaohua = new General(this, "liaohua", "shu");

    General *guanxingzhangbao = new General(this, "guanxingzhangbao", "shu");

    General *chengpu = new General(this, "chengpu", "wu");

    General *bulianshi = new General(this, "bulianshi", "wu", 3, false);

    General *handang = new General(this, "handang", "qun");

    General *liubiao = new General(this, "liubiao", "qun", 3);
*/
    addMetaObject<ZhenlieCard>();
}

ADD_PACKAGE(YJCM2012)
