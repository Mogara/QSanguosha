#include "skill.h"
#include "clientplayer.h"
#include "standard.h"
#include "tigerfly.h"
#include "engine.h"

class Shemi: public ViewAsSkill {
public:
    Shemi():ViewAsSkill("shemi") {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        int hp = Self->getHp();
        if (hp <= 2) {
            return selected.length() < 1;
        }else {
            return selected.length() < 2;
        };
    };

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ShemiAG");
    };

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        int hp = Self->getHp();
        if ((hp <= 2 && cards.length() != 1) || (hp > 2 && cards.length() != 2))
            return NULL;

        Card *amazing_grace = new AmazingGrace(Card::SuitToBeDecided, -1);
        amazing_grace->addSubcards(cards);
        amazing_grace->setSkillName(objectName());
        return amazing_grace;
    };

};

class Kuanhui: public TriggerSkill{
public:
    Kuanhui(): TriggerSkill("kuanhui") {
        events << TargetConfirmed << CardEffected << SlashEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *selfplayer = room->findPlayerBySkillName(objectName());
        if (triggerEvent == TargetConfirmed && TriggerSkill::triggerable(player)){
            CardUseStruct use = data.value<CardUseStruct>();
            const Card *card = use.card;
            if (card->isKindOf("SkillCard"))
                return false;
            if (use.to.length() >= 2){
                room->setTag("kuanhui_user", card->toString());
                if (room->askForSkillInvoke(selfplayer, "kuanhui", data)){
                    room->broadcastSkillInvoke("kuanhui", 1);
                    JudgeStruct judge;
                    judge.pattern = ".|diamond";
                    judge.good = false;
                    judge.reason = objectName();
                    judge.who = selfplayer;
                    room->judge(judge);
                    room->getThread()->delay(1000);
                    if (judge.isGood()){
                        room->broadcastSkillInvoke("kuanhui", 2);
                        ServerPlayer *target = room->askForPlayerChosen(selfplayer, use.to, objectName());
                        room->setPlayerFlag(target, "kuanhuitarget");
                        room->setPlayerMark(target, "kuanhuiCardId", card->getId() + 1);
                        LogMessage log;
                        log.type = "#Kuanhui1";
                        log.from = selfplayer;
                        log.arg = objectName();
                        log.to.append(target);
                        room->sendLog(log);
                    }
                    else
                        room->broadcastSkillInvoke("kuanhui", 3);
                }
                room->removeTag("kuanhui_user");
            }
        }
        else if (triggerEvent == CardEffected){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if (effect.card->isKindOf("Slash"))
                return false;
            if (effect.to != NULL && effect.to->hasFlag("kuanhuitarget")
                    && (effect.card->getId() == effect.to->getMark("kuanhuiCardId") - 1)){
                room->setPlayerFlag(effect.to, "-kuanhuitarget");
                room->setPlayerMark(effect.to, "kuanhuiCardId", 0);
                LogMessage log;
                log.type = "#DanlaoAvoid";
                log.arg2 = objectName();
                log.from = effect.to;
                log.arg = effect.card->objectName();
                room->sendLog(log);
                return true;
            }
        }
        else{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if (effect.to != NULL && effect.to->hasFlag("kuanhuitarget")
                    && (effect.slash->getId() == effect.to->getMark("kuanhuiCardId") - 1)){
                room->setPlayerFlag(effect.to, "-kuanhuitarget");
                room->setPlayerMark(effect.to, "kuanhuiCardId", 0);
                LogMessage log;
                log.type = "#DanlaoAvoid";
                log.arg2 = objectName();
                log.from = effect.to;
                log.arg = effect.slash->objectName();
                room->sendLog(log);
                return true;
            }
        }
        return false;
    }
};

