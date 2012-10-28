#include "scenario.h"
#include "skill.h"
#include "maneuvering.h"
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
    room->setTag("ZhanShuangxiong", true);
    source->pindian(targets.first(), "zhanshuangxiong");
}

class GreatYiji: public MasochismSkill{
public:
    GreatYiji():MasochismSkill("greatyiji"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *guojia, const DamageStruct &damage) const{
        Room *room = guojia->getRoom();
        int x = damage.damage, i;
        for(i = 0; i < x; i++)
        {
            if (!room->askForSkillInvoke(guojia, objectName()))
                return;
            room->broadcastSkillInvoke("yiji");
            QList<int> yiji_cards;
            yiji_cards.append(room->drawCard());
            yiji_cards.append(room->drawCard());
            yiji_cards.append(room->drawCard());
            CardsMoveStruct move;
            move.card_ids = yiji_cards;
            move.to = guojia;
            move.to_place = Player::PlaceHand;
            move.reason = CardMoveReason(CardMoveReason::S_REASON_PREVIEW, guojia->objectName(), "greatyiji", QString());
            room->moveCardsAtomic(move, false);

            if(yiji_cards.isEmpty())
                continue;

            while(room->askForYiji(guojia, yiji_cards)) {}
        }
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

class ZhanShuangxiongViewAsSkill: public ZeroCardViewAsSkill{
public:
    ZhanShuangxiongViewAsSkill():ZeroCardViewAsSkill("zhanshuangxiong"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && !player->hasUsed("ZhanShuangxiongCard");
    }

    virtual const Card *viewAs() const{
        return new ZhanShuangxiongCard();
    }
};

class ZhanShuangxiong: public TriggerSkill{
public:
    ZhanShuangxiong(): TriggerSkill("zhanshuangxiong") {
        events << Pindian;
        view_as_skill = new ZhanShuangxiongViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        if (pindian->reason != objectName())
            return false;
        if (pindian->from_card->getNumber() == pindian->to_card->getNumber())
            return false;

        ServerPlayer *winner = pindian->isSuccess() ? pindian->from : pindian->to;
        ServerPlayer *loser = pindian->isSuccess() ? pindian->to : pindian->from;
        DamageStruct damage;
        damage.from = winner;
        damage.to = loser;
        room->damage(damage);

        return false;
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

            if(can_invoke && room->askForUseCard(zhangliao, "@@smalltuxi", "@smalltuxi-card"))
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
        events << GameStart << DrawNCards << Damaged << GameOverJudge;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        switch(triggerEvent){
        case GameStart:{
            player = room->getLord();
            room->installEquip(player, "RenwangShield");
            room->installEquip(player, "HuaLiu");

            ServerPlayer *caocao = room->findPlayer("caocao");
            room->installEquip(caocao, "QinggangSword");
            room->installEquip(caocao, "ZhuaHuangFeiDian");

            ServerPlayer *liubei = room->findPlayer("liubei");
            room->installEquip(liubei, "DoubleSword");

            ServerPlayer *guanyu = room->findPlayer("guanyu");
            room->installEquip(guanyu, "Blade");
            room->installEquip(guanyu, "ChiTu");
            room->acquireSkill(guanyu, "zhanshuangxiong");


            ServerPlayer *zhangliao = room->findPlayer("zhangliao");
            room->detachSkillFromPlayer(zhangliao, "tuxi");
            room->acquireSkill(zhangliao, "smalltuxi");


            break;
                       }

        case DrawNCards:{
                if(player->getPhase() == Player::Draw){
                    bool burned = room->getTag("BurnWuchao").toBool();
                    if(!burned){
                        QString name = player->getGeneralName();
                        if(name == "caocao" || name == "guojia" || name == "guanyu"){
                            data = data.toInt() - 1;
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

                        const Card *originalCard = Sanguosha->getCard(card_id);
                        SupplyShortage *shortage = new SupplyShortage(originalCard->getSuit(), originalCard->getNumber());
                        shortage->setSkillName("duanliang");
                        WrappedCard *card = Sanguosha->getWrappedCard(originalCard->getId());
                        card->takeOver(shortage);
                        room->broadcastUpdateCard(room->getPlayers(), card->getId(), card);
                        room->moveCardTo(card, to, Player::PlaceDelayedTrick, true);
                        shortage->deleteLater();
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
            room->detachSkillFromPlayer(guojia, "yiji");
            room->acquireSkill(guojia, "greatyiji");
            room->acquireSkill(guojia, "damagebeforeplay", false);
        }
    }
}
