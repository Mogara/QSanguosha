#include "hongyan-scenario.h"
#include "engine.h"
#include "standard-skillcards.h"
#include "clientplayer.h"
#include "client.h"
#include "carditem.h"
#include "skill.h"
#include "carditem.h"
#include "standard.h"

LesbianJieyinCard::LesbianJieyinCard()
    :JieyinCard()
{

}

bool LesbianJieyinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return to_select->getGeneral()->isFemale() && to_select->isWounded();
}

class LesbianJieyin: public ViewAsSkill{
public:
    LesbianJieyin():ViewAsSkill("lesbianjieyin"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LesbianJieyinCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() > 2)
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        LesbianJieyinCard *jieyin_card = new LesbianJieyinCard();
        jieyin_card->addSubcards(cards);

        return jieyin_card;
    }
};

//-----------------------------

LesbianLijianCard::LesbianLijianCard()
    :LijianCard()
{
}

bool LesbianLijianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!to_select->getGeneral()->isFemale())
        return false;

    if(targets.isEmpty() && to_select->hasSkill("kongcheng") && to_select->isKongcheng()){
        return false;
    }

    return true;
}

class LesbianLijian: public OneCardViewAsSkill{
public:
    LesbianLijian():OneCardViewAsSkill("lesbianlijian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LesbianLijianCard");
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LesbianLijianCard *lijian_card = new LesbianLijianCard;
        lijian_card->addSubcard(card_item->getCard()->getId());

        return lijian_card;
    }
};


//LESBIAN LIANLI RELATED

LesbianLianliCard::LesbianLianliCard(){

}

bool LesbianLianliCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getGeneral()->isFemale();
}

void LesbianLianliCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    LogMessage log;
    log.type = "#lesbianlianliConnection";
    log.from = effect.from;
    log.to << effect.to;
    room->sendLog(log);

    if(effect.from->getMark("@tied") == 0)
        effect.from->gainMark("@tied");

    if(effect.to->getMark("@tied") == 0){
        QList<ServerPlayer *> players = room->getOtherPlayers(effect.from);
        foreach(ServerPlayer *player, players){
            if(player->getMark("@tied") > 0){
                player->loseMark("@tied");
                break;
            }
        }

        effect.to->gainMark("@tied");
    }
}

class LesbianLianliStart: public GameStartSkill{
public:
    LesbianLianliStart():GameStartSkill("#lesbianlianli-start") {

    }

    virtual void onGameStart(ServerPlayer *player) const{
        Room *room = player->getRoom();

        QList<ServerPlayer *> players = room->getOtherPlayers(player);
        foreach(ServerPlayer *player, players){
            if(player->getGeneral()->isFemale())
                room->attachSkillToPlayer(player, "lesbianlianli-slash");
        }
    }
};

LesbianLianliSlashCard::LesbianLianliSlashCard(){

}

bool LesbianLianliSlashCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->canSlash(to_select);
}

void LesbianLianliSlashCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhangfei = effect.from;
    Room *room = zhangfei->getRoom();

    ServerPlayer *xiahoujuan = room->findPlayerBySkillName("lesbianlianli");
    if(xiahoujuan){
        const Card *slash = room->askForCard(xiahoujuan, "slash", "@lesbianlianli-slash");
        if(slash){
            zhangfei->invoke("increaseSlashCount");
            room->cardEffect(slash, zhangfei, effect.to);
            return;
        }
    }
}

class LesbianLianliSlashViewAsSkill:public ZeroCardViewAsSkill{
public:
    LesbianLianliSlashViewAsSkill():ZeroCardViewAsSkill("lesbianlianli-slash"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@tied") > 0 && Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return new LesbianLianliSlashCard;
    }
};

class LesbianLianliSlash: public TriggerSkill{
public:
    LesbianLianliSlash():TriggerSkill("#lesbianlianli-slash"){
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@tied") > 0 && !target->hasSkill("lesbianlianli");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "slash")
            return false;

        Room *room = player->getRoom();
        if(!player->askForSkillInvoke("lesbianlianli-slash", data))
            return false;

        ServerPlayer *xiahoujuan = room->findPlayerBySkillName("lesbianlianli");
        if(xiahoujuan){
            const Card *slash = room->askForCard(xiahoujuan, "slash", "@lesbianlianli-slash");
            if(slash){
                room->provide(slash);
                return true;
            }
        }

        return false;
    }
};

class LesbianLianliJink: public TriggerSkill{
public:
    LesbianLianliJink():TriggerSkill("#lesbianlianli-jink"){
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@tied") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *xiahoujuan, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "jink")
            return false;

        if(!xiahoujuan->askForSkillInvoke("lesbianlianli-jink", data))
            return false;

        Room *room = xiahoujuan->getRoom();
        QList<ServerPlayer *> players = room->getOtherPlayers(xiahoujuan);
        foreach(ServerPlayer *player, players){
            if(player->getMark("@tied") > 0){
                ServerPlayer *zhangfei = player;

                const Card *jink = room->askForCard(zhangfei, "jink", "@lesbianlianli-jink");
                if(jink){
                    room->provide(jink);
                    return true;
                }

                break;
            }
        }

        return false;
    }
};

