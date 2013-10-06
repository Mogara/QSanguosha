#include "skill.h"
#include "clientplayer.h"
#include "standard.h"
#include "tigerfly.h"
#include "engine.h"
#include "settings.h"
#include "jsonutils.h"
#include "protocol.h"

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
        source->setFlags("Global_PlayPhaseTerminated");
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
        if (player == death.who && death.who->hasSkill(objectName()))
            if (death.damage && death.damage->from && death.damage->from != death.who){
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

//-----------------old zhugeke skills ------------------
/*

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
*/

//-----------------old zhugeke skills end ------------------

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
                        //ToDo: 忘了问了……
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
                        QString choice = "give";
                        if (victim->getCardCount(true) >= 2)
                            choice = room->askForChoice(victim, objectName(), "discard+give", QVariant());
                        if (choice == "give"){  //change this skill according to Ling Tianyi
                            const Card *card = room->askForExchange(victim, objectName() + "-give", 1, true, "@neoaocaigive");
                            room->obtainCard(player, card, false);
                        }
                        else {
                            room->askForDiscard(victim, objectName() + "-discard", 2, 2, false, true, "@neoaocaidiscard");
                        }
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


JisiCard::JisiCard(): SkillCard(){
    target_fixed = true;
    mute = true;
}

const Card *JisiCard::validateInResponse(ServerPlayer *player) const{
    Room *room = player->getRoom();
    room->setPlayerMark(player, "jisiused", 1);
    ServerPlayer *current = room->getCurrent();
    if (!current || current->isDead() || current->getPhase() == Player::NotActive || current == player)
        return NULL;
    
    room->broadcastSkillInvoke("jisi", 1);

    if (player->pindian(current, "jisi")){
        Nullification *nul = new Nullification(Card::NoSuit, 0);
        nul->setSkillName("_jisi");
        return nul;
    }
    else
        room->broadcastSkillInvoke("jisi", 4);

    return NULL;
}

class JisiVS: public ZeroCardViewAsSkill{
public:
    JisiVS(): ZeroCardViewAsSkill("jisi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return player->getMark("jisiused") == 0 && pattern == "nullification" && player->getPhase() == Player::NotActive;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        return (player->getMark("jisiused") == 0 && !player->isKongcheng() && player->getPhase() == Player::NotActive);
    }

    virtual const Card *viewAs() const{
        return new JisiCard;
    }
};

class Jisi: public PhaseChangeSkill{
public:
    Jisi(): PhaseChangeSkill("jisi"){
        view_as_skill = new JisiVS;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (!target->getPhase() != Player::RoundStart)
            return false;

        Room *room = target->getRoom();
        QList<ServerPlayer *> zhugekes = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *zhugeke, zhugekes){
            if (zhugeke->getMark("jisiused") != 0)
                room->setPlayerMark(zhugeke, "jisiused", 0);
        }

        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *player, const Card *card) const{
        return qrand() % 2 + 2;
    }
};

class Xiongjie: public TriggerSkill{
public:
    Xiongjie(): TriggerSkill("xiongjie"){
        frequency = Compulsory;
        events << DamageCaused;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        int d = damage.damage;
        damage.damage = qMax(player->getMark("@xiongjie"), 1);
        data = QVariant::fromValue(damage);

        LogMessage l;
        l.type = "#xiongjiedamage";
        l.from = player;
        l.to << damage.to;
        l.arg = QString::number(damage.damage);
        room->sendLog(l);

        if (damage.damage > d)
            room->broadcastSkillInvoke(objectName());

        return false;
    }
};
class XiongjieCount: public TriggerSkill{
public:
    XiongjieCount(): TriggerSkill("#xiongjie-count"){
        events << Damage << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == Damage){
            DamageStruct damage = data.value<DamageStruct>();
            room->setPlayerMark(player, "xiongjiedamage", damage.damage + player->getMark("xiongjiedamage"));
        }
        else {
            if (player->getPhase() == Player::NotActive)
                room->setPlayerMark(player, "xiongjiedamage", 0);
        }

        if (player->hasSkill("xiongjie")){
            int xiongjiemark = player->getMark("@xiongjie");
            int xiongjiedamage = player->getMark("xiongjiedamage");
            if (xiongjiemark > xiongjiedamage)
                player->loseMark("@xiongjie", xiongjiemark - xiongjiedamage);
            else if (xiongjiedamage > xiongjiemark)
                player->gainMark("@xiongjie", xiongjiedamage - xiongjiemark);
        }

        return false;
    }
};
class XiongjieAcDe: public TriggerSkill{
public:
    XiongjieAcDe(): TriggerSkill("#xiongjie-acquire-detach"){
        events << EventAcquireSkill << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data) const{
        if (data.toString() != "xiongjie")
            return false;

        if (triggerEvent == EventAcquireSkill){
            if (player->getMark("xiongjiedamage") > 0)
                player->gainMark("@xiongjie", player->getMark("xiongjiedamage"));
        }
        else {
            if (player->getMark("@xiongjie") > 0)
                player->loseMark("@xiongjie", player->getMark("xiongjie"));
        }

        return false;
    }
};

