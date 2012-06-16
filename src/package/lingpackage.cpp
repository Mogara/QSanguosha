#include "lingpackage.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"


LuoyiCard::LuoyiCard(){
    once = true;
    will_throw = true;
}

bool LuoyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select == Self ;
}

void LuoyiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    if(room->askForCard(source, ".Equip", "@luoyi-discard", QVariant(), CardDiscarded))
        source->setFlags("luoyi");
}

class NeoLuoyi: public ZeroCardViewAsSkill{
public:
    NeoLuoyi():ZeroCardViewAsSkill("neoluoyi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("LuoyiCard") && !player->isNude();
    }

    virtual const Card *viewAs() const{
        return new LuoyiCard;
    }
};

NeoFanjianCard::NeoFanjianCard(){
    once = true;
}

void NeoFanjianCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhouyu = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = zhouyu->getRoom();

    int card_id = room->askForCardChosen(zhouyu, zhouyu, "h", "neofanjian");
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target, "neofanjian");

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = target;
    log.arg = Card::Suit2String(suit);
    room->sendLog(log);

    room->getThread()->delay();
    target->obtainCard(card);
    room->showCard(target, card_id);

    if(card->getSuit() != suit){
        DamageStruct damage;
        damage.card = NULL;
        damage.from = zhouyu;
        damage.to = target;

        room->damage(damage);
    }
}

class NeoFanjian:public ZeroCardViewAsSkill{
public:
    NeoFanjian():ZeroCardViewAsSkill("neofanjian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && ! player->hasUsed("NeoFanjianCard");
    }

    virtual const Card *viewAs() const{
        return new NeoFanjianCard;
    }
};

class Zhongyi: public TriggerSkill{
public:
    Zhongyi():TriggerSkill("zhongyi"){
        events << DamageCaused;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->inherits("Slash") && damage.card->getSuit() == Card::Heart &&
           !damage.chain && !damage.to->isAllNude() && player->askForSkillInvoke(objectName(), data)){

            LogMessage log;
            log.type = "#Zhongyi";
            log.from = player;
            log.arg = objectName();
            log.to << damage.to;
            room->sendLog(log);
            int card_id = room->askForCardChosen(player, damage.to, "hej", objectName());
            CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, player->objectName());
            room->obtainCard(player, Sanguosha->getCard(card_id), reason, room->getCardPlace(card_id) != Player::Hand);
            return true;
        }
        return false;
    }
};

class Zhulou:public PhaseChangeSkill{
public:
    Zhulou():PhaseChangeSkill("zhulou"){
    }

    virtual bool onPhaseChange(ServerPlayer *gongsun) const{
        Room *room = gongsun->getRoom();
        if(gongsun->getPhase() == Player::Finish && gongsun->askForSkillInvoke(objectName())){
            gongsun->drawCards(2);
            QString choice = room->askForChoice(gongsun, "zhulou", "throw+losehp");
            if(choice == "losehp" || !room->askForCard(gongsun, ".Weapon", "@zhulou-discard", QVariant(), CardDiscarded))
                room->loseHp(gongsun);
        }
        return false;
    }
};

class Tannang: public DistanceSkill{
public:
    Tannang():DistanceSkill("tannang")
    {
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(from->hasSkill(objectName()) && !from->loseDistanceSkills())
            return -from->getLostHp();
        else
            return 0;
    }
};

class NeoJushou: public PhaseChangeSkill{
public:
    NeoJushou():PhaseChangeSkill("neojushou"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            if(room->askForSkillInvoke(target, objectName())){
                target->drawCards(2+target->getLostHp());
                target->turnOver();

                room->playSkillEffect(objectName());
            }
        }

        return false;
    }
};

class NeoGanglie:public MasochismSkill{
public:
    NeoGanglie():MasochismSkill("neoganglie"){

    }

    virtual void onDamaged(ServerPlayer *xiahou, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = xiahou->getRoom();
        QVariant source = QVariant::fromValue(from);

        if(from && from->isAlive() && room->askForSkillInvoke(xiahou, "ganglie", source)){
            room->playSkillEffect(objectName());

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = xiahou;

            room->judge(judge);
            if(judge.isGood()){
                QStringList choicelist;
                choicelist << "damage";
                if (from->getHandcardNum() > 1)
                    choicelist << "throw";
                QString choice = room->askForChoice(xiahou, "neoganglie", choicelist.join("+"));
                if(choice == "damage"){
                    DamageStruct damage;
                    damage.from = xiahou;
                    damage.to = from;

                    room->setEmotion(xiahou, "good");
                    room->damage(damage);
                }
                else
                     room->askForDiscard(from, objectName(), 2, 2);
            }else
                room->setEmotion(xiahou, "bad");
        }
    }
};

LingPackage::LingPackage()
    :Package("ling")
{

    General * neo_xuchu = new General(this, "neo_xuchu", "wei");
    neo_xuchu->addSkill(new NeoLuoyi);
    neo_xuchu->addSkill("#luoyi");

    General * neo_zhouyu = new General(this, "neo_zhouyu", "wu", 3);
    neo_zhouyu->addSkill(new NeoFanjian);
    neo_zhouyu->addSkill("yingzi");

    General * neo_guanyu = new General(this, "neo_guanyu", "shu");
    neo_guanyu->addSkill(new Zhongyi);
    neo_guanyu->addSkill("wusheng");

    General * neo_gongsunzan = new General(this, "neo_gongsunzan", "qun");
    neo_gongsunzan->addSkill(new Zhulou);
    neo_gongsunzan->addSkill("yicong");

    General * neo_zhangfei = new General(this, "neo_zhangfei", "shu");
    neo_zhangfei->addSkill("paoxiao");
    neo_zhangfei->addSkill(new Tannang);

    General * neo_zhaoyun = new General(this, "neo_zhaoyun", "shu");
    neo_zhaoyun->addSkill("longdan");
    neo_zhaoyun->addSkill("yicong");

    General * neo_caoren = new General(this, "neo_caoren", "wei");
    neo_caoren->addSkill(new NeoJushou);

    General * neo_xiahoudun = new General(this, "neo_xiahoudun", "wei");
    neo_xiahoudun->addSkill(new NeoGanglie);

    addMetaObject<LuoyiCard>();
    addMetaObject<NeoFanjianCard>();
}

ADD_PACKAGE(Ling)
