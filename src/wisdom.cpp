#include "wisdom.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"
#include "room.h"
#include "maneuvering.h"

JuaoCard::JuaoCard(){
    once = true;
}

bool JuaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return true;
}

void JuaoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    foreach(int cardid, this->getSubcards()){
        targets.first()->addToPile("juaocd", cardid, false);
    }
    targets.first()->addMark("juao");
}

class JuaoViewAsSkill: public ViewAsSkill{
public:
    JuaoViewAsSkill():ViewAsSkill("juao"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("JuaoCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() > 2)
            return false;
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        JuaoCard *card = new JuaoCard();
        card->addSubcards(cards);
        return card;
    }
};

class Juao: public PhaseChangeSkill{
public:
    Juao():PhaseChangeSkill("juao"){
        view_as_skill = new JuaoViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("juao") > 0;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Start){
            Room *room = player->getRoom();
            player->loseAllMarks("juao");
            foreach(int cdid, player->getPile("juaocd"))
                player->obtainCard(Sanguosha->getCard(cdid));

            LogMessage log;
            log.type = "#Juao_get";
            log.from = player;
            log.to << room->findPlayerBySkillName(objectName());
            room->sendLog(log);

            player->skip(Player::Draw);
        }
        return false;
    }
};

class Tanlan: public TriggerSkill{
public:
    Tanlan():TriggerSkill("tanlan"){
        events << Pindian << Damaged;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *xuyou, QVariant &data) const{
        if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            ServerPlayer *from = damage.from;
            Room *room = xuyou->getRoom();
            if(from && !from->isKongcheng() && !xuyou->isKongcheng() && room->askForSkillInvoke(xuyou, objectName())){
                xuyou->pindian(from, "tanlan");
            }
            return false;
        }
        else {
            PindianStar pindian = data.value<PindianStar>();
            if(pindian->reason == "tanlan"){
                ServerPlayer *winner = pindian->from_card->getNumber() > pindian->to_card->getNumber() ? pindian->from : pindian->to;
                if(winner == xuyou){
                    winner->obtainCard(pindian->to_card);
                    winner->obtainCard(pindian->from_card);
                }
            }
        }

        return false;
    }
};

class Shicai: public TriggerSkill{
public:
    Shicai():TriggerSkill("shicai"){
        events << Pindian;
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *xuyou = room->findPlayerBySkillName(objectName());
        PindianStar pindian = data.value<PindianStar>();
        if(pindian->from != xuyou && pindian->to != xuyou)
            return false;
        ServerPlayer *winner = pindian->from_card->getNumber() > pindian->to_card->getNumber() ? pindian->from : pindian->to;
        if(winner == xuyou){
            LogMessage log;
            log.type = "#Shicai_draw";
            log.from = xuyou;
            room->sendLog(log);

            xuyou->drawCards(1);
        }
        return false;
    }
};

class Yicai:public TriggerSkill{
public:
    Yicai():TriggerSkill("yicai"){
        events << CardUsed << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *jiangwei, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->inherits("TrickCard") && !card->inherits("DelayedTrick")){
            Room *room = jiangwei->getRoom();
            if(room->askForSkillInvoke(jiangwei, objectName()))
                room->askForUseCard(jiangwei, "slash", "@yicai");
        }
        return false;
    }
};

class Beifa: public TriggerSkill{
public:
    Beifa():TriggerSkill("beifa"){
        events << CardLost;
        frequency = Compulsory;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *jiangwei, QVariant &data) const{
        if(jiangwei->isKongcheng()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place != Player::Hand)
                return false;

            Room *room = jiangwei->getRoom();
            QList<ServerPlayer *> players = room->getOtherPlayers(jiangwei);
            foreach(ServerPlayer *player, players){
                if(!jiangwei->inMyAttackRange(player))
                    players.removeOne(player);
                if(player->hasSkill("kongcheng") && player->isKongcheng())
                    players.removeOne(player);
            }
            ServerPlayer *target = jiangwei;
            if(players.length() > 0)
                target = room->askForPlayerChosen(jiangwei, players, objectName());
            if(!target) target = jiangwei;

            LogMessage log;
            log.type = "#beifa_effect";
            log.from = jiangwei;
            room->sendLog(log);

            CardEffectStruct effect;
            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName(objectName());
            effect.card = slash;
            effect.from = jiangwei;
            effect.to = target;

            room->cardEffect(effect);
        }
        return false;
    }
};

