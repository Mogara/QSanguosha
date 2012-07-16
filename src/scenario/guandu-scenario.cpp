#include "scenario.h"
#include "skill.h"
#include "guandu-scenario.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"

ZhanShuangxiongCard::ZhanShuangxiongCard(){
    once = true;
}

bool ZhanShuangxiongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getGeneralName() == "yanliangwenchou" && !to_select->isKongcheng();
}

void ZhanShuangxiongCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *shuangxiong = targets.first();

    DamageStruct damage;
    damage.from = source;
    damage.to = shuangxiong;

    bool success = source->pindian(shuangxiong, "zhanshuangxiong");
    if(!success)
        qSwap(damage.from, damage.to);

    room->damage(damage);

    room->setTag("ZhanShuangxiong", true);
}

class GreatYiji: public MasochismSkill{
public:
    GreatYiji():MasochismSkill("greatyiji"){
        frequency = Compulsory;
    }

    virtual void onDamaged(ServerPlayer *guojia, const DamageStruct &damage) const{
        Room *room = guojia->getRoom();

        room->broadcastSkillInvoke(objectName());
        int n = damage.damage * 3;
        guojia->drawCards(n);
        QList<int> yiji_cards = guojia->handCards().mid(guojia->getHandcardNum() - n);

        while(room->askForYiji(guojia, yiji_cards))
            ; // empty loop
    }
};

class DamageBeforePlay: public PhaseChangeSkill{
public:
    DamageBeforePlay():PhaseChangeSkill("damagebeforeplay"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Play){
            DamageStruct damage;
            damage.to = target;
            target->getRoom()->damage(damage);
        }

        return false;
    }
};

class ZhanShuangxiong: public ZeroCardViewAsSkill{
public:
    ZhanShuangxiong():ZeroCardViewAsSkill("zhanshuangxiong"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && !player->hasUsed("ZhanShuangxiongCard");
    }

    virtual Card *viewAs() const{
        return new ZhanShuangxiongCard();
    }
};

SmallTuxiCard::SmallTuxiCard(){
}

bool SmallTuxiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

void SmallTuxiCard::onEffect(const CardEffectStruct &effect) const{
    TuxiCard::onEffect(effect);

    effect.from->getRoom()->broadcastSkillInvoke("tuxi");
}

class SmallTuxiViewAsSkill: public ZeroCardViewAsSkill{
public:
    SmallTuxiViewAsSkill():ZeroCardViewAsSkill("smalltuxi"){
    }

    virtual Card *viewAs() const{
        return new SmallTuxiCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@smalltuxi";
    }
};

class SmallTuxi:public PhaseChangeSkill{
public:
    SmallTuxi():PhaseChangeSkill("smalltuxi"){
        view_as_skill = new SmallTuxiViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->getGeneralName() == "zhangliao"
                && ! target->getRoom()->getTag("BurnWuchao").toBool();
    }

    virtual bool onPhaseChange(ServerPlayer *zhangliao) const{
        if(zhangliao->getPhase() == Player::Draw){
            Room *room = zhangliao->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(zhangliao);
            foreach(ServerPlayer *player, other_players){
                if(!player->isKongcheng()){
                    can_invoke = true;
                    break;
                }
            }

            if(!can_invoke || !room->askForUseCard(zhangliao, "@@smalltuxi", "@smalltuxi-card"))
                zhangliao->drawCards(1, false);

            return true;
        }

        return false;
    }
};