TushouGiveCard::TushouGiveCard(){
    will_throw = false;
    handling_method = Card::MethodNone;
    mute = true;
}

bool TushouGiveCard::targetFilter(const QList<const Player *> &selected, const Player *to_select, const Player *) const{
    if (selected.length() > 0)
        return false;

    int maxhp = -1000;
    QList<const Player *> maxhps;
    foreach(const Player *p, Self->getAliveSiblings()){
        if (p->getHp() > maxhp){
            maxhp = p->getHp();
            maxhps.clear();
            maxhps << p;
        }
        else if (p->getHp() == maxhp)
            maxhps << p;
    }

    return (maxhps.contains(to_select));
}

void TushouGiveCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);
}

class TushouGiveVS: public OneCardViewAsSkill{
public:
    TushouGiveVS(): OneCardViewAsSkill("tushou"){
        filter_pattern = ".";
        response_pattern = "@@tushou";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        TushouGiveCard *c = new TushouGiveCard;
        c->addSubcard(originalCard);
        return c;
    }
};

class Tushou: public TriggerSkill{
public:
    Tushou(): TriggerSkill("tushou"){
        events << EventPhaseStart << DamageInflicted << DamageCaused;
        view_as_skill = new TushouGiveVS;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        switch (triggerEvent){
            case (EventPhaseStart):{
                if (player->getPhase() != Player::Start || player->isNude())
                    return false;

                QStringList TSchoices;
                TSchoices << "cancel" << "give";

                if (player->getCardCount(true) >= 2)
                    TSchoices << "discard";

                QString TSchoice = room->askForChoice(player, objectName(), TSchoices.join("+"));

                if (TSchoice == "give" && room->askForUseCard(player, "@@tushou", "@tushou-give", -1, Card::MethodNone)){
                    room->broadcastSkillInvoke(objectName(), 1);
                    if (player->getMaxHp() - player->getHandcardNum() > 0)
                        player->drawCards(player->getMaxHp() - player->getHandcardNum(), objectName());
                    room->setPlayerFlag(player, "tushou_give");
                }
                else if (TSchoice == "discard" && room->askForDiscard(player, objectName(), 2, 2, true, true, "@tushou-discard")){
                    room->broadcastSkillInvoke(objectName(), 2);
                    RecoverStruct r;
                    r.who = player;
                    room->recover(player, r);
                    room->setPlayerFlag(player, "tushou_disc");
                }
                break;
            }
            case (DamageCaused):{
                if (player->hasFlag("tushou_give")){
                    LogMessage l;
                    l.type = "#TushouAvoid";
                    l.from = player;
                    l.to << data.value<DamageStruct>().to;
                    room->sendLog(l);

                    room->broadcastSkillInvoke(objectName(), 3);

                    return true;
                }
                break;
            }
            case (DamageInflicted):{
                if (player->hasFlag("tushou_disc")){
                    LogMessage l;
                    l.type = "#TushouAvoid";
                    l.from = data.value<DamageStruct>().from;
                    l.to << player;
                    room->sendLog(l);

                    room->broadcastSkillInvoke(objectName(), 4);

                    return true;
                }
                break;
            }
            default:
                break;
        }
        return false;
    }
};

class Kangdao: public TriggerSkill{
public:
    Kangdao(): TriggerSkill("kangdao"){
        events << BeforeCardsMove;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == NULL || move.from == player)
            return false;