HouyuanCard::HouyuanCard(){
    once = true;
}

void HouyuanCard::onEffect(const CardEffectStruct &effect) const{
    int n = subcardsLength();
    effect.to->drawCards(n);
}

class Houyuan: public ViewAsSkill{
public:
    Houyuan():ViewAsSkill("houyuan"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 2;
    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("HouyuanCard");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        HouyuanCard *card = new HouyuanCard;
        card->addSubcards(cards);
        return card;
    }
};

class Chouliang: public PhaseChangeSkill{
public:
    Chouliang():PhaseChangeSkill("chouliang"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Finish && player->getHandcardNum()<3
           && room->askForSkillInvoke(player, objectName())){
            for(int i=0; i<4; i++){
                int card_id = room->drawCard();
                room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, true);
                room->getThread()->delay();

                const Card *card = Sanguosha->getCard(card_id);
                if(!card->inherits("BasicCard"))
                    room->throwCard(card_id);
                else
                    room->obtainCard(player, card_id);
            }
        }
        return false;
    }
};

BawangCard::BawangCard(){
}

bool BawangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;
    if(to_select == Self)
        return false;
    if(to_select->isKongcheng() && to_select->hasSkill("kongcheng"))
        return false;
    return true;
}

void BawangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    CardEffectStruct effect2;
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("bawang");
    effect2.card = slash;
    effect2.from = effect.from;
    effect2.to = effect.to;

    room->cardEffect(effect2);

    room->setEmotion(effect.to, "bad");
    room->setEmotion(effect.from, "good");
}

class BawangViewAsSkill: public ZeroCardViewAsSkill{
public:
    BawangViewAsSkill():ZeroCardViewAsSkill("tuxi"){
    }

    virtual const Card *viewAs() const{
        return new BawangCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@bawang";
    }
};

class Bawang: public TriggerSkill{
public:
    Bawang():TriggerSkill("bawang"){
        view_as_skill = new BawangViewAsSkill;
        events << SlashMissed;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *sunce, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(!effect.to->isNude() && !sunce->isKongcheng() && !effect.to->isKongcheng()){
            Room *room = sunce->getRoom();
            if(room->askForSkillInvoke(sunce, objectName())){
                bool success = sunce->pindian(effect.to, objectName(), NULL);
                if(success){
                    room->askForUseCard(sunce, "@@bawang", "@bawangask");
                }
            }
        }
        return false;
    }
};

WeidaiCard::WeidaiCard(){
        target_fixed = true;
}

void WeidaiCard::use(Room *room, ServerPlayer *sunce, const QList<ServerPlayer *> &targets) const{
    if(sunce->hasFlag("drank"))
        return;
    QList<ServerPlayer *> lieges = room->getLieges("wu", sunce);
    const Card *analeptic = NULL;
    foreach(ServerPlayer *liege, lieges){
        analeptic = room->askForCard(liege, ".S29", "@weidai-analeptic");
        if(analeptic){
            LogMessage log;
            log.type = "$Weidai";
            log.from = liege;
            log.card_str = analeptic->getEffectIdString();
            room->sendLog(log);

            CardEffectStruct effect;
            Analeptic *ana = new Analeptic(analeptic->getSuit(), analeptic->getNumber());
            ana->setSkillName(objectName());
            effect.card = ana;
            effect.from = sunce;
            effect.to = sunce;

            room->cardEffect(effect);
            return;
        }
    }
}

class SpatwoninePattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return ! player->hasEquip(card) && card->getSuit() == Card::Spade
                && card->getNumber() > 1 && card->getNumber() < 10;
    }
};

class WeidaiViewAsSkill:public ZeroCardViewAsSkill{
public:
    WeidaiViewAsSkill():ZeroCardViewAsSkill("analeptic$"){
    }

    virtual bool isEnabledAtPlay() const{
        return Self->hasLordSkill("weidai") && !Self->hasFlag("drank");
    }

    virtual const Card *viewAs() const{
        return new WeidaiCard;
    }
};

class Weidai: public TriggerSkill{
public:
    Weidai():TriggerSkill("weidai$"){
        events <<  AskForPeaches;
        default_choice = "ignore";

        view_as_skill = new WeidaiViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("weidai");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *sunce, QVariant &data) const{

        Room *room = sunce->getRoom();
        QList<ServerPlayer *> lieges = room->getLieges("wu", sunce);
        if(lieges.isEmpty())
            return false;

        if(!room->askForSkillInvoke(sunce, objectName()))
            return false;

        foreach(ServerPlayer *liege, lieges){
            const Card *analeptic = room->askForCard(liege, ".S29", "@weidai-analeptic");
            if(analeptic){
                LogMessage log;
                log.type = "$Weidai";
                log.from = liege;
                log.card_str = analeptic->getEffectIdString();
                room->sendLog(log);

                CardEffectStruct effect;
                Analeptic *ana = new Analeptic(analeptic->getSuit(), analeptic->getNumber());
                ana->setSkillName(objectName());
                effect.card = ana;
                effect.to = sunce;
                effect.from = sunce;

                room->cardEffect(effect);
                break;
            }
        }

        return false;
    }
};

class Longluo: public TriggerSkill{
public:
    Longluo():TriggerSkill("longluo"){
        events << PhaseChange;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Discard){
            player->tag["cardnum"] = player->getHandcardNum();
        }
        else if(player->getPhase() == Player::Finish){
            int drawnum = player->tag.value("cardnum", 0).toInt() - player->getHandcardNum();
            if(drawnum > 0 && player->askForSkillInvoke(objectName())){
                ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
                if(target)
                    target->drawCards(drawnum);
            }
        }
        return false;
    }
};

class Fuzuo: public TriggerSkill{
public:
    Fuzuo():TriggerSkill("fuzuo"){
        events << Pindian;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *zhangzhao = room->findPlayerBySkillName(objectName());
        if(zhangzhao->askForSkillInvoke(objectName())){
            PindianStar pindian = data.value<PindianStar>();
            const Card *car = room->askForCard(zhangzhao, ".K8", "@fuzuo_card");
            QList<ServerPlayer *> pindians;
            pindians << pindian->from << pindian->to;
            ServerPlayer *target = room->askForPlayerChosen(zhangzhao, pindians, objectName());
            int from = target == pindian->from ? pindian->from_card->getNumber() + car->getNumber()/2 : pindian->from_card->getNumber();
            int to = target == pindian->to ? pindian->to_card->getNumber() + car->getNumber()/2 : pindian->to_card->getNumber();

            pindian->from_card = Sanguosha->cloneCard(pindian->from_card->objectName(), pindian->from_card->getSuit(), from);
            pindian->to_card = Sanguosha->cloneCard(pindian->to_card->objectName(), pindian->to_card->getSuit(), to);

            /*LogMessage log;
            log.type = "$Fuzuo";
            log.from = zhangzhao;
            log.to << target;
            log.card_str = pindian->from_card->getEffectIdString();
            room->sendLog(log);
            */
            data = QVariant::fromValue(pindian);
        }
        return false;
    }
};

class EightPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return ! player->hasEquip(card) && card->getNumber() < 8;
    }
};

