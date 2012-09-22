#include "assassinspackage.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"

class Moukui: public TriggerSkill{
public:
    Moukui(): TriggerSkill("moukui"){
        events << TargetConfirmed << SlashMissed << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(!player->hasSkill(objectName()) || player->objectName() != use.from->objectName() || !use.card->isKindOf("Slash"))
                return false;
            foreach(ServerPlayer *p, use.to){
                if(player->askForSkillInvoke(objectName(), QVariant::fromValue(p))){
                    room->broadcastSkillInvoke(objectName());
                    QString choice;
                    if (p->isNude())
                        choice = "draw";
                    else
                        choice = room->askForChoice(player, objectName(), "draw+discard");
                    if (choice == "draw")
                        player->drawCards(1);
                    else {
                        int disc = room->askForCardChosen(player, p, "he", objectName());
                        room->throwCard(disc, p, player);
                    }
                    room->setPlayerMark(p, objectName() + use.card->getEffectIdString(),
                                        p->getMark(objectName() + use.card->getEffectIdString()) + 1);
                }
            }
        } else if (triggerEvent == SlashMissed) {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if (effect.to->getMark(objectName() + effect.slash->getEffectIdString()) <= 0)
                return false;
            if (!effect.from->isAlive() || !effect.to->isAlive() || effect.from->isNude())
                return false;
            int disc = room->askForCardChosen(effect.to, effect.from, "he", objectName());
            room->throwCard(disc, effect.from, player);
            room->setPlayerMark(effect.to, objectName() + effect.slash->getEffectIdString(),
                                effect.to->getMark(objectName() + effect.slash->getEffectIdString()) - 1);
        } else if (triggerEvent == CardFinished) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.card->isKindOf("Slash"))
                return false;
            foreach (ServerPlayer *p, room->getAllPlayers())
                room->setPlayerMark(p, objectName() + use.card->getEffectIdString(), 0);
        }

        return false;
    }
};

class Tianming: public TriggerSkill{
public:
    Tianming(): TriggerSkill("tianming"){
        events << TargetConfirming;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card && use.card->isKindOf("Slash") && player->askForSkillInvoke(objectName())){
            if(!player->isNude()){
                int total = 0;
                QSet<const Card *> jilei_cards;
                QList<const Card *> handcards = player->getHandcards();
                foreach(const Card *card, handcards){
                    if(player->isJilei(card))
                        jilei_cards << card;
                }
                total = handcards.size() - jilei_cards.size() + player->getEquips().length();

                if(total <= 2)
                    player->throwAllHandCardsAndEquips();
                else
                    room->askForDiscard(player, objectName(), 2, 2, false, true);
            }

            player->drawCards(2);

            int max = -1000;
            foreach(ServerPlayer *p, room->getAllPlayers())
                if(p->getHp() > max)
                    max = p->getHp();
            if (player->getHp() == max)
                return false;

            QList<ServerPlayer *> maxs;
            foreach(ServerPlayer *p, room->getAllPlayers()){
                if (p->getHp() == max)
                    maxs << p;
                if (maxs.size() != 1)
                    return false;
                ServerPlayer *mosthp = maxs.first();
                if (room->askForSkillInvoke(mosthp, objectName())) {
                    QSet<const Card *> jilei_cards;
                    QList<const Card *> handcards = mosthp->getHandcards();
                    foreach(const Card *card, handcards){
                        if(mosthp->isJilei(card))
                            jilei_cards << card;
                    }
                    int total = handcards.size() - jilei_cards.size() + mosthp->getEquips().length();

                    if(total <= 2)
                        mosthp->throwAllHandCardsAndEquips();
                    else {
                        room->askForDiscard(mosthp, objectName(), 2, 2, false, true);
                        mosthp->drawCards(2);
                    }
                }
            }
		}

        return false;
    }
};

MizhaoCard::MizhaoCard(){
    will_throw = false;
}

bool MizhaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void MizhaoCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(effect.card, false);
    Room *room = effect.from->getRoom();

    QList<ServerPlayer *> targets;
    foreach(ServerPlayer *p, room->getOtherPlayers(effect.to))
        if(!p->isKongcheng())
            targets << p;

    if(!effect.to->isKongcheng() && !targets.isEmpty()) {
        ServerPlayer *target = room->askForPlayerChosen(effect.from, targets, "mizhao");
        effect.to->pindian(target, "mizhao", NULL);
    }
}