        if (move.to_place == Player::PlaceTable){
            foreach(int card_id, move.card_ids){
                const Card *card = Sanguosha->getCard(card_id);
                if (Sanguosha->getCard(card_id)->getTypeId() == Card::TypeEquip
                        && room->getCardOwner(card_id) == move.from
                        && (room->getCardPlace(card_id) == Player::PlaceHand
                        || room->getCardPlace(card_id) == Player::PlaceEquip))
                    card->setFlags("kangdaomove");
            }
        }
        else if (move.to_place == Player::DiscardPile){
            CardMoveReason reason = move.reason;
            QList<int> card_ids;
            foreach(int card_id, move.card_ids){
                const Card *cd = Sanguosha->getCard(card_id);
                if (cd->getTypeId() == Card::TypeEquip){
                    if (cd->hasFlag("kangdaomove") ||
                            (room->getCardOwner(card_id) == move.from
                            && (room->getCardPlace(card_id) == Player::PlaceHand
                            || room->getCardPlace(card_id) == Player::PlaceEquip))){
                        if (can_kangdao(player, card_id))
                            card_ids << card_id;
                        if (cd->hasFlag("kangdaomove"))
                            cd->setFlags("-kangdaomove");
                    }
                }
            }
            if (card_ids.isEmpty())
                return false;
            else if (player->askForSkillInvoke(objectName(), data)){
                room->broadcastSkillInvoke(objectName(), 1);
                int ai_delay = Config.AIDelay;
                Config.AIDelay = 0;
                QList<int> ids;
                while (!card_ids.isEmpty()){
                    room->fillAG(card_ids, player);
                    int id = room->askForAG(player, card_ids, true, objectName() + "Gain");
                    if (id == -1){
                        room->clearAG(player);
                        break;
                    }
                    card_ids.removeOne(id);
                    foreach(int cardid, card_ids){
                        if (equip_type(id) == equip_type(cardid))
                            card_ids.removeOne(cardid);
                    }
                    ids << id;
                    room->clearAG(player);
                }
                Config.AIDelay = ai_delay;

                if (!ids.isEmpty()){
                    foreach(int id, ids){
                        if (move.card_ids.contains(id)){
                            move.from_places.removeAt(move.card_ids.indexOf(id));
                            move.card_ids.removeOne(id);
                            data = QVariant::fromValue(move);
                        }
                        room->moveCardTo(Sanguosha->getCard(id), player, Player::PlaceEquip, move.reason, true);
                        if (!player->isAlive())
                            break;
                    }
                    ServerPlayer *from = (ServerPlayer *)move.from;
                    if (from->isAlive()){
                        QList<const Card *> cards = player->getEquips();
                        foreach(const Card *card, from->getEquips()){
                            foreach(const Card *cd, cards){
                                if (equip_type(card->getId()) == equip_type(cd->getId()))
                                    cards.removeOne(cd);
                            }
                        }
                        if (cards.isEmpty())
                            return false;
                        QList<int> cardids;
                        foreach(const Card *cd, cards)
                            cardids.append(cd->getId());
                        if (room->askForChoice(from, objectName(), "kangdaogain+kangdaocancel") == "kangdaogain"){
                            room->fillAG(cardids);
                            int cid = room->askForAG(from, cardids, true, objectName() + "Chosen");
                            room->clearAG();
                            if (cid != -1){
                                room->broadcastSkillInvoke(objectName(), 2);
                                CardMoveReason reason(CardMoveReason::S_REASON_PUT, player->objectName());
                                room->moveCardTo(Sanguosha->getCard(cid), player, from, Player::PlaceEquip, reason, true);
                                room->drawCards(player, 1);
                            }
                        }
                    }
                }
            }
        }
        if (move.from_places.contains(Player::PlaceTable)){
            foreach(int id, move.card_ids){
                const Card *card = Sanguosha->getCard(id);
                if (card->hasFlag("kangdaomove"))
                    card->setFlags("-kangdaomove");
            }
        }
        return false;
    }

private:

    bool can_kangdao(ServerPlayer *player, int id) const{
        const Card *card = Sanguosha->getCard(id);
        if (card->isKindOf("Weapon"))
            return !player->getWeapon();
        else if (card->isKindOf("Armor"))
            return !player->getArmor();
        else if (card->isKindOf("DefensiveHorse"))
            return !player->getDefensiveHorse();
        else if (card->isKindOf("OffensiveHorse"))
            return !player->getOffensiveHorse();

        return false;
    }

    QString equip_type(int id) const{
        const Card *card = Sanguosha->getCard(id);
        if (card->isKindOf("Weapon"))
            return "Weapon";
        else if (card->isKindOf("Armor"))
            return "Armor";
        else if (card->isKindOf("DefensiveHorse"))
            return "DefensiveHorse";
        else if (card->isKindOf("OffensiveHorse"))
            return "OffensiveHorse";

        return "Unknown";
    }

};

class Xiangshu: public TriggerSkill{
public:
    Xiangshu(): TriggerSkill("xiangshu"){
        events << CardUsed << CardFinished << CardsMoveOneTime;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent != CardsMoveOneTime){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("AmazingGrace")){
                ServerPlayer *p = room->findPlayerBySkillName(objectName());
                p->setFlags((triggerEvent == CardUsed) ? "agusing": "-agusing");
            }
        }
        if (TriggerSkill::triggerable(player) && !player->hasFlag("agusing")){
            if (triggerEvent == CardsMoveOneTime){
                CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
                if (!(move.from_places.contains(Player::DrawPile) || move.to_place == Player::DrawPile))
                    return false;
            }

            QList<int> drawpile = room->getDrawPile();
            QList<int> watchlist;
            for (int i = 0; i < qMin(drawpile.length(), 4); i++)
                watchlist << drawpile[i];

            Json::Value gongxinArgs(Json::arrayValue);

            gongxinArgs[0] = QSanProtocol::Utils::toJsonString(QString());
            gongxinArgs[1] = false;
            gongxinArgs[2] = QSanProtocol::Utils::toJsonArray(watchlist);

            room->doNotify(player, QSanProtocol::S_COMMAND_SHOW_ALL_CARDS, gongxinArgs);


            LogMessage l;
            l.type = "$xiangshudrawpile";
            l.card_str = IntList2StringList(watchlist).join("+");

            room->doNotify(player, QSanProtocol::S_COMMAND_LOG_SKILL, l.toJsonValue());

        }
        return false;
    }

};

