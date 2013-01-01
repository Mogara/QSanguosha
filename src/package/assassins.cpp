#include "assassins.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"

class Moukui: public TriggerSkill{
public:
    Moukui(): TriggerSkill("moukui"){
        events << CardUsed << SlashMissed << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room* room = player->getRoom();
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(player != use.from || !use.card->isKindOf("Slash"))
                return false;
            foreach(ServerPlayer *p, use.to) {
                if(player->askForSkillInvoke(objectName(), QVariant::fromValue(p))) {
                    QString choice;
                    if(p->isNude())
                        choice = "draw";
                    else
                        choice = room->askForChoice(player, objectName(), "draw+discard");
                    if(choice == "draw") {
                        room->playSkillEffect(objectName(), 1);
                        player->drawCards(1);
                    }else {
                        room->playSkillEffect(objectName(), 2);
                        int disc = room->askForCardChosen(player, p, "he", objectName());
                        room->throwCard(disc);
                    }
                    room->setPlayerMark(p, objectName() + use.card->getEffectIdString(),
                                        p->getMark(objectName() + use.card->getEffectIdString()) + 1);
                }
            }
        }else if(event == SlashMissed) {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(effect.to->getMark(objectName() + effect.slash->getEffectIdString()) <= 0)
                return false;
            if(!effect.from->isAlive() || !effect.to->isAlive() || effect.from->isNude())
                return false;
            int disc = room->askForCardChosen(effect.to, effect.from, "he", objectName());
            room->playSkillEffect(objectName(), 3);
            room->throwCard(disc);
            room->setPlayerMark(effect.to, objectName() + effect.slash->getEffectIdString(),
                                effect.to->getMark(objectName() + effect.slash->getEffectIdString()) - 1);
        }else if(event == CardFinished) {
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
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room* room = player->getRoom();
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card && use.card->isKindOf("Slash") && player->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName(), 1);
            if(!player->isNude()){
                int total = 0;
                QSet<const Card *> jilei_cards;
                QList<const Card *> handcards = player->getHandcards();
                foreach(const Card *card, handcards){
                    if(player->isJilei(card))
                        jilei_cards << card;
                }
                total = handcards.size() - jilei_cards.size() + player->getEquips().length();

                if(total <= 2){
                    player->throwAllEquips();
                    player->throwAllHandCards();
                }
                else
                    room->askForDiscard(player, objectName(), 2, false, true);
            }

            player->drawCards(2);

            int max = -1000;
            foreach(ServerPlayer *p, room->getAllPlayers())
                if(p->getHp() > max)
                    max = p->getHp();
            if (player->getHp() == max)
                return false;

            QList<ServerPlayer *> maxs;
            foreach(ServerPlayer *p, room->getAllPlayers()) {
                if (p->getHp() == max)
                    maxs << p;
                if (maxs.size() > 1)
                    return false;
            }
            ServerPlayer *mosthp = maxs.first();
            if (room->askForSkillInvoke(mosthp, objectName())) {
                int index = 2;
                if (mosthp->getGeneral()->isFemale())
                    index = 3;
                room->playSkillEffect(objectName(), index);
                
                QSet<const Card *> jilei_cards;
                QList<const Card *> handcards = mosthp->getHandcards();
                foreach(const Card *card, handcards){
                    if(mosthp->isJilei(card))
                        jilei_cards << card;
                }
                int total = handcards.size() - jilei_cards.size() + mosthp->getEquips().length();

                if(total <= 2){
                    player->throwAllEquips();
                    player->throwAllHandCards();
                }
                else 
                    room->askForDiscard(mosthp, objectName(), 2, false, true);
                mosthp->drawCards(2);
            }
        }

        return false;
    }
};

MizhaoCard::MizhaoCard(){
    will_throw = false;
    mute = true;
    once = true;
}

