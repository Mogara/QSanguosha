#include "scenario.h"
#include "skill.h"
#include "guandu-scenario.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"

ZhanShuangxiongCard::ZhanShuangxiongCard(){

}

bool ZhanShuangxiongCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && to_select->getGeneralName() == "shuangxiong" && !to_select->isKongcheng();
}

void ZhanShuangxiongCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *shuangxiong = targets.first();

    DamageStruct damage;
    damage.from = source;
    damage.to = shuangxiong;

    bool success = room->pindian(source, shuangxiong);
    if(!success)
        qSwap(damage.from, damage.to);

    room->damage(damage);

    room->setTag("ZhanShuangxiong", true);
}

void ZhanShuangxiongCard::use(const QList<const ClientPlayer *> &) const{
    ClientInstance->turn_tag.insert("zhanshuangxiong", true);
}

class GreatYiji: public MasochismSkill{
public:
    GreatYiji():MasochismSkill("greatyiji"){
        frequency = Compulsory;
    }

    virtual void onDamaged(ServerPlayer *guojia, const DamageStruct &damage) const{
        Room *room = guojia->getRoom();

        room->playSkillEffect(objectName());
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

    virtual bool isEnabledAtPlay() const{
        return !Self->isKongcheng() && !ClientInstance->turn_tag.value(objectName(), false).toBool();
    }

    virtual const Card *viewAs() const{
        return new ZhanShuangxiongCard();
    }
};

SmallTuxiCard::SmallTuxiCard(){
}

bool SmallTuxiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

void SmallTuxiCard::onEffect(const CardEffectStruct &effect) const{
    TuxiCard::onEffect(effect);

    effect.from->getRoom()->playSkillEffect("tuxi");
}

class SmallTuxiViewAsSkill: public ZeroCardViewAsSkill{
public:
    SmallTuxiViewAsSkill():ZeroCardViewAsSkill("smalltuxi"){
    }

    virtual const Card *viewAs() const{
        return new SmallTuxiCard;
    }

protected:
    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@smalltuxi";
    }
};

class SmallTuxi:public PhaseChangeSkill{
public:
    SmallTuxi():PhaseChangeSkill("smalltuxi"){
        view_as_skill = new SmallTuxiViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getGeneralName() == "zhangliao"
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

            if(!can_invoke || !room->askForUseCard(zhangliao, "@@smalltuxi", "@tuxi-card"))
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
        events << GameStart << PhaseChange << Damaged << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        switch(event){
        case GameStart:{
                if(player->isLord()){
                    room->installEquip(player, "renwang_shield");
                    room->installEquip(player, "hualiu");

                    ServerPlayer *caocao = room->findPlayer("caocao");
                    room->installEquip(caocao, "qinggang_sword");
                    room->installEquip(caocao, "zhuahuangfeidian");

                    ServerPlayer *liubei = room->findPlayer("liubei");
                    room->installEquip(liubei, "double_sword");

                    ServerPlayer *guanyu = room->findPlayer("guanyu");
                    if(guanyu){
                        room->installEquip(guanyu, "blade");
                        room->installEquip(guanyu, "chitu");
                        room->acquireSkill(guanyu, "zhanshuangxiong");
                    }

                    ServerPlayer *zhangliao = room->findPlayer("zhangliao");
                    if(zhangliao)
                        room->acquireSkill(zhangliao, "smalltuxi");
                }

                break;
            }

        case PhaseChange:{
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
                    tos << "yuanshao" << "shuangxiong" << "zhenji" << "liubei";

                    foreach(QString name, tos){
                        qDebug("duanliang against %s", qPrintable(name));

                        ServerPlayer *to = room->findPlayer(name);
                        if(to == NULL || to->containsTrick("supply_shortage"))
                            continue;

                        int card_id = room->getCardFromPile("@duanliang");
                        if(card_id == -1){
                            break;
                        }

                        room->moveCardTo(card_id, to, Player::Judging, true);
                    }
                }

                break;
            }

        case Death:{
                if(player->getRoleEnum() == Player::Lord){
                    QStringList roles = room->aliveRoles(player);
                    if(roles.length() == 2){
                        QString first = roles.at(0);
                        QString second = roles.at(1);
                        if(first == "renegade" && second == "renegade"){
                            player->throwAllCards();
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
    role_map["yuanshao"] = "lord";
    role_map["shuangxiong"] = role_map["zhenji"] = "loyalist";
    role_map["caocao"] = role_map["zhangliao"] = role_map["guojia"] = "rebel";
    role_map["liubei"] = role_map["guanyu"] = "renegade";

    rule = new GuanduRule(this);

    t["guandu"] = tr("guandu");

    t["smalltuxi"] = tr("smalltuxi");
    t["zhanshuangxiong"] = tr("zhanshuangxiong");
    t["greatyiji"] = tr("greatyiji");

    skills << new SmallTuxi
            << new ZhanShuangxiong
            << new GreatYiji
            << new DamageBeforePlay;

    addMetaObject<ZhanShuangxiongCard>();
    addMetaObject<SmallTuxiCard>();
}

void GuanduScenario::onTagSet(Room *room, const QString &key) const{
    bool zhanshuangxiong = room->getTag("ZhanShuangxiong").toBool();
    bool burnwuchao = room->getTag("BurnWuchao").toBool();
    if(zhanshuangxiong && burnwuchao){
        ServerPlayer *guojia = room->findPlayer("guojia");
        if(guojia && !guojia->hasSkill("greatyiji")){
            const TriggerSkill *yiji = qobject_cast<const TriggerSkill *>(Sanguosha->getSkill("yiji"));
            room->getThread()->removeTriggerSkill(yiji);

            room->acquireSkill(guojia, "greatyiji");
            room->acquireSkill(guojia, "damagebeforeplay", false);
        }
    }
}

ADD_SCENARIO(Guandu);