class Bushi: public TriggerSkill{
public:
    Bushi(): TriggerSkill("bushi"){
        events << EventPhaseStart << CardsMoveOneTime << EventPhaseChanging;
    }

private:
    const static int bushi_notinvoke = 0;
    const static int bushi_red = 1;
    const static int bushi_black = 2;

public:
    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        switch (triggerEvent){
            case (EventPhaseStart):{
                if (player->getPhase() != Player::Play)
                    return false;

                room->setPlayerMark(player, "bushi_color", bushi_notinvoke);

                ServerPlayer *p = room->findPlayerBySkillName("bushi");
                if (p == NULL || p->getHandcardNum() > 4)
                    return false;

                const Card *c;

                if (c = room->askForCard(p, "..", "@bushi-discard", QVariant::fromValue(player), Card::MethodNone)){
                    room->broadcastSkillInvoke(objectName(), 1);
                    room->showCard(p, c->getId());
                    room->setPlayerMark(player, "bushi_color", c->isRed() ? bushi_red : bushi_black);
                    room->setTag("bushi_invoker", QVariant::fromValue(p));
                }
                else
                    room->setPlayerMark(player, "bushi_color", bushi_notinvoke);

                break;
            }
            case (CardsMoveOneTime):{
                CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
                if (move.from != player || move.to == player || player->getMark("bushi_color") == bushi_notinvoke)
                    return false;

                if (player->getPhase() != Player::Play || player->hasFlag("bushi_candraw"))
                    return false;

                foreach(int id, move.card_ids){
                    const Card *c = Sanguosha->getCard(id);
                    bool iscolor = (c->isRed() && player->getMark("bushi_color") == bushi_red)
                                || (c->isBlack() && player->getMark("bushi_color") == bushi_black);

                    if (!iscolor)
                        continue;

                    int index = move.card_ids.indexOf(id);
                    if (move.from_places[index] == Player::PlaceHand || move.from_places[index] == Player::PlaceEquip){
                        //ToAsk: 如果牌移动过程是不可见的，怎么确定已经失去了这个颜色的牌？
                        //这个也忘了……
                        room->setPlayerFlag(player, "bushi_candraw");
                        return false;
                    }
                }
                break;
            }
            case (EventPhaseChanging):{
                PhaseChangeStruct change = data.value<PhaseChangeStruct>();
                if (change.from == Player::Play && player->getMark("bushi_color") != bushi_notinvoke){
                    ServerPlayer *p = room->getTag("bushi_invoker").value<ServerPlayer *>();
                    room->removeTag("bushi_invoker");

                    if (p == NULL || !p->isAlive())
                        return false;

                    if (!player->hasFlag("bushi_candraw")){
                        room->broadcastSkillInvoke(objectName(), 2);
                        return false;
                    }
                    room->setPlayerMark(player, "bushi_color", bushi_notinvoke);
                    room->setPlayerFlag(player, "-bushi_candraw");


                    room->drawCards(p, 1, "bushi");

                    QString choice = room->askForChoice(p, "bushi", "bushiinc+bushidec", QVariant::fromValue(player));

                    room->setPlayerFlag(player, choice);
                    LogMessage l;
                    l.from = p;
                    l.to << player;
                    l.arg = objectName();
                    if (choice == "bushiinc"){
                        room->broadcastSkillInvoke(objectName(), 3);
                        l.type = "#bushi_inc";
                    }
                    else{
                        room->broadcastSkillInvoke(objectName(), 4);
                        l.type = "#bushi_dec";
                    }
                    room->sendLog(l);
                }
                break;
            }
            default:
                break;
        }
        return false;
    }
};
class BushiMaxCards: public MaxCardsSkill{
public:
    BushiMaxCards(): MaxCardsSkill("#bushi"){
    }

    virtual int getExtra(const Player *target) const{
        if (target->hasFlag("bushiinc"))
            return 1;
        else if (target->hasFlag("bushidec"))
            return -1;
        return 0;
    }
};

class Juanxia:public DistanceSkill{
public:
    Juanxia():DistanceSkill("juanxia"){
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if (from->hasSkill(objectName()))
            if (from->getHp() <= to->getHp())
                return -1;
        if (to->hasSkill(objectName()))
            if (to->getHp() <= from->getHp())
                return 1;
        return 0;
    }
};

ChouduCard::ChouduCard(){
    will_throw = false;
}

bool ChouduCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (targets.length() < Self->getMark("chouduuse"))
        if (to_select != Self)
            return !to_select->isKongcheng();
    return false;
}

void ChouduCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    room->setPlayerMark(source, "choudutargets", targets.length());
    QList<CardsMoveStruct> moves;
    QStringList players;
    foreach(ServerPlayer *p, targets){
        players << p->objectName();
        CardsMoveStruct move;
        int id = room->askForCardChosen(source, p, "h", objectName());
        move.card_ids << id;
        move.to = source;
        move.to_place = Player::PlaceHand;
        moves << move;
        room->setPlayerFlag(p, "choudutarget");
    }
    room->setTag("choudutargets", players.join("+"));
    room->setPlayerMark(source, "chouduuse", 0);
    room->moveCards(moves, false);
}

class ChouduVS: public ZeroCardViewAsSkill{
public:
    ChouduVS(): ZeroCardViewAsSkill("choudu"){
        response_pattern = "@@choudu";
    }

    virtual const Card *viewAs() const{
        return new ChouduCard;
    }
};

