#include "sp-package.h"
#include "general.h"
#include "skill.h"
#include "standard-skillcards.h"
#include "carditem.h"
#include "engine.h"
#include "standard.h"

class SPMoonSpearSkill: public WeaponSkill{
public:
    SPMoonSpearSkill():WeaponSkill("sp_moonspear"){
        events << CardResponsed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;

        CardStar card = NULL;
        card = data.value<CardStar>();

        if(!card || !card->isBlack())
            return false;

        Room *room = player->getRoom();
        if(!room->askForSkillInvoke(player, objectName(), data))
            return false;
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
            if(player->inMyAttackRange(tmp))
                targets << tmp;
        }
        if(targets.isEmpty()) return false;
        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
        if(!room->askForCard(target, "jink", "@moon-spear-jink")){
            DamageStruct damage;
            damage.from = player;
            damage.to = target;
            room->damage(damage);
        }
        return false;
    }
};

class SPMoonSpear: public Weapon{
public:
    SPMoonSpear(Suit suit = Card::Diamond, int number = 12)
        :Weapon(suit, number, 3){
        setObjectName("sp_moonspear");
        skill = new SPMoonSpearSkill;
    }
};

class JileiClear: public PhaseChangeSkill{
public:
    JileiClear():PhaseChangeSkill("#jilei-clear"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::NotActive){
            Room *room = target->getRoom();
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *player, players){
                if(player->hasFlag("jilei")){
                    player->jilei(".");
                    player->invoke("jilei");

                    LogMessage log;
                    log.type = "#JileiClear";
                    log.from = player;
                    room->sendLog(log);
                }
            }
        }

        return false;
    }
};


class Jilei: public TriggerSkill{
public:
    Jilei():TriggerSkill("jilei"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yangxiu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from == NULL)
           return false;

        Room *room = yangxiu->getRoom();
        if(room->askForSkillInvoke(yangxiu, objectName(), data)){
            QString choice = room->askForChoice(yangxiu, objectName(), "basic+equip+trick");
            room->playSkillEffect(objectName());

            damage.from->jilei(choice);
            damage.from->invoke("jilei", choice);
            damage.from->setFlags("jilei");

            LogMessage log;
            log.type = "#Jilei";
            log.from = yangxiu;
            log.to << damage.from;
            log.arg = choice;
            room->sendLog(log);
        }

        return false;
    }
};

class Danlao: public TriggerSkill{
public:
    Danlao():TriggerSkill("danlao"){
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.multiple && effect.card->inherits("TrickCard")){
            Room *room = player->getRoom();
            if(room->askForSkillInvoke(player, objectName(), data)){
                room->playSkillEffect(objectName());

                LogMessage log;

                log.type = "#DanlaoAvoid";
                log.from = player;
                log.arg = effect.card->objectName();

                room->sendLog(log);

                player->drawCards(1);
                return true;
            }
        }

        return false;
    }
};


class Yongsi: public TriggerSkill{
public:
    Yongsi():TriggerSkill("yongsi"){
        events << DrawNCards << PhaseChange;
        frequency = Compulsory;
    }

    int getKingdoms(ServerPlayer *yuanshu) const{
        QSet<QString> kingdom_set;
        Room *room = yuanshu->getRoom();
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            kingdom_set << p->getKingdom();
        }

        return kingdom_set.size();
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yuanshu, QVariant &data) const{
        if(event == DrawNCards){
            int x = getKingdoms(yuanshu);
            data = data.toInt() + x;

            Room *room = yuanshu->getRoom();
            LogMessage log;
            log.type = "#YongsiGood";
            log.from = yuanshu;
            log.arg = QString::number(x);
            room->sendLog(log);

            room->playSkillEffect("yongsi", x);

        }else if(event == PhaseChange && yuanshu->getPhase() == Player::Discard){
            int x = getKingdoms(yuanshu);
            int total = yuanshu->getEquips().length() + yuanshu->getHandcardNum();
            Room *room = yuanshu->getRoom();

            if(total <= x){
                yuanshu->throwAllHandCards();
                yuanshu->throwAllEquips();

                LogMessage log;
                log.type = "#YongsiWorst";
                log.from = yuanshu;
                log.arg = QString::number(total);
                room->sendLog(log);

            }else{
                room->askForDiscard(yuanshu, "yongsi", x, false, true);

                LogMessage log;
                log.type = "#YongsiBad";
                log.from = yuanshu;
                log.arg = QString::number(x);
                room->sendLog(log);
            }
        }

        return false;
    }
};

class Weidi:public ZeroCardViewAsSkill{
public:
    Weidi():ZeroCardViewAsSkill("weidi"){
        frequency = Compulsory;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasLordSkill("jijiang") && Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return Sanguosha->cloneSkillCard("JijiangCard");
    }
};