class Hongliang: public MasochismSkill{
public:
    Hongliang(): MasochismSkill("hongliang$"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasLordSkill(objectName()) && target->isAlive();
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> lieges = room->getLieges("wei", player);
        if (lieges.length() == 0)
            return;
        room->setTag("honglianglord", QVariant::fromValue(player));
        foreach (ServerPlayer *p, lieges)
            if (!p->isNude()){
                const Card *card = room->askForCard(p, ".|.|.|.|.", "@HongliangGive", QVariant::fromValue(damage), Card::MethodNone);
                if (card != NULL){
                    if (!player->isLord() && player->hasSkill("weidi"))
                        room->broadcastSkillInvoke("weidi");
                    else
                        room->broadcastSkillInvoke("hongliang");
                    room->getThread()->delay(1000);
                    CardMoveReason reason;
                    reason.m_reason = CardMoveReason::S_REASON_GIVE;
                    reason.m_playerId = player->objectName();
                    reason.m_targetId = p->objectName();
                    room->moveCardTo(card, p, player, Player::PlaceHand, reason);
                    p->drawCards(1);
                }
            }
        room->setTag("honglianglord", QVariant());
    }
};

PozhenCard::PozhenCard(){
    mute = true;
}

bool PozhenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    if (targets.length() == 0)
        return to_select->hasEquip();
    return false;
}

QString PozhenCard::suittb(Card::Suit s) const{
    switch (s){
        case Card::Club:
            return "club";
            break;
        case Card::Diamond:
            return "diamond";
            break;
        case Card::Heart:
            return "heart";
            break;
        case Card::Spade:
            return "spade";
            break;
        default:
            return "unknown";
            break;
    }
    return "unknown";
}

void PozhenCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *dest = effect.to;
    Room *room = dest->getRoom();
    room->setTag("pozhen_dest", QVariant::fromValue(dest));
    room->broadcastSkillInvoke("pozhen", 1);
    ServerPlayer *source = effect.from;
    room->setTag("pozhen_source", QVariant::fromValue(source));
    Card::Suit suit = room->askForSuit(source, "pozhen");
    LogMessage log;
    log.type = "#pozhensuit";
    log.from = source;
    log.arg = suittb(suit);
    room->sendLog(log);
    QString pattern = ".|" + suittb(suit) + "|.|hand|.";
    if (!dest->isKongcheng() &&
            room->askForCard(dest, pattern, "@pozhen:" + source->objectName() + "::" + suittb(suit), QVariant(), "pozhen")){
        room->broadcastSkillInvoke("pozhen", 2);
        source->setFlags("Global_EndPlayPhase");
    }
    else {
        room->broadcastSkillInvoke("pozhen", 3);
        QList<int> idlist;
        QList<const Card *> equips = dest->getEquips();
        foreach (const Card *equip, equips)
            idlist << equip->getId();
        CardsMoveStruct move;
        move.card_ids = idlist;
        move.to = dest;
        move.to_place = Player::PlaceHand;
        room->moveCards(move, true);
        source->drawCards(1);
    }
    room->removeTag("pozhen_dest");
    room->removeTag("pozhen_source");
}

class Pozhen: public ZeroCardViewAsSkill{
public:
    Pozhen(): ZeroCardViewAsSkill("pozhen"){
    }

    virtual const Card *viewAs() const{
        return new PozhenCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("PozhenCard");
    }
};

class Huaming: public TriggerSkill{
public:
    Huaming(): TriggerSkill("huaming"){
        frequency = Limited;
        events << Death;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if (player->objectName() == death.who->objectName() && death.who->hasSkill(objectName()))
            if (death.damage && death.damage->from && death.damage->from->objectName() != player->objectName()){
                room->setTag("huamingkiller", QVariant::fromValue(death.damage->from));
                room->setTag("shanfu", QVariant::fromValue(death.who));
            }

        return false;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }
};