class Choudu: public TriggerSkill{
public:
    Choudu(): TriggerSkill("choudu"){
        view_as_skill = new ChouduVS;
        events << CardUsed << CardResponded << EventPhaseStart << CardsMoveOneTime;
    }

private:
    ServerPlayer *findPlayerByObjectName(Room *room, QString name, bool include_death = false, ServerPlayer *except = NULL) const{
        QList<ServerPlayer *> players;
        if (include_death)
            players = room->getPlayers();
        else
            players = room->getAllPlayers();
        if (except != NULL)
            players.removeOne(except);
        foreach(ServerPlayer *p, players){
            if (p->objectName() == name)
                return p;
        }
        return NULL;
    }

public:
    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == CardUsed || triggerEvent == CardResponded){
            if (player->getPhase() == Player::NotActive)
                return false;
            const Card *c;
            if (triggerEvent == CardUsed)
                c = data.value<CardUseStruct>().card;
            else {
                CardResponseStruct resp = data.value<CardResponseStruct>();
                if (resp.m_isUse)
                    c = resp.m_card;
                else
                    return false;
            }
            if (!c->isKindOf("SkillCard"))
                room->addPlayerMark(player, "chouduuse");
        }
        else if (triggerEvent == EventPhaseStart){
            if (player->getPhase() == Player::Discard){
                int count = player->getMark("chouduuse");
                if (count > 0){
                    bool can_invoke = false;
                    foreach(ServerPlayer *target, room->getOtherPlayers(player))
                        if (!target->isKongcheng()){
                            can_invoke = true;
                            break;
                        }
                    if (!can_invoke)
                        return false;
                    if (room->askForUseCard(player, "@@choudu", "@choudu")){
                        QList<int> ids;
                        foreach(const Card *card, player->getCards("he"))
                            ids << card->getId();
                        room->setPlayerFlag(player, "choudumove");
                        QStringList players = room->getTag("choudutargets").toString().split("+");
                        QList<ServerPlayer *> targets;
                        foreach(QString name, players)
                            targets << findPlayerByObjectName(room, name);
                        while (room->askForYiji(player, ids, objectName(), false, false, true ,-1, targets))
                            ;
                        room->setPlayerFlag(player, "-choudumove");
                        if (player->getMark("choudutargets") > 0){
                            room->broadcastSkillInvoke(objectName(), 4);
                            room->loseHp(player, player->getMark("choudutargets"));
                        }
                    }
                }
            }
        }
        else {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from && move.from == player && move.from->hasFlag("choudumove")){
                if (!move.to->hasFlag("chouduselected"))
                    room->removePlayerMark(player, "choudutargets");
                room->setPlayerFlag(room->findPlayer(move.to->getGeneralName()), "chouduselected");

                room->broadcastSkillInvoke(objectName(), qrand() % 2 + 2);
            }
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 1;
    }
};


class Xuedian: public TargetModSkill{
public:
    Xuedian(): TargetModSkill("xuedian"){
        pattern = "Slash";
        frequency = NotFrequent;
    }

    virtual int getExtraTargetNum(const Player *from, const Card *) const{
        if (from->hasSkill(objectName()))
            return from->getLostHp();

        return 0;
    }
};
class Xuediantr: public TriggerSkill{
public:
    Xuediantr(): TriggerSkill("#xuediantr"){
        events << Damage << EventPhaseChanging;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == Damage){
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card && damage.card->isKindOf("Slash"))
                room->setPlayerFlag(player, "xuedian_damage");
        }
        else {
            int x = player->getLostHp();
            if (x == 0)
                return false;
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive && player->hasFlag("xuedian_damage")){
                const Card *card = room->askForCard(player, ".|red", "@xuedian:" + QString::number(x), data, Card::MethodNone);
                if (card != NULL){
                    player->drawCards(x);
                    room->moveCardTo(card, NULL, Player::DrawPile, true);
                }
            }
        }
        return false;
    }
};

class Duanhun: public ViewAsSkill{
public:
    Duanhun(): ViewAsSkill("duanhun"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getCardCount(true) >= 2 && Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return player->getCardCount(true) >= 2 && pattern == "slash";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (selected.length() >= 2)
            return false;

        if (Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY
                && Self->getWeapon() && to_select->getEffectiveId() == Self->getWeapon()->getId()
                && to_select->isKindOf("Crossbow"))
            return Self->canSlashWithoutCrossbow();

        return true;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 2)
            return NULL;

        Slash *slash = new Slash(Card::SuitToBeDecided, 0);
        slash->setSkillName(objectName());
        slash->addSubcards(cards);

        return slash;
    }
};