class LesbianLianliViewAsSkill: public ZeroCardViewAsSkill{
public:
    LesbianLianliViewAsSkill():ZeroCardViewAsSkill(""){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@lesbianlianli";
    }

    virtual const Card *viewAs() const{
        return new LesbianLianliCard;
    }
};

class LesbianLianli: public PhaseChangeSkill{
public:
    LesbianLianli():PhaseChangeSkill("lesbianlianli"){
        view_as_skill = new LesbianLianliViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Start){
            Room *room = target->getRoom();
            bool used = room->askForUseCard(target, "@lesbianlianli", "@@lesbianlianli-card");
            if(used){
                if(target->getKingdom() != "shu")
                    room->setPlayerProperty(target, "kingdom", "shu");
            }else{
                if(target->getKingdom() != "wei")
                    room->setPlayerProperty(target, "kingdom", "wei");

                QList<ServerPlayer *> players = room->getAllPlayers();
                foreach(ServerPlayer *player, players){
                    if(player->getMark("@tied") > 0)
                        player->loseMark("@tied");
                }
            }
        }

        return false;
    }
};

class Tongxin: public MasochismSkill{
public:
    Tongxin():MasochismSkill("tongxin"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@tied") > 0;
    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{
        Room *room = target->getRoom();
        ServerPlayer *xiahoujuan = room->findPlayerBySkillName(objectName());

        if(xiahoujuan && xiahoujuan->askForSkillInvoke(objectName(), QVariant::fromValue(damage))){
            room->playSkillEffect(objectName());

            ServerPlayer *zhangfei = NULL;
            if(target == xiahoujuan){
                QList<ServerPlayer *> players = room->getOtherPlayers(xiahoujuan);
                foreach(ServerPlayer *player, players){
                    if(player->getMark("@tied") > 0){
                        zhangfei = player;
                        break;
                    }
                }
            }else
                zhangfei = target;

            xiahoujuan->drawCards(damage.damage);

            if(zhangfei)
                zhangfei->drawCards(damage.damage);
        }
    }
};

class LesbianLianliClear: public TriggerSkill{
public:
    LesbianLianliClear():TriggerSkill("#lesbianlianli-clear"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> players = room->getAllPlayers();
        foreach(ServerPlayer *player, players){
            if(player->getMark("@tied") > 0)
                player->loseMark("@tied");
        }

        return false;
    }
};
//END LESBIAN LIANLI RELATED

class HongyanRule: public ScenarioRule{
public:
    HongyanRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(player->hasSkill("jieyin"))
            room->acquireSkill(player, "lesbianjieyin");
        else if(player->hasSkill("lijian"))
            room->acquireSkill(player, "lesbianlijian");
        else if(player->hasSkill("lianli")){
            room->acquireSkill(player, "lesbianlianli");
            room->acquireSkill(player, "lesbianlianli-start");
            room->acquireSkill(player, "lesbianlianli-slash");
            room->acquireSkill(player, "lesbianlianli-jink");
            room->acquireSkill(player, "lesbianlianli-clear");
        }
        return false;
    }
};

HongyanScenario::HongyanScenario()
    :Scenario("hongyan_lesbian")
{
    females << "zhenji" << "huangyueying" << "zhurong"
            << "daqiao" << "caiwenji" << "xiaoqiao"
            << "diaochan" << "caizhaoji" << "sunshangxiang"
            << "wuguotai" << "xiahoujuan";
    standard_roles << "lord" << "loyalist" << "loyalist" << "loyalist"
            << "rebel" << "rebel" << "rebel" << "rebel" << "renegade" << "renegade";

    rule = new HongyanRule(this);

    related_skills.insertMulti("lesbianlianli", "#lesbianlianli-start");
    related_skills.insertMulti("lesbianlianli", "#lesbianlianli-slash");
    related_skills.insertMulti("lesbianlianli", "#lesbianlianli-jink");
    related_skills.insertMulti("lesbianlianli", "#lesbianlianli-clear");

    skills << new LesbianJieyin << new LesbianLijian
            << new LesbianLianliStart
            << new LesbianLianli
            << new LesbianLianliSlash
            << new LesbianLianliJink
            << new LesbianLianliClear
            << new LesbianLianliSlashViewAsSkill;

    addMetaObject<LesbianJieyinCard>();
    addMetaObject<LesbianLijianCard>();
    addMetaObject<LesbianLianliCard>();
    addMetaObject<LesbianLianliSlashCard>();
}

bool HongyanScenario::exposeRoles() const{
    return false;
}

void HongyanScenario::assign(QStringList &generals, QStringList &roles) const{
    generals = females;
    qShuffle(generals);
    generals.removeLast();
    roles = standard_roles;
    qShuffle(roles);
}

int HongyanScenario::getPlayerCount() const{
    return 10;
}

void HongyanScenario::getRoles(char *roles) const{
    strcpy(roles, "ZCCCFFFFNN");
}

void HongyanScenario::onTagSet(Room *room, const QString &key) const{
    // dummy
}

ADD_SCENARIO(Hongyan);