bool MizhaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void MizhaoCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(effect.card, false);
    Room *room = effect.from->getRoom();

    int index = 1;
    if (effect.to->getGeneralName().contains("liubei"))
        index = 2;
    room->playSkillEffect("mizhao", index);

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
    MizhaoViewAsSkill():ViewAsSkill("mizhao"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && !player->hasUsed("MizhaoCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
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

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room* room = player->getRoom();
        PindianStar pindian = data.value<PindianStar>();
        if (pindian->reason != objectName())
            return false;
        if (pindian->from_card->getNumber() == pindian->to_card->getNumber())
            return false;

        ServerPlayer *winner = pindian->isSuccess() ? pindian->from : pindian->to;
        ServerPlayer *loser = pindian->isSuccess() ? pindian->to : pindian->from;
        if (winner->canSlash(loser, false)) {
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

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const {
        return -2;
    }
};

class Jieyuan: public TriggerSkill{
public:
    Jieyuan(): TriggerSkill("jieyuan"){
        events << Predamaged << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room* room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        if(event == Predamage){
            if(damage.to && damage.to->isAlive()
               && damage.to->getHp() >= player->getHp() && damage.to != player && !player->isKongcheng())
                if(room->askForCard(player, ".black", "@JieyuanIncrease", data, CardDiscarded)){
                    room->playSkillEffect(objectName(), 1);

                    LogMessage log;
                    log.type = "#JieyuanIncrease";
                    log.from = player;
                    log.arg = QString::number(damage.damage);
                    log.arg2 = QString::number(++damage.damage);
                    room->sendLog(log);

                    data = QVariant::fromValue(damage);
                }
        }else if(event == Predamaged){
            if(damage.from && damage.from->isAlive()
               && damage.from->getHp() >= player->getHp() && damage.from != player && !player->isKongcheng())
                if(room->askForCard(player, ".red", "@JieyuanDecrease", data, CardDiscarded)){
                    room->playSkillEffect(objectName(), 2);

                    LogMessage log;
                    log.type = "#JieyuanDecrease";
                    log.from = player;
                    log.arg = QString::number(damage.damage);
                    log.arg2 = QString::number(--damage.damage);
                    room->sendLog(log);

                    if (damage.damage < 1){
                        LogMessage log;
                        log.type = "#ZeroDamage";
                        log.from = damage.from;
                        log.to << player;
                        room->sendLog(log);
                        return true;
                    }
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

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room* room = player->getRoom();
        if (!room->getMode().endsWith("p") && !room->getMode().endsWith("pd") && !room->getMode().endsWith("pz"))
            return false;
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.damage == NULL)
            return false;
        ServerPlayer *killer = dying.damage->from;
        if (killer == NULL || killer->isLord() || player->isLord() || player->getHp() > 0)
            return false;
        if (!killer->hasSkill(objectName()) || killer->getMark("@burnheart") == 0)
            return false;
        room->setPlayerFlag(player, "FenxinTarget");
        if (room->askForSkillInvoke(killer, objectName(), QVariant::fromValue(player))) {
            room->playSkillEffect(objectName());
            room->broadcastInvoke("animate", "lightbox:$fenxin");
            room->getThread()->delay(1500);
            killer->loseMark("@burnheart");
            QString role1 = killer->getRole();
            killer->setRole(player->getRole());
            room->setPlayerProperty(killer, "role", player->getRole());
            player->setRole(role1);
            room->setPlayerProperty(player, "role", role1);
        }
        room->setPlayerFlag(player, "-FenxinTarget");
        return false;
    }
};

MixinCard::MixinCard(){
    will_throw = false;
    mute = true;
}

bool MixinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void MixinCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *source = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = source->getRoom();
    room->playSkillEffect("mixin", 1);
    target->obtainCard(this, false);
    QList<ServerPlayer *> others;
    foreach(ServerPlayer *p, room->getOtherPlayers(target))
        if(target->canSlash(p, false))
            others << p;

    if(others.isEmpty())
        return;

    ServerPlayer *target2 = room->askForPlayerChosen(source, others, "mixin");
    LogMessage log;
    log.type = "#CollateralSlash";
    log.from = source;
    log.to << target2;
    room->sendLog(log);
    room->setPlayerFlag(target, "jiefanUsed");
    const Card *card = room->askForCard(target, "slash", "@mixin:" + target2->objectName(), QVariant());
    if(card){
        room->setPlayerFlag(target, "-jiefanUsed");
        room->playSkillEffect("mixin", 2);
        CardUseStruct card_use;
        card_use.from = target;
        card_use.to << target2;
        card_use.card = card;
        room->useCard(card_use, false);
    }
    else{
        room->setPlayerFlag(target, "-jiefanUsed");
        room->playSkillEffect("mixin", 3);
        QList<int> card_ids = target->handCards();
        room->fillAG(card_ids, target2);
        int cdid = room->askForAG(target2, card_ids, false, objectName());
        room->obtainCard(target2, cdid, false);
        target2->invoke("clearAG");
    }
}

class Mixin:public OneCardViewAsSkill{
public:
    Mixin():OneCardViewAsSkill("mixin"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("MixinCard");
    }

    virtual bool viewFilter(const CardItem *card) const{
        return !card->isEquipped();
    }

    virtual const Card *viewAs(CardItem *originalCard) const{
        MixinCard *card = new MixinCard;
        card->addSubcard(originalCard->getFilteredCard());
        return card;
    }
};

class Cangni: public TriggerSkill{
public:
    Cangni():TriggerSkill("cangni"){
        events << PhaseChange << CardLostDone << CardGotDone;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room* room = player->getRoom();
        if(event == PhaseChange && player->getPhase() == Player::Discard && player->askForSkillInvoke(objectName())) {
            QStringList choices;
            choices << "draw";
            if(player->isWounded())
                choices << "recover";

            QString choice;
            if(choices.size() == 1)
                choice = choices.first();
            else
                choice = room->askForChoice(player, objectName(), choices.join("+"));

            if(choice == "recover") {
                RecoverStruct recover;
                recover.who = player;
                room->recover(player, recover);
            }
            else
                player->drawCards(2);

            room->playSkillEffect("cangni", 1);
            player->turnOver();
            return false;
        }
        else if(event == CardLostDone || event == CardGotDone) {
            if(player->getPhase() != Player::NotActive || player->faceUp())
                return false;

            ServerPlayer *target = room->getCurrent();
            if(target->isDead())
                return false;
            if(event == CardLostDone && !target->isNude() && player->askForSkillInvoke(objectName())) {
                room->playSkillEffect("cangni", 3);
                room->askForDiscard(target, objectName(), 1, 1, false, true);
                return false;
            }

            if(event == CardGotDone && !target->hasFlag("cangni_used") && player->askForSkillInvoke(objectName())) {
                room->setPlayerFlag(target, "cangni_used");
                room->playSkillEffect("cangni", 2);
                target->drawCards(1);
            }
        }
        return false;
    }
};                    

DuyiCard::DuyiCard(){
    target_fixed = true;
    mute = true;
}

void DuyiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    QList<int> card_ids = room->getNCards(1);
    int id = card_ids.first();
    room->fillAG(card_ids, NULL);
    room->getThread()->delay();
    ServerPlayer *target = room->askForPlayerChosen(source, room->getAlivePlayers(), "duyi");
    const Card *card = Sanguosha->getCard(id);
    target->obtainCard(card);
    if(card->isBlack()) {
        target->jilei(".|.|.|hand");
        target->invoke("jilei", ".|.|.|hand");
        room->setPlayerFlag(target, "duyi_target");
        LogMessage log;
        log.type = "#duyi_eff";
        log.from = source;
        log.to << target;
        log.arg = "duyi";
        room->sendLog(log);
        room->playSkillEffect("duyi", 1);
    }
    else
        room->playSkillEffect("duyi", 2);

    room->getThread()->delay();
    foreach(ServerPlayer *p, room->getPlayers())
        p->invoke("clearAG");
}

class DuyiViewAsSkill:public ZeroCardViewAsSkill{
public:
    DuyiViewAsSkill():ZeroCardViewAsSkill("duyi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DuyiCard");
    }

    virtual const Card *viewAs() const{
        return new DuyiCard;
    }
};