class Zhanji: public TriggerSkill{
public:
    Zhanji(): TriggerSkill("zhanji"){
        frequency = Wake;
        events << PreDamageDone << EventPhaseChanging << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        switch (triggerEvent){
            case (PreDamageDone):{
                DamageStruct damage = data.value<DamageStruct>();
                if (damage.from != NULL && damage.from->isAlive()
                        && damage.from == room->getCurrent()
                        && damage.from->getMark("zhanji") == 0)
                    room->addPlayerMark(damage.from, "zhanji_damage", damage.damage);
                break;
            }
            case (EventPhaseChanging):{
                PhaseChangeStruct change = data.value<PhaseChangeStruct>();
                if (change.to == Player::NotActive)
                    if (player->getMark("zhanji_damage") > 0)
                        room->setPlayerMark(player, "zhanji_damage", 0);
                break;
            }
            case (EventPhaseStart):{
                if (TriggerSkill::triggerable(player)
                        && player->getPhase() == Player::Finish
                        && player->getMark("zhanji") == 0
                        && player->getMark("zhanji_damage") >= 3){

                    room->notifySkillInvoked(player, objectName());
                    LogMessage l;
                    l.type = "#ZhanjiWake";
                    l.from = player;
                    l.arg = QString::number(player->getMark("zhanji_damage"));
                    l.arg2 = objectName();
                    room->sendLog(l);
                    room->broadcastSkillInvoke(objectName());
                    room->doLightbox("$ZhanjiAnimate", 4000);

                    if (room->changeMaxHpForAwakenSkill(player, 1)){
                        room->addPlayerMark(player, "zhanji");
                        RecoverStruct recover;
                        recover.who = player;
                        room->recover(player, recover);

                        room->handleAcquireDetachSkills(player, "-duanhun");
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

class Tuwei: public TargetModSkill{
public:
    Tuwei(): TargetModSkill("tuwei"){
    }

    virtual int getExtraTargetNum(const Player *from, const Card *card) const{
        if (!from->hasSkill(objectName()))
            return 0;

        int i = 0;
        foreach(const Player *p, from->getAliveSiblings()) {
            if(p->inMyAttackRange(from))
                i++;
        }

        return i > from->getHp() ? 1 : 0;
    }
};

GudanCard::GudanCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool GudanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
    } else if (Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return false;
    }

    CardStar card = Self->tag.value("gudan").value<CardStar>();
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
}

bool GudanCard::targetFixed() const{
    if (Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFixed();
    } else if (Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return true;
    }

    CardStar card = Self->tag.value("gudan").value<CardStar>();
    return card && card->targetFixed();
}

bool GudanCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if (Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetsFeasible(targets, Self);
    } else if (Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return true;
    }

    CardStar card = Self->tag.value("gudan").value<CardStar>();
    return card && card->targetsFeasible(targets, Self);
}

const Card *GudanCard::validate(CardUseStruct &card_use) const{
    ServerPlayer *wenyang = card_use.from;
    Room *room = wenyang->getRoom();

    QString to_gudan = user_string;
    if (user_string == "slash"
        && Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
            QStringList gudan_list;
            gudan_list << "slash";
            if (!Config.BanPackages.contains("maneuvering"))
                gudan_list << "thunder_slash" << "fire_slash";
            to_gudan = room->askForChoice(wenyang, "gudan_slash", gudan_list.join("+"));
    }

    Card *use_card = Sanguosha->cloneCard(to_gudan, SuitToBeDecided, -1);
    use_card->setSkillName("gudan");
    use_card->addSubcards(subcards);
    use_card->deleteLater();
    return use_card;
}

const Card *GudanCard::validateInResponse(ServerPlayer *wenyang) const{
    Room *room = wenyang->getRoom();

    QString to_gudan;
    if (user_string == "peach+analeptic") {
        QStringList gudan_list;
        gudan_list << "peach";
        if (!Config.BanPackages.contains("maneuvering"))
            gudan_list << "analeptic";
        to_gudan = room->askForChoice(wenyang, "gudan_saveself", gudan_list.join("+"));
    } else if (user_string == "slash") {
        QStringList gudan_list;
        gudan_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            gudan_list << "thunder_slash" << "fire_slash";
        to_gudan = room->askForChoice(wenyang, "gudan_slash", gudan_list.join("+"));
    }
    else
        to_gudan = user_string;

    Card *use_card = Sanguosha->cloneCard(to_gudan, SuitToBeDecided, -1);
    use_card->setSkillName("gudan");
    use_card->addSubcards(subcards);
    use_card->deleteLater();
    return use_card;
}

class Gudan: public ViewAsSkill {
public:
    Gudan(): ViewAsSkill("gudan") {
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if (player->isKongcheng() || pattern.startsWith(".") || pattern.startsWith("@")) return false;
        if (pattern == "peach" && player->hasFlag("Global_PreventPeach")) return false;
        for (int i = 0; i < pattern.length(); i++) {
            QChar ch = pattern[i];
            if (ch.isUpper() || ch.isDigit()) return false; // This is an extremely dirty hack!! For we need to prevent patterns like 'BasicCard'
        }
        return !(pattern == "nullification"); //bugfix: gudan is enabled when pattern == nullification and there are nullifications in hand
    }

    virtual QDialog *getDialog() const{
        return GuhuoDialog::getInstance("gudan", true, false);
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() < Self->getHandcardNum())
            return NULL;

        if (Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE
            || Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
                GudanCard *card = new GudanCard;
                card->setUserString(Sanguosha->getCurrentCardUsePattern());
                card->addSubcards(cards);
                return card;
        }

        CardStar c = Self->tag.value("gudan").value<CardStar>();
        if (c) {
            GudanCard *card = new GudanCard;
            card->setUserString(c->objectName());
            card->addSubcards(cards);
            return card;
        } else
            return NULL;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng();
    }
};

ShangjianCard::ShangjianCard(): SkillCard(){
    will_throw = false;
    handling_method = Card::MethodNone;
}

void ShangjianCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);
}