class MizhaoViewAsSkill: public ViewAsSkill{
public:
    MizhaoViewAsSkill():ViewAsSkill("#mizhao"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && !player->hasUsed("MizhaoCard");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() < Self->getHandcardNum())
            return NULL;

        MizhaoCard *card = new MizhaoCard;
        card->addSubcards(cards);
        return card;
    }
};

class Mizhao: public TriggerSkill{
public:
    Mizhao(): TriggerSkill("mizhao") {
        events << Pindian;
        view_as_skill = new MizhaoViewAsSkill;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        if (pindian->reason != objectName())
            return false;

        ServerPlayer *winner = pindian->isSuccess() ? pindian->from : pindian->to;
        ServerPlayer *loser = pindian->isSuccess() ? pindian->to : pindian->from;
        if (winner->canSlash(loser, NULL, false)) {
            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName("mizhao");
            CardUseStruct card_use;
            card_use.from = winner;
            card_use.to << loser;
            card_use.card = slash;
            room->useCard(card_use, false);
        }

        return false;
    }
};

class Jieyuan: public TriggerSkill{
public:
    Jieyuan(): TriggerSkill("jieyuan"){
        events << DamageCaused << DamageInflicted;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(triggerEvent == DamageCaused){
            if(damage.to->getHp() >= player->getHp() || damage.to != player)
                if(room->askForCard(player, ".black", "@JieyuanIncrease", data, CardDiscarded)){
                    LogMessage log;
                    log.type = "#JieyuanIncrease";
                    log.from = player;
                    log.arg = QString::number(damage.damage);
                    log.arg2 = QString::number(++damage.damage);
                    room->sendLog(log);

                    data = QVariant::fromValue(damage);
                }
        }else if(triggerEvent == DamageInflicted){
            if(damage.from->getHp() >= player->getHp() && damage.from != player)
                if(room->askForCard(player, ".red", "@JieyuanDecrease", data, CardDiscarded)){
                    LogMessage log;
                    log.type = "#JieyuanDecrease";
                    log.from = player;
                    log.arg = QString::number(damage.damage);
                    log.arg2 = QString::number(--damage.damage);
                    room->sendLog(log);

                    if (damage.damage < 1)
                        return true;
                    data = QVariant::fromValue(damage);
                }
        }

        return false;
    }
};

class Fenxin: public TriggerSkill{
public:
    Fenxin(): TriggerSkill("fenxin"){
        events << AskForPeachesDone;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        if (!room->getMode().endsWith("p") && !room->getMode().endsWith("pd") && !room->getMode().endsWith("pz"))
            return false;
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.damage == NULL)
            return false;
        ServerPlayer *killer = dying.damage->from;
        if (killer->isLord() || player->isLord() || player->getHp() > 0)
            return false;
        if (!killer->hasSkill(objectName()) || killer->getMark("@burnheart") == 0)
            return false;
        if (room->askForSkillInvoke(killer, objectName(), QVariant::fromValue(player))) {
            killer->loseMark("@burnheart");
            QString role1 = killer->getRole();
            killer->setRole(player->getRole());
            room->setPlayerProperty(killer, "role", player->getRole());
            player->setRole(role1);
            room->setPlayerProperty(player, "role", role1);
        }
        return false;
    }
};

AssassinsPackage::AssassinsPackage():Package("assassins"){
    General *mushun = new General(this, "mushun", "qun", 4);
    mushun->addSkill(new Moukui);

    General *liuxie = new General(this, "liuxie", "qun", 3);
    liuxie->addSkill(new Tianming);
    liuxie->addSkill(new Mizhao);

    General *lingju = new General(this, "lingju", "qun", 3, false);
    lingju->addSkill(new Jieyuan);
    lingju->addSkill(new Fenxin);
    lingju->addSkill(new MarkAssignSkill("@burnheart", 1));

    addMetaObject<MizhaoCard>();
}

ADD_PACKAGE(Assassins)