class Duyi:public TriggerSkill{
public:
    Duyi():TriggerSkill("duyi"){
        view_as_skill = new DuyiViewAsSkill;
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room* room = player->getRoom();
        ServerPlayer *splayer = room->findPlayerBySkillName(objectName());
        if(!splayer)
            return false;

        if(splayer->getPhase() == Player::Discard)
            if(splayer->hasFlag("duyi_target")) {
                splayer->jilei(".");
                splayer->invoke("jilei", ".");
                room->setPlayerFlag(splayer, "-duyi_target");
            }

        if(splayer->getPhase() == Player::NotActive)
            foreach(ServerPlayer *p, room->getAlivePlayers())
                if(p->hasFlag("duyi_target")) {
                    p->jilei(".");
                    p->invoke("jilei", ".");
                    room->setPlayerFlag(p, "-duyi_target");
                    LogMessage log;
                    log.type = "#duyi_clear";
                    log.from = p;
                    log.arg = objectName();
                    room->sendLog(log);
                }

        return false;
    }
};

class Duanzhi: public TriggerSkill{
public:
    Duanzhi(): TriggerSkill("duanzhi") {
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const {
        Room* room = player->getRoom();
        ServerPlayer *splayer = room->findPlayerBySkillName(objectName());
        if(!splayer || splayer == player)
            return false;

        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->getTypeId() == Card::Skill || !use.to.contains(splayer))
            return false;
        
        if(player->askForSkillInvoke(objectName())) {
            room->setPlayerFlag(player, "DuanzhiTarget_InTempMoving");
            ServerPlayer *target = use.from;
            DummyCard *dummy = new DummyCard;
            QList<int> card_ids;
            QList<Player::Place> original_places;
            for(int i = 0; i < 2; i++) {
                if(player->isNude())
                    break;
                if(room->askForChoice(player, objectName(), "discard+cancel") == "cancel")
                    break;
                card_ids << room->askForCardChosen(player, target, "he", objectName());
                original_places << room->getCardPlace(card_ids[i]);
                dummy->addSubcard(card_ids[i]);
                target->addToPile("#duanzhi", card_ids[i], false);
            }

            if(dummy->subcardsLength() > 0)
                for (int i = 0; i < dummy->subcardsLength(); i++) {
                    room->moveCardTo(Sanguosha->getCard(card_ids[i]), target, original_places[i], false);
                    room->throwCard(dummy);
                    dummy->deleteLater();
                }
            
            room->setPlayerFlag(player, "-DuanzhiTarget_InTempMoving");
            room->loseHp(player);
        }
        return false;
    }
};