class ShangjianVS: public ViewAsSkill{
public:
    ShangjianVS(): ViewAsSkill("shangjian"){
        response_pattern = "@@shangjian";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return NULL;

        ShangjianCard *card = new ShangjianCard;
        card->addSubcards(cards);
        return card;
    }
};

class Shangjian: public PhaseChangeSkill{
public:
    Shangjian(): PhaseChangeSkill("shangjian"){
        view_as_skill = new ShangjianVS;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() == Player::Discard)
            target->getRoom()->askForUseCard(target, "@@shangjian", "@shangjian-give", -1, Card::MethodNone, false);
        return false;
    }
};

class Manwu: public TriggerSkill{
public:
    Manwu(): TriggerSkill("manwu"){
        events << EventPhaseChanging << FinishJudge;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseChanging){
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::Judge && !player->isSkipped(Player::Judge) && !player->isSkipped(Player::Play)
                    && player->askForSkillInvoke(objectName())){
                player->skip(Player::Judge);
                player->skip(Player::Play);

                JudgeStruct judge;
                judge.reason = objectName();
                judge.good = true;
                judge.pattern = ".";
                //judge.play_animation = false;
                room->judge(judge);

                bool red = (judge.pattern == "red");
                QList<ServerPlayer *> invoke_players;
                foreach (ServerPlayer *p, room->getAlivePlayers()){
                    QList<const Card *> field_cards = p->getCards("ej");
                    bool can_invoke = false;
                    foreach (const Card *c, field_cards)
                        if (c->isRed() == red){
                            can_invoke = true;
                            break;
                        }
                    if (can_invoke)
                        invoke_players << p;
                }

                if (invoke_players.isEmpty())
                    return false;

                ServerPlayer *victim = room->askForPlayerChosen(player, invoke_players, objectName(), "@manwu", true);

                if (victim == NULL)
                    return false;

                QList<int> ids;
                QList<const Card *> cards = victim->getCards("ej");
                foreach(const Card *c, cards)
                    if (c->isRed() == red)
                        ids << c->getId();

                if (ids.isEmpty())
                    return false;

                int aidelay = Config.AIDelay;
                Config.AIDelay = 0;

                room->fillAG(ids, player);
                int id = room->askForAG(player, ids, true, objectName());
                room->clearAG(player);

                Config.AIDelay = aidelay;

                if (id == -1)
                    return false;

                player->addToPile("ci", id, true);

            }
        }
        else if (triggerEvent == FinishJudge){
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (judge->reason == objectName())
                judge->pattern = judge->card->isRed() ? "red" : "black";
        }
        return false;
    }
};

class Annei: public MasochismSkill{
public:
    Annei(): MasochismSkill("annei"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{
        if (damage.from == NULL)
            return;

        Room *room = target->getRoom();

        QList<ServerPlayer *> bians = room->findPlayersBySkillName(objectName());

        foreach(ServerPlayer *bian, bians){
            if (bian->getPile("ci").isEmpty())
                continue;

            if (bian->askForSkillInvoke(objectName(), QVariant::fromValue(damage))){
                int aidelay = Config.AIDelay;
                Config.AIDelay = 0;

                QList<int> ci = bian->getPile("ci");
                room->fillAG(ci, bian);
                int id = room->askForAG(bian, ci, true, objectName());
                room->clearAG(bian);

                Config.AIDelay = aidelay;

                if (id == -1)
                    continue;

                damage.from->obtainCard(Sanguosha->getCard(id), false);

                RecoverStruct recover;
                recover.who = bian;
                room->recover(target, recover);
            }
        }
    }
};

//for tigerfly liyan. Liyan's Dangliang is complicated
class Jingao: public ProhibitSkill{
public:
    Jingao(): ProhibitSkill("jingao"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others = QList<const Player *>()) const{
        return to->hasSkill(objectName()) && from->getEquips().length() < to->getEquips().length() && card->isKindOf("BasicCard");
    }
};

JingshangCard::JingshangCard(): SkillCard(){
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool JingshangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() < 2 && to_select != Self && !to_select->isKongcheng();
}

bool JingshangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void JingshangCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    const Card *showcard;

    if (subcards.length() == 0){
        int aidelay = Config.AIDelay;
        Config.AIDelay = 0;

        room->fillAG(source->getPile("zi"), source);
        int showcard_id = room->askForAG(source, source->getPile("zi"), false, objectName());
        room->clearAG(source);

        Config.AIDelay = aidelay;

        showcard = Sanguosha->getCard(showcard_id);
    }
    else
        showcard = Sanguosha->getCard(subcards.first());

    room->showCard(source, showcard->getId());

    source->tag["jingshang_card"] = QVariant::fromValue(showcard);
    source->tag["jingshang_using"] = true;

    targets.first()->pindian(targets.last(), objectName());

    source->tag["jingshang_using"] = false;
    source->tag.remove("jingshang_card");
}

class JingshangVS: public ViewAsSkill{
public:
    JingshangVS(): ViewAsSkill("jingshang"){

    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.isEmpty();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        JingshangCard *card = new JingshangCard;
        card->addSubcards(cards);
        return card;
    }
};