class Jincui: public TriggerSkill{
public:
    Jincui():TriggerSkill("jincui"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(room->askForSkillInvoke(player, objectName())){
            ServerPlayer *target = room->askForPlayerChosen(player, room->getAllPlayers(), objectName());
            if (room->askForChoice(player, objectName(), "draw+throw") == "draw")
                target->drawCards(3);
            else
                room->askForDiscard(target, objectName(), 3);
        }
        return false;
    }
};

class Badao: public TriggerSkill{
public:
    Badao():TriggerSkill("badao"){
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *hua, QVariant &data) const{
        Room *room = hua->getRoom();
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("Slash") && effect.card->isBlack()){
            if(room->askForSkillInvoke(hua, objectName())){
                room->askForUseCard(hua, "slash", "badaoslash");
            }
        }
        return false;
    }
};

class Wenjiu: public TriggerSkill{
public:
    Wenjiu():TriggerSkill("wenjiu"){
        events << Predamage << SlashProceed;
        frequency = Compulsory;
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *hua = room->findPlayerBySkillName(objectName());
        if(event == SlashProceed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(effect.to == hua && effect.slash->isRed()){

                LogMessage log;
                log.type = "#Wenjiu1";
                log.from = effect.from;
                log.to << effect.to;
                room->sendLog(log);

                room->slashResult(effect, NULL);
                return true;
            }
        }
        else if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();
            const Card *reason = damage.card;
            if(reason == NULL || damage.from != hua)
                return false;

            if(reason->inherits("Slash") && reason->isBlack()){
                LogMessage log;
                log.type = "#Wenjiu2";
                log.from = hua;
                log.to << damage.to;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(damage.damage + 1);
                hua->getRoom()->sendLog(log);
                damage.damage ++;
                data = QVariant::fromValue(damage);
            }
        }

        return false;
    }
};

class Shipo: public TriggerSkill{
public:
    Shipo():TriggerSkill("shipo"){
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::Judge || player->getJudgingArea().length() == 0)
            return false;
        Room *room = player->getRoom();
        ServerPlayer *tianfeng = room->findPlayerBySkillName(objectName());
        if(tianfeng->getCardCount(true)>=2 && room->askForSkillInvoke(tianfeng, objectName())
                && room->askForDiscard(tianfeng, objectName(),2,false,true)){
            foreach(const Card *jcd, player->getJudgingArea())
                tianfeng->obtainCard(jcd);
        }
        return false;
    }
};

class Gushou:public TriggerSkill{
public:
    Gushou():TriggerSkill("gushou"){
        frequency = Frequent;
        events << CardUsed << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *tianfeng, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->inherits("Jink")){
            Room *room = tianfeng->getRoom();
            if(room->askForSkillInvoke(tianfeng, objectName())){
                room->playSkillEffect(objectName());
                tianfeng->drawCards(1);
            }
        }

        return false;
    }
};

class Yuwen: public TriggerSkill{
public:
    Yuwen():TriggerSkill("yuwen"){
        events << AskForPeachesDone;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *tianfeng, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        damage.from = tianfeng;

        LogMessage log;
        log.type = "#Yuweneffect";
        log.from = tianfeng;
        tianfeng->getRoom()->sendLog(log);

        data = QVariant::fromValue(damage);
        return false;
    }
};

ShouyeCard::ShouyeCard(){
    once = true;
}

bool ShouyeCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;

    if(to_select == Self)
        return false;
    return true;
}

void ShouyeCard::onEffect(const CardEffectStruct &effect) const{
    //room->throwCard(this);

    effect.to->drawCards(1);
    if(effect.from->getMark("jiehuo") == 0)
        effect.from->addMark("shouye");
}