class DuanzhiAvoidTriggeringCardsMove: public TriggerSkill{
public:
    DuanzhiAvoidTriggeringCardsMove():TriggerSkill("#duanzhi"){
        events << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority() const{
        return 10;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        Room* room = player->getRoom();
        foreach(ServerPlayer *p, room->getAllPlayers())
            if(p->hasFlag("DuanzhiTarget_InTempMoving"))
                return true;
        return false;
    }
};

FengyinCard::FengyinCard(){
    target_fixed = true;
    will_throw = false;
    mute = true;
}

void FengyinCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    ServerPlayer *target = room->getCurrent();
    target->obtainCard(this);
    room->playSkillEffect("fengyin");
    room->setPlayerFlag(target, "fengyin_target");
}

class FengyinViewAsSkill:public OneCardViewAsSkill{
public:
    FengyinViewAsSkill():OneCardViewAsSkill("fengyin"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@fengyin";
    }

    virtual bool viewFilter(const CardItem *card) const{
        return card->getCard()->isKindOf("Slash");
    }

    virtual const Card *viewAs(CardItem *originalCard) const{
        FengyinCard *card = new FengyinCard;
        card->addSubcard(originalCard->getFilteredCard());
        return card;
    }
};

class Fengyin:public TriggerSkill{
public:
    Fengyin():TriggerSkill("fengyin"){
        view_as_skill = new FengyinViewAsSkill;
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room* room = player->getRoom();
        ServerPlayer *splayer = room->findPlayerBySkillName(objectName());
        if(!splayer)
            return false;

        if(player->getPhase() == Player::Start){
            if(player->getHp() > splayer->getHp())
                room->askForUseCard(splayer, "@@fengyin", "@fengyin");
            if(player->hasFlag("fengyin_target")){
                player->skip(Player::Play);
                player->skip(Player::Discard);
            }
        }
        return false;
    }
};

class ChizhongKeep: public MaxCardsSkill{
public:
    ChizhongKeep():MaxCardsSkill("chizhong"){
    }

    virtual int getExtra(const Player *target) const{
        if(target->hasSkill(objectName()))
            return target->getLostHp();
        else
            return 0;
    }
};