class Jingshang: public TriggerSkill{
public:
    Jingshang(): TriggerSkill("jingshang"){
        events << Pindian;
        view_as_skill = new JingshangVS;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        PindianStruct *pindian = data.value<PindianStruct *>();
        if (pindian->reason != objectName())
            return false;

        ServerPlayer *mizhu = NULL;
        const Card *showcard = NULL;
        foreach (ServerPlayer *p, room->getAlivePlayers())
            if (p->tag["jingshang_using"].toBool() == true){
                mizhu = p;
                showcard = p->tag["jingshang_card"].value<const Card *>();
            }

        if (mizhu != NULL){
            ServerPlayer *winner = NULL;
            if (pindian->success)
                winner = pindian->from;
            else if (pindian->from_number < pindian->to_number)
                winner = pindian->to;

            if (winner != NULL){
                /*CardMoveReason reason;
                if (room->getCardPlace(showcard->getId()) == Player::PlaceSpecial)
                    reason = CardMoveReason(CardMoveReason::S_REASON_EXCHANGE_FROM_PILE, mizhu->objectName());*/
                winner->obtainCard(showcard);
            }

            QList<int> to_addtopile;
            to_addtopile << pindian->from_card->getEffectiveId() << pindian->to_card->getEffectiveId();

            mizhu->addToPile("zi", to_addtopile, true);
        }

        return false;
    }
};

class Zijun: public PhaseChangeSkill{
public:
    Zijun(): PhaseChangeSkill("zijun"){

    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive() && !target->hasSkill(objectName()) && target->getHandcardNum() < target->getHp();
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        QList<ServerPlayer *> mizhus = room->findPlayersBySkillName(objectName());

        foreach(ServerPlayer *mizhu, mizhus){
            QList<int> zi = mizhu->getPile("zi");
            if (zi.length() > 0){
                QList<int> to_give;
                while (room->askForChoice(mizhu, objectName(), "dismiss+give" + QString(to_give.isEmpty()? "": "continue"), QVariant::fromValue(target)).startsWith("give")){
                    room->fillAG(mizhu->getPile("zi"), mizhu, to_give);
                    int card_id = room->askForAG(mizhu, zi, false, objectName());
                    room->clearAG(mizhu);
                    zi.removeOne(card_id);
                    to_give << card_id;
                    if (zi.isEmpty())
                        break;
                }

                CardsMoveStruct move(to_give, target, Player::PlaceHand, 
                    CardMoveReason(CardMoveReason::S_REASON_GOTCARD, mizhu->objectName(), objectName(), QString()));

                room->moveCards(move, true);

                if (to_give.length() >= 2){
                    RecoverStruct recover;
                    recover.who = mizhu;
                    room->recover(mizhu, recover);
                }
            }
        }

        return false;
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
    neo_zhugeke->addSkill(new Skill("qiangbian", Skill::Compulsory));
    neo_zhugeke->addSkill(new NeoAocai);
    neo_zhugeke->addRelateSkill("zhuanquan");

    General *tadun = new General(this, "tadun", "qun", 4);
    tadun->addSkill(new Xiongjie);
    tadun->addSkill(new XiongjieAcDe);
    tadun->addSkill(new XiongjieCount);
    related_skills.insertMulti("xiongjie", "#xiongjie-count");
    related_skills.insertMulti("xiongjie", "#xiongjie-acquire-detach");

    General *liuzhang = new General(this, "liuzhang", "qun", 4);
    liuzhang->addSkill(new Tushou);
    liuzhang->addSkill("zongshi");

    General *zhoucang = new General(this, "zhoucang", "shu", 4);
    zhoucang->addSkill(new Kangdao);

    General *guanlu = new General(this, "guanlu", "qun", 3);
    guanlu->addSkill(new Xiangshu);
    guanlu->addSkill(new Bushi);
    guanlu->addSkill(new BushiMaxCards);
    related_skills.insertMulti("bushi", "#bushi");

    General *yangyi = new General(this, "yangyi", "shu", 3);
    yangyi->addSkill(new Juanxia);
    yangyi->addSkill(new Choudu);

    General *zhangxingcai = new General(this, "zhangxingcai", "shu", 3, false);
    zhangxingcai->addSkill(new Duanhun);
    zhangxingcai->addSkill(new Xuedian);
    zhangxingcai->addSkill(new Xuediantr);
    zhangxingcai->addSkill(new Zhanji);
    related_skills.insertMulti("xuedian", "#xuediantr");

    General *wenyang = new General(this, "wenyang", "wei");
    wenyang->addSkill(new Tuwei);
    wenyang->addSkill(new Gudan);

    General *bian = new General(this, "bianfuren", "wei", 3, false);
    bian->addSkill(new Shangjian);
    bian->addSkill(new Manwu);
    bian->addSkill(new Annei);

    General *mizhu = new General(this, "mizhu", "shu", 3);
    mizhu->addSkill(new Jingshang);
    mizhu->addSkill(new Zijun);

    addMetaObject<PozhenCard>();
    addMetaObject<TushouGiveCard>();
    addMetaObject<ChouduCard>();
    addMetaObject<GudanCard>();
    addMetaObject<JisiCard>();
    addMetaObject<ShangjianCard>();
    addMetaObject<JingshangCard>();

    skills << new Zhuanquan;
};

ADD_PACKAGE(TigerFly)