class Poli: public PhaseChangeSkill{
public:
    Poli(): PhaseChangeSkill("poli"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if (player->getPhase() == Player::Finish){
            Room *room = player->getRoom();
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            QList<ServerPlayer *> targets;
            foreach (ServerPlayer *p, players){
                if (p->inMyAttackRange(player))
                    targets << p;
            }
            if (targets.length() >= 3){
                QString choice;
                if (!player->isNude())
                    choice = room->askForChoice(player, objectName(), "discard+changehero");
                else
                    choice = "changehero";
                if (choice == "discard"){
                    room->broadcastSkillInvoke(objectName(), 1);
                    room->askForDiscard(player, objectName(), 1, 1, false, true);
                }
                else{
                    room->broadcastSkillInvoke(objectName(), 2);
                    room->doLightbox("$PoliAnimate", 5000);
                    room->loseMaxHp(player);
                    room->drawCards(player, 2);
                    room->showAllCards(player);
                    QStringList l;
                    l << "wuyan" << "jujian" << "-pozhen" << "-huaming" << "-poli";
                    room->handleAcquireDetachSkills(player, l);
                }
            }
        }
        return false;
    }
};

class Zhuixi: public TriggerSkill{
public:
    Zhuixi(): TriggerSkill("zhuixi"){
        frequency = Frequent;
        events << Damage << EventPhaseChanging << FinishJudge;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == Damage){
            DamageStruct damage = data.value<DamageStruct>();
            ServerPlayer *victim = damage.to;
            if (player->getPhase() == Player::Play)
                if (damage.card != NULL && damage.card->isKindOf("Slash"))
                    if (damage.from == player)
                        if (player->distanceTo(victim) == 1)
                            if (!damage.chain && !damage.transfer)
                                if (room->askForSkillInvoke(player, objectName())){
                                    room->broadcastSkillInvoke(objectName(), 1);
                                    JudgeStruct judge;
                                    judge.pattern = ".|black";
                                    judge.good = true;
                                    judge.reason = objectName();
                                    judge.who = player;
                                    room->judge(judge);
                                    room->getThread()->delay(1000);
                                    if (judge.isGood()){
                                        room->addPlayerMark(player, "zhuixi_extra");
                                        room->broadcastSkillInvoke(objectName(), 3);
                                    }
                                    else
                                        room->broadcastSkillInvoke(objectName(), 2);
                                }
        }
        else if (triggerEvent == EventPhaseChanging){
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.from == Player::Play){
                int x = player->getMark("zhuixi_extra");
                if (x > 0)
                    room->setPlayerMark(player, "zhuixi_extra", 0);
            }
        }
        else{
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (judge->reason == objectName())
                player->obtainCard(judge->card);
        }
        return false;
    }
};
class ZhuixiTm: public TargetModSkill{
public:
    ZhuixiTm(): TargetModSkill("#zhuixitm"){
        pattern = "Slash";
    }

    virtual int getResidueNum(const Player *from, const Card *) const{
        int num = from->getMark("zhuixi_extra");
        if (from->hasSkill("zhuixi"))
            return num;
        return 0;
    }
};
class ZhuixiRm: public TriggerSkill{
public:
    ZhuixiRm(): TriggerSkill("#zhuixirm"){
        events << EventLoseSkill;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (data.toString() == "zhuixi")
            room->setPlayerMark(player, "zhuixi_extra", 0);
        return false;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }
};

class Jisi: public TriggerSkill{
public:
    Jisi(): TriggerSkill("jisi"){
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *current = room->getCurrent();
        CardEffectStruct effect = data.value<CardEffectStruct>();
        ServerPlayer *owner = room->findPlayerBySkillName(objectName());
        if (owner == NULL)
            return false;
        if (effect.card->isKindOf("TrickCard"))
            if (current != owner)
                if (!owner->isKongcheng() && !current->isKongcheng())
                    if (room->askForSkillInvoke(owner, objectName(), data)){
                        room->broadcastSkillInvoke(objectName(), 1);
                        bool pindian = owner->pindian(current, "jisi", NULL);
                        if (pindian){
                            room->broadcastSkillInvoke(objectName(), qrand() % 2 + 2);
                            return true;
                        }
                        else
                            room->broadcastSkillInvoke(objectName(), 4);
                    }
        return false;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }
};