class Yicong: public DistanceSkill{
public:
    Yicong():DistanceSkill("yicong"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->hasSkill(objectName()) && from->getHp() > 2)
            correct --;
        if(to->hasSkill(objectName()) && to->getHp() <= 2)
            correct ++;

        return correct;
    }
};

class Xuwei: public ZeroCardViewAsSkill{
public:
    Xuwei():ZeroCardViewAsSkill("xuwei"){
        huanzhuang_card = new HuanzhuangCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->hasUsed("HuanzhuangCard"))
            return false;

        return player->getGeneralName() == "sp_diaochan";
    }

    virtual const Card *viewAs() const{
        return huanzhuang_card;
    }

private:
    HuanzhuangCard *huanzhuang_card;
};

class Xiuluo: public PhaseChangeSkill{
public:
    Xiuluo():PhaseChangeSkill("xiuluo"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && !target->isKongcheng()
                && !target->getJudgingArea().isEmpty();
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(!target->askForSkillInvoke(objectName()))
            return false;

        Room *room = target->getRoom();
        int card_id = room->askForCardChosen(target, target, "j", objectName());
        const Card *card = Sanguosha->getCard(card_id);

        QString suit_str = card->getSuitString();
        QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
        QString prompt = QString("@xiuluo:::%1").arg(suit_str);
        if(room->askForCard(target, pattern, prompt)){
            room->throwCard(card);
        }

        return false;
    }
};

class Shenwei: public DrawCardsSkill{
public:
    Shenwei():DrawCardsSkill("shenwei"){
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        return n + 2;
    }
};

class Danji: public PhaseChangeSkill{
public:
    Danji():PhaseChangeSkill("danji"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("danji") == 0
                && target->getHandcardNum() > target->getHp();
    }

    virtual bool onPhaseChange(ServerPlayer *guanyu) const{
        Room *room = guanyu->getRoom();
        ServerPlayer *the_lord = room->getLord();
        if(the_lord && the_lord->getGeneralName() == "caocao"){
            LogMessage log;
            log.type = "#DanjiWake";
            log.from = guanyu;
            log.arg = QString::number(guanyu->getHandcardNum());
            log.arg2 = QString::number(guanyu->getHp());
            room->sendLog(log);

            guanyu->setMark("danji", 1);

            room->loseMaxHp(guanyu);
            room->acquireSkill(guanyu, "mashu");
        }

        return false;
    }
};

SPCardPackage::SPCardPackage()
    :Package("sp_cards")
{
    (new SPMoonSpear)->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(SPCard)

SPPackage::SPPackage()
    :Package("sp")
{
    General *yangxiu = new General(this, "yangxiu", "wei", 3);
    yangxiu->addSkill(new Jilei);
    yangxiu->addSkill(new JileiClear);
    yangxiu->addSkill(new Danlao);

    related_skills.insertMulti("jilei", "#jilei-clear");

    General *gongsunzan = new General(this, "gongsunzan", "qun");
    gongsunzan->addSkill(new Yicong);

    General *yuanshu = new General(this, "yuanshu", "qun");
    yuanshu->addSkill(new Yongsi);
    yuanshu->addSkill(new Weidi);

    General *sp_diaochan = new General(this, "sp_diaochan", "qun", 3, false, true);
    sp_diaochan->addSkill("lijian");
    sp_diaochan->addSkill("biyue");
    sp_diaochan->addSkill(new Xuwei);

    General *sp_sunshangxiang = new General(this, "sp_sunshangxiang", "shu", 3, false, true);
    sp_sunshangxiang->addSkill("jieyin");
    sp_sunshangxiang->addSkill("xiaoji");

    General *shenlvbu1 = new General(this, "shenlvbu1", "god", 8, true, true);
    shenlvbu1->addSkill("mashu");
    shenlvbu1->addSkill("wushuang");

    General *shenlvbu2 = new General(this, "shenlvbu2", "god", 4, true, true);
    shenlvbu2->addSkill("mashu");
    shenlvbu2->addSkill("wushuang");
    shenlvbu2->addSkill(new Xiuluo);
    shenlvbu2->addSkill(new Shenwei);
    shenlvbu2->addSkill(new Skill("shenji"));

    General *sp_guanyu = new General(this, "sp_guanyu", "wei", 4);
    sp_guanyu->addSkill("wusheng");
    sp_guanyu->addSkill(new Danji);

    General *sp_caiwenji = new General(this, "sp_caiwenji", "wei", 3, false, true);
    sp_caiwenji->addSkill("beige");
    sp_caiwenji->addSkill("duanchang");
}

ADD_PACKAGE(SP);