class Chizhong: public TriggerSkill{
public:
    Chizhong():TriggerSkill("#chizhong"){
        events << Death << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room* room = player->getRoom();
        ServerPlayer *splayer = room->findPlayerBySkillName(objectName());
        if(!splayer)
            return false;

        if(event == PhaseChange && splayer == player && player->getPhase() == Player::Discard) {
            if(player->getHandcardNum() > player->getHp()){
                LogMessage log;
                log.type = "#Chizhong";
                log.from = splayer;
                log.arg = "chizhong";
                room->sendLog(log);
                room->playSkillEffect("chizhong", 1);
            }
            return false;
        }

        if(event != Death || player == splayer)
            return false;

        room->setPlayerProperty(splayer, "maxhp", splayer->getMaxHp()+1);
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = splayer;
        log.arg = "chizhong";
        room->sendLog(log);
        room->playSkillEffect("chizhong", 2);

        return false;
    }
};

AssassinsPackage::AssassinsPackage():Package("assassins"){
    General *fuhuanghou = new General(this, "fuhuanghou", "qun", 3, false);
    fuhuanghou->addSkill(new Mixin);
    fuhuanghou->addSkill(new Cangni);

    General *jiben = new General(this, "jiben", "qun", 3);
    jiben->addSkill(new Duyi);
    jiben->addSkill(new Duanzhi);
    jiben->addSkill(new DuanzhiAvoidTriggeringCardsMove);
    related_skills.insertMulti("duanzhi", "#duanzhi");

    General *fuwan = new General(this, "fuwan", "qun", 3);
    fuwan->addSkill(new Fengyin);
    fuwan->addSkill(new ChizhongKeep);
    fuwan->addSkill(new Chizhong);
    related_skills.insertMulti("chizhong", "#chizhong");

    General *mushun = new General(this, "mushun", "qun");
    mushun->addSkill(new Moukui);

    General *hanxiandi = new General(this, "hanxiandi", "qun", 3);
    hanxiandi->addSkill(new Tianming);
    hanxiandi->addSkill(new Mizhao);

    General *lingju = new General(this, "lingju", "qun", 3, false);
    lingju->addSkill(new Jieyuan);
    lingju->addSkill(new Fenxin);
    lingju->addSkill(new MarkAssignSkill("@burnheart", 1));
    
    addMetaObject<MizhaoCard>();
    addMetaObject<MixinCard>();
    addMetaObject<DuyiCard>();
    addMetaObject<FengyinCard>();
}

//Olympics
JisuCard::JisuCard(){
}

bool JisuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select == Self;
}

void JisuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName(skill_name);
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to = targets;
    room->useCard(use);
}

class JisuViewAsSkill: public ZeroCardViewAsSkill{
public:
    JisuViewAsSkill():ZeroCardViewAsSkill("jisu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@jisu";
    }

    virtual const Card *viewAs() const{
        return new JisuCard;
    }
};

class Jisu: public PhaseChangeSkill{
public:
    Jisu():PhaseChangeSkill("jisu"){
        view_as_skill = new JisuViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *poem) const{
        Room *room = poem->getRoom();
        if(poem->getPhase() == Player::Judge){
            if(room->askForUseCard(poem, "@@jisu", "@jisu")){
                poem->skip(Player::Draw);
                return true;
            }
        }
        return false;
    }
};

class Shuiyong: public TriggerSkill{
public:
    Shuiyong():TriggerSkill("shuiyong"){
        events << Predamaged;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Fire){
            Room *room = player->getRoom();
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#ShuiyongProtect";
            log.from = player;
            log.arg2 = QString::number(damage.damage);
            log.arg = objectName();
            room->sendLog(log);

            return true;
        }else
            return false;
    }
};

class Shuijian:public DrawCardsSkill{
public:
    Shuijian():DrawCardsSkill("shuijian"){
        frequency = Frequent;
    }

    virtual int getDrawNum(ServerPlayer *sunyang, int n) const{
        Room *room = sunyang->getRoom();
        if(room->askForSkillInvoke(sunyang, objectName())){
            room->playSkillEffect(objectName());
            int x = sunyang->getEquips().count();
            return n + x/2 + 1;
        }else
            return n;
    }
};

OlympicsPackage::OlympicsPackage():Package("olympics"){
    General *yeshiwen = new General(this, "yeshiwen", "wu", 3, false);
    yeshiwen->addSkill(new Jisu);
    yeshiwen->addSkill(new Shuiyong);

    General *sunyang = new General(this, "sunyang", "wu");
    sunyang->addSkill(new Shuijian);
}

ADD_PACKAGE(Assassins)
ADD_PACKAGE(Olympics)
