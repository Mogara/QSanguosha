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
    return targets.isEmpty() && to_select->getGeneralName() == "shuangxiong" && !to_select->isKongcheng();
}

void ZhanShuangxiongCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *shuangxiong = targets.first();

    DamageStruct damage;
    damage.from = source;
    damage.to = shuangxiong;

    bool success = source->pindian(shuangxiong, "zhanshuangxiong");
    if(!success)
        qSwap(damage.from, damage.to);

    room->damage(damage);

    room->setTag("ZhanShuangxiong", true);

    LogMessage log;
    log.type = "#Guandu_ZhanShuangxiong";
    log.from = source;
    source->getRoom()->sendLog(log);
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

            LogMessage log;
            log.type = "#Guandu_Yijidingliaodong";
            log.from = target;
            target->getRoom()->sendLog(log);
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

    virtual const Card *viewAs() const{
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

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getGeneralName() == "guandu-zhangliao"
                && ! target->getRoom()->getTag("BurnWuchao").toBool();
    }

    virtual bool onPhaseChange(ServerPlayer *Guandu_zhangliao) const{
        if(Guandu_zhangliao->getPhase() == Player::Draw){
            Room *room = Guandu_zhangliao->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(Guandu_zhangliao);
            foreach(ServerPlayer *player, other_players){
                if(!player->isKongcheng()){
                    can_invoke = true;
                    break;
                }
            }

            if(!can_invoke || !room->askForUseCard(Guandu_zhangliao, "@@smalltuxi", "@tuxi-card"))
                Guandu_zhangliao->drawCards(1, false);

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
        events << GameStart << PhaseChange << Damaged << GameOverJudge;
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
                    room->installEquip(guanyu, "blade");
                    room->installEquip(guanyu, "chitu");
                    room->acquireSkill(guanyu, "zhanshuangxiong");

                    //ServerPlayer *zhangliao = room->findPlayer("zhangliao");
                    //room->acquireSkill(zhangliao, "smalltuxi");

                    ServerPlayer *zhenji = room->findPlayer("zhenji");
                        room->setPlayerProperty(zhenji, "kingdom", "qun");
                }

                break;
            }

        case PhaseChange:{
                if(player->getPhase() == Player::Draw){
                    bool burned = room->getTag("BurnWuchao").toBool();
                    if(!burned){
                        QString name = player->getGeneralName();
                        if(name == "caocao" || name == "guojia" || name == "guanyu"){

                            LogMessage log;
                            log.type = "#Guandu_Caojunqueliang";
                            log.from = player;
                            player->getRoom()->sendLog(log);

                            player->drawCards(1, false);
                            return true;
                        }
                        else if(name == "guandu-zhangliao"){
                            LogMessage log;
                            log.type = "#Guandu_Caojunqueliang";
                            log.from = player;
                            player->getRoom()->sendLog(log);
                            return false;
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
                ServerPlayer *Guandu_zhangliao = room->findPlayer("guandu-zhangliao");
                if(player->getGeneralName() == "yuanshao" && damage.nature == DamageStruct::Fire
                   && damage.from->getRoleEnum() == Player::Rebel){
                    room->setTag("BurnWuchao", true);

                    LogMessage log;
                    log.type = "#Guandu_BurnWuchao";
                    log.from = player;
                    player->getRoom()->sendLog(log);

                    QStringList tos;
                    tos << "yuanshao" << "shuangxiong" << "zhenji" << "liubei";

                    foreach(QString name, tos){
                        ServerPlayer *to = room->findPlayer(name);
                        if(to == NULL || to->containsTrick("supply_shortage"))
                            continue;

                        int card_id = room->getCardFromPile("@duanliang");
                        if(card_id == -1){
                            break;
                        }

                        room->moveCardTo(Sanguosha->getCard(card_id), to, Player::Judging, true);
                    }
                    room->setPlayerProperty(Guandu_zhangliao, "general", "zhangliao");
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
    loyalists << "shuangxiong" << "zhenji";
    rebels << "caocao" << "guandu-zhangliao" << "guojia";
    renegades << "liubei" << "guanyu";

    rule = new GuanduRule(this);

    skills  << new ZhanShuangxiong
            << new GreatYiji
            << new DamageBeforePlay;

    General *Guandu_zhangliao = new General(this, "guandu-zhangliao", "wei", 4, true, true);
    Guandu_zhangliao->addSkill(new SmallTuxi);

    addMetaObject<ZhanShuangxiongCard>();
    addMetaObject<SmallTuxiCard>();
}

AI::Relation GuanduScenario::relationTo(const ServerPlayer *a, const ServerPlayer *b) const{
    if(a->getRole() == "renegade" && b->getRole() == "renegade")
        return AI::Friend;
    else
        return AI::GetRelation(a, b);
}

void GuanduScenario::getRoles(char *roles) const{
    strcpy(roles, "ZCCFFFNN");
}

void GuanduScenario::onTagSet(Room *room, const QString &key) const{
    bool zhanshuangxiong = room->getTag("ZhanShuangxiong").toBool();
    bool burnwuchao = room->getTag("BurnWuchao").toBool();
    if(zhanshuangxiong && burnwuchao){
        ServerPlayer *guojia = room->findPlayer("guojia");
        if(guojia && !guojia->hasSkill("greatyiji")){
            room->acquireSkill(guojia, "greatyiji");
            room->acquireSkill(guojia, "damagebeforeplay", false);
        }
    }
}

ADD_SCENARIO(Guandu);