class Zhuanquan: public PhaseChangeSkill{
public:
    Zhuanquan(): PhaseChangeSkill("zhuanquan"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if (player->getPhase() == Player::Discard)
            if (player->getHandcardNum() > player->getMaxCards()){
                Room *room = player->getRoom();
                ServerPlayer *owner = room->findPlayerBySkillName(objectName());
                if (owner != NULL && room->askForSkillInvoke(owner, objectName())){
                    room->broadcastSkillInvoke(objectName());
                    int x = player->getHandcardNum() - player->getMaxCards();
                    //ToAsk: 这个弃牌是一张一张弃？还是一起弃？要是一起弃的话要不要加Flag和FakeMoveSkill？
                    for (int i = 0; i < x; ++i){
                        int card = room->askForCardChosen(owner, player, "h", objectName());
                        //ToAsk: CardMoveReason哪里去了？
                        room->throwCard(card, player, owner);
                    }
                }
            }
        return false;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && !target->hasSkill(objectName()) && target->isAlive();
    }
};

class NeoAocai: public TriggerSkill{
public:
    NeoAocai(): TriggerSkill("neoaocai"){
        events << EventPhaseChanging;
        frequency = Limited;
        limit_mark = "@neoaocai"; //Todo：加入这个Mark的图片
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to == Player::NotActive){
            QList<ServerPlayer *> others = room->getOtherPlayers(player);
            QList<ServerPlayer *> count;
            foreach(ServerPlayer *p, others)
                if (p->getHandcardNum() > player->getHandcardNum())
                    count << p;
            if (count.length() > 0)
                if (room->askForSkillInvoke(player, objectName())){
                    room->broadcastSkillInvoke(objectName());
                    room->doLightbox("$neoaocai", 5000); //ToAsk: 这里没有Animate的翻译和图像文件，这里只能显示大黑框
                    player->loseMark("@neoaocai");
                    foreach(ServerPlayer *victim, count){
                        const Card *card = room->askForCard(victim, ".|.|.|.|.", "@neoaocaigive", QVariant(), Card::MethodNone);
                        if (card != NULL)
                            room->obtainCard(player, card, false);
                        else
                            room->damage(DamageStruct(objectName(), player, victim));
                    }
                    room->acquireSkill(player, "zhuanquan");
                }

        }
        return false;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@neoaocai") > 0;
    }
};

TigerFlyPackage::TigerFlyPackage(): Package("tigerfly") {
    General *caorui = new General(this, "caorui$", "wei", 3);
    caorui->addSkill(new Shemi);
    caorui->addSkill(new Kuanhui);
    caorui->addSkill(new Hongliang);

    General *shanfu = new General(this, "shanfu", "shu", 4);
    shanfu->addSkill(new Pozhen);
    shanfu->addSkill(new Huaming);
    shanfu->addSkill(new Poli);

    General *tf_spmadai = new General(this, "tf_sp_madai", "shu", 4);
    tf_spmadai->addSkill(new Zhuixi);
    tf_spmadai->addSkill(new ZhuixiTm);
    tf_spmadai->addSkill(new ZhuixiRm);
    tf_spmadai->addSkill("mashu");
    related_skills.insertMulti("zhuixi", "#zhuixirm");
    related_skills.insertMulti("zhuixi", "#zhuixitm");

    General *neo_zhugeke = new General(this, "neo_zhugeke", "wu", 4);
    neo_zhugeke->addSkill(new Jisi);
    neo_zhugeke->addSkill(new NeoAocai);
    neo_zhugeke->addRelateSkill("zhuanquan");

    addMetaObject<PozhenCard>();

    skills << new Zhuanquan;
};

ADD_PACKAGE(TigerFly)