class Shouye: public OneCardViewAsSkill{
public:
    Shouye():OneCardViewAsSkill("shouye"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("ShouyeCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ShouyeCard *shouye_card = new ShouyeCard;
        shouye_card->addSubcard(card_item->getCard()->getId());

        return shouye_card;
    }
};

class Jiehuo: public TriggerSkill{
public:
    Jiehuo():TriggerSkill("jiehuo"){
        events << CardFinished;
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("jiehuo") == 0
                && target->getMark("shouye") > 6;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        //CardUseStruct use = data.value<CardUseStruct>();
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#JiehuoWake";
        log.from = player;
        room->sendLog(log);

        room->setPlayerMark(player, "jiehuo", 1);
        room->setPlayerMark(player, "shouye", 0);
        room->acquireSkill(player, "shien");

        room->loseMaxHp(player);
        return false;
    }
};

class Shien:public TriggerSkill{
public:
    Shien():TriggerSkill("shien"){
        events << CardUsed << CardResponsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->isNDTrick()){
            Room *room = player->getRoom();
            ServerPlayer *shuijing = room->findPlayerBySkillName(objectName());
            if(player != shuijing && room->askForSkillInvoke(player, objectName()))
                shuijing->drawCards(1);
        }

        return false;
    }
};

/*
class Jiehuo: public TriggerSkill{
public:
    Jiehuo():TriggerSkill("jiehuo"){
        events << CardUsed << CardEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *shuijing = room->findPlayerBySkillName(objectName());
        if(event == CardEffected){
            CardEffectStruct effect = data.value<CardEffectStruct>();

            if(!(effect.multiple &&
                (effect.card->inherits("GlobalEffect") ||
                 effect.card->inherits("AOE"))))
                return false;
            foreach(ServerPlayer *p,room->getAlivePlayers())
                if(p->getMark("jiehuo") > 0)
                    return false;
            if(shuijing->getMark("jiehuoover")<1){
                ServerPlayer *target = room->askForPlayerChosen(shuijing, room->getAlivePlayers(), objectName());
                if(target){
                    target->addMark("jiehuo");
                    shuijing->addMark("jiehuoover");
                }
            }
            else if(shuijing->getMark("jiehuoover")>0 && player->getMark("jiehuo")>0){
                player->loseAllMarks("juehuo");
                shuijing->loseAllMarks("jiehuoover");
                return true;
            }
        }
        else if(event == CardUsed){
            CardUseStruct effect = data.value<CardUseStruct>();

            if(effect.card->inherits("TrickCard") &&
               !effect.card->inherits("Collateral") &&
               effect.to.contains(shuijing) && effect.to.length() > 1){
                Room *room = player->getRoom();
                if(room->askForSkillInvoke(shuijing, objectName(), data)){
                    ServerPlayer *target = room->askForPlayerChosen(shuijing, effect.to, objectName());
                    if(target)
                        effect.to.removeOne(target);
                    data = QVariant::fromValue(effect);
                    return false;
                }
            }
        }
        return false;
    }
};
*/