class GuanduRule: public ScenarioRule{
public:
    GuanduRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << EventPhaseStart << Damaged << GameOverJudge;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        switch(triggerEvent){
        case GameStart:{
            player = room->getLord();
            room->installEquip(player, "renwang_shield");
            room->installEquip(player, "hualiu");

            ServerPlayer *caocao = room->findPlayer("caocao");
            room->installEquip(caocao, "qinggang_sword");
            room->installEquip(caocao, "zhuahuangfeidian");

            ServerPlayer *liubei = room->findPlayer("liubei");
            room->installEquip(liubei, "double_sword");

            ServerPlayer *guanyu = room->findPlayer("guanyu");
            room->installEquip(guanyu, "blade");
            room->installEquip(guanyu, "chitu");
            room->acquireSkill(guanyu, "zhanshuangxiong");


            ServerPlayer *zhangliao = room->findPlayer("zhangliao");
            room->detachSkillFromPlayer(zhangliao, "tuxi");
            room->acquireSkill(zhangliao, "smalltuxi");


            break;
                       }

        case EventPhaseStart:{
                if(player->getPhase() == Player::Draw){
                    bool burned = room->getTag("BurnWuchao").toBool();
                    if(!burned){
                        QString name = player->getGeneralName();
                        if(name == "caocao" || name == "guojia" || name == "guanyu"){
                            player->drawCards(1, false);
                            return true;
                        }
                    }
                }

                break;
            }

        case Damaged:{
                bool burned = room->getTag("BurnWuchao").toBool();
                if(burned)
                    return false;

                DamageStruct damage = data.value<DamageStruct>();
                if(player->getGeneralName() == "yuanshao" && damage.nature == DamageStruct::Fire
                   && damage.from->getRoleEnum() == Player::Rebel){
                    room->setTag("BurnWuchao", true);

                    QStringList tos;
                    tos << "yuanshao" << "yanliangwenchou" << "zhenji" << "liubei";

                    foreach(QString name, tos){
                        ServerPlayer *to = room->findPlayer(name);
                        if(to == NULL || to->containsTrick("supply_shortage"))
                            continue;

                        int card_id = room->getCardFromPile("@duanliang");
                        if(card_id == -1){
                            break;
                        }

                        room->moveCardTo(Sanguosha->getCard(card_id), to, Player::PlaceDelayedTrick, true);
                    }
                }

                break;
            }

        case GameOverJudge:{
                if(player->isLord()){
                    QStringList roles = room->aliveRoles(player);
                    if(roles.length() == 2){
                        QString first = roles.at(0);
                        QString second = roles.at(1);
                        if(first == "renegade" && second == "renegade"){
                            player->bury();
                            room->gameOver("renegade");
                            return true;
                        }
                    }
                }

                break;
            }

        default:
            break;
        }

        return false;
    }
};

GuanduScenario::GuanduScenario()
    :Scenario("guandu")
{
    lord = "yuanshao";
    loyalists << "yanliangwenchou" << "zhenji";
    rebels << "caocao" << "zhangliao" << "guojia";
    renegades << "liubei" << "guanyu";

    rule = new GuanduRule(this);

    skills << new SmallTuxi
            << new ZhanShuangxiong
            << new GreatYiji
            << new DamageBeforePlay;

    addMetaObject<ZhanShuangxiongCard>();
    addMetaObject<SmallTuxiCard>();
}

AI::Relation GuanduScenario::relationTo(const ServerPlayer *a, const ServerPlayer *b) const{
    if(a->getRole() == "renegade" && b->getRole() == "renegade")
        return AI::Friend;
    else
        return AI::GetRelation(a, b);
}

void GuanduScenario::onTagSet(Room *room, const QString &key) const{
    bool zhanshuangxiong = room->getTag("ZhanShuangxiong").toBool();
    bool burnwuchao = room->getTag("BurnWuchao").toBool();

    if(burnwuchao){
        ServerPlayer *zhangliao = room->findPlayer("zhangliao");
        if(zhangliao && !zhangliao->hasSkill("tuxi")){
            room->acquireSkill(zhangliao, "tuxi");
            room->detachSkillFromPlayer(zhangliao, "smalltuxi");
        }
    }
    if(zhanshuangxiong && burnwuchao){
        ServerPlayer *guojia = room->findPlayer("guojia");
        if(guojia && !guojia->hasSkill("greatyiji")){
            room->acquireSkill(guojia, "greatyiji");
            room->acquireSkill(guojia, "damagebeforeplay", false);
        }
    }
}

ADD_SCENARIO(Guandu);