WisdomPackage::WisdomPackage()
    :Package("wisdom")
{
    //智包内厕
    General *wisxuyou,
            *wisjiangwei, *wisjiangwan,
            *wissunce, *wiszhangzhao,
            *wishuaxiong, *wistianfeng, *wisshuijing;
/*魏 许攸 3血 恃才傲物
倨傲:出牌阶段,你可以选择两张手牌移出游戏,指定一名角色,被指定的角色到下个回合开始阶段时,跳过摸牌阶段,得到你所移出游戏的两张牌。每回合限一次。
贪婪:你每受到一次伤害,可与伤害来源进行拼点:若你赢,你获得两张拼点牌。
恃才:锁定技,当你拼点成功时,立即摸一张牌。*/
        wisxuyou = new General(this, "wisxuyou", "wei",3,true); //finished
        wisxuyou->addSkill(new Juao);
        wisxuyou->addSkill(new Tanlan);
        wisxuyou->addSkill(new Shicai);
/*蜀 姜维 4血 天水麒麟
异才:每当你使用一张非延时类锦囊时(在它结算之前),可立即对攻击范围内的角色使用一张【杀】。
北伐:锁定技,当你失去最后一张手牌时,视为对攻击范围内的一名角色使用了一张【杀】。*/
        wisjiangwei = new General(this, "wisjiangwei", "shu",4,true); //finished
        wisjiangwei->addSkill(new Yicai);
        wisjiangwei->addSkill(new Beifa);
/*蜀 蒋琬 3血 武侯后继
后援:出牌阶段,你可以弃掉两张手牌,指定一名其他角色摸两张牌。
筹粮:回合结束阶段,当你手牌少于三张时,你可以从牌堆顶亮出四张牌,拿走其中的基本牌,然后弃掉其余的牌。
*/
        wisjiangwan = new General(this, "wisjiangwan", "shu",3,true); //finished
        wisjiangwan->addSkill(new Houyuan);
        wisjiangwan->addSkill(new Chouliang);
/*吴 孙策 4血 江东小霸王
霸王:当你使用的【杀】被闪避时,你可以和对方拼点:若你赢,可以选择最多两个目标，视为对其分别使用了一张【杀】。
危殆:主公技,当你需要使用一张【酒】时，吴势力角色可以打出一张黑桃2~9的手牌，视为替你出了一张【酒】。(见详解)
*/
        wissunce = new General(this, "wissunce$", "wu",4,true);
        wissunce->addSkill(new Bawang);
        wissunce->addSkill(new Weidai);
/*吴 张昭 3血 东吴重臣
笼络:弃牌阶段,你可以选择一名其他角色摸取与你弃牌数量相同的牌。
辅佐:当有角色拼点时,你可以弃掉自己的一张点数小于8的手牌，让其中一名角色的拼点牌加上这张牌点数的二分之一（向下取整）。
尽瘁:当你死亡时,可令一名角色立即摸取或者弃掉三张牌。
*/
        wiszhangzhao = new General(this, "wiszhangzhao", "wu",3,true);
        wiszhangzhao->addSkill(new Longluo);
        wiszhangzhao->addSkill(new Fuzuo);
        wiszhangzhao->addSkill(new Jincui);

/*群 华雄 4血 心高命薄
霸刀:当你成为黑色的【杀】目标时,你可以立即对你攻击范围内的一名角色使用一张【杀】。
温酒:锁定技,你使用黑色的【杀】造成的伤害+1,你无法闪避红色的【杀】。
*/
        wishuaxiong = new General(this, "wishuaxiong", "qun",4,true);
        wishuaxiong->addSkill(new Badao);
        wishuaxiong->addSkill(new Wenjiu);
/*群 田丰 3血 甘冒虎口
识破:任意角色判定阶段判定前,你可以弃掉两张牌,获得该角色判定区里的所有牌。
固守:当你使用或打出一张【闪】时,可立即摸一张牌。
狱刎:锁定技,当你死亡时,凶手视为自己。
*/
        wistianfeng = new General(this, "wistianfeng", "qun",3,true);
        wistianfeng->addSkill(new Shipo);
        wistianfeng->addSkill(new Gushou);
        wistianfeng->addSkill(new Yuwen);
/*
群 司马徽 3血 水镜先生
授业:出牌阶段,你可以弃掉一张红色手牌,指定最多两名其他角色各摸一张牌。
解惑:当一个锦囊指定了不止一个目标,且你也是其中之一时,你可以指定一名角色跳过该锦囊的结算。
*/
        wisshuijing = new General(this, "wisshuijing", "qun",4,true);
        wisshuijing->addSkill(new Shouye);
        wisshuijing->addSkill(new Jiehuo);

        skills << new Shien;
        patterns[".K8"] = new EightPattern;
        patterns[".S29"] = new SpatwoninePattern;

        addMetaObject<JuaoCard>();
        addMetaObject<BawangCard>();
        addMetaObject<WeidaiCard>();
        addMetaObject<HouyuanCard>();
        addMetaObject<ShouyeCard>();
}

ADD_PACKAGE(Wisdom)
