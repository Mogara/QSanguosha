#include "wisdompackage.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"
#include "room.h"
#include "maneuvering.h"
#include "general.h"

JuaoCard::JuaoCard(){
    once = true;
    will_throw = false;
}

bool JuaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return true;
}

void JuaoCard::onEffect(const CardEffectStruct &effect) const{
    foreach(int cardid, this->getSubcards()){
        //source->getRoom()->moveCardTo(Sanguosha->getCard(cardid), targets.first(), Player::Special);
        effect.to->addToPile("hautain", cardid, false);
    }
    effect.to->addMark("juao");
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
            player->setMark("juao", 0);
            ServerPlayer *xuyou = room->findPlayerBySkillName(objectName());
            foreach(int card_id, player->getPile("hautain")){
                if(!xuyou)
                    room->throwCard(card_id);
                else
                    room->obtainCard(player, card_id);
            }
            if(!xuyou)
                return false;

            LogMessage log;
            log.type = "#JuaoObtain";
            log.from = player;
            log.arg = objectName();
            log.to << xuyou;
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
            if(from && !from->isKongcheng() && !xuyou->isKongcheng() && room->askForSkillInvoke(xuyou, objectName(), data)){
                xuyou->pindian(from, "tanlan");
            }
            return false;
        }
        else {
            PindianStar pindian = data.value<PindianStar>();
            if(pindian->reason == "tanlan" && pindian->isSuccess()){
                xuyou->obtainCard(pindian->to_card);
                xuyou->obtainCard(pindian->from_card);
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
        if(!xuyou) return false;
        PindianStar pindian = data.value<PindianStar>();
        if(pindian->from != xuyou && pindian->to != xuyou)
            return false;
        ServerPlayer *winner = pindian->from_card->getNumber() > pindian->to_card->getNumber() ? pindian->from : pindian->to;
        if(winner == xuyou){
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = xuyou;
            log.arg = objectName();
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
            if(!room->askForSkillInvoke(jiangwei, objectName(), data))
                return false;
            room->throwCard(card);
            room->askForUseCard(jiangwei, "slash", "@askforslash");
        }
        return false;
    }
};

class Beifa: public TriggerSkill{
public:
    Beifa():TriggerSkill("beifa"){
        events << CardLostOnePiece;
        frequency = Compulsory;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *jiangwei, QVariant &data) const{
        if(jiangwei->isKongcheng()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place != Player::Hand)
                return false;

            Room *room = jiangwei->getRoom();
            QList<ServerPlayer *> players;
            foreach(ServerPlayer *player, room->getOtherPlayers(jiangwei)){
                if(player->hasSkill("kongcheng") && player->isKongcheng())
                    continue;
                if(jiangwei->inMyAttackRange(player))
                    players << player;
            }

            ServerPlayer *target = jiangwei;
            if(!players.isEmpty())
                target = room->askForPlayerChosen(jiangwei, players, objectName());
            //if(!target)
            //    target = jiangwei;

            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName(objectName());
            CardUseStruct use;
            use.card = slash;
            use.from = jiangwei;
            use.to << target;

            room->useCard(use);
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
        return !to_select->isEquipped() && selected.length() < 2;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("HouyuanCard");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;
        HouyuanCard *card = new HouyuanCard;
        card->addSubcards(cards);
        return card;
    }
};

class Chouliang: public PhaseChangeSkill{
public:
    Chouliang():PhaseChangeSkill("chouliang"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        int handcardnum = player->getHandcardNum();
        if(player->getPhase() == Player::Finish && handcardnum < 3
           && room->askForSkillInvoke(player, objectName())){
            for(int i = 0; i < 4 - handcardnum; i++){
                int card_id = room->drawCard();
                room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::PlaceTakeoff, true);
                room->getThread()->delay();

                const Card *card = Sanguosha->getCard(card_id);
                if(!card->inherits("BasicCard")){
                    room->throwCard(card_id);
                    room->setEmotion(player, "bad");
                }
                else{
                    LogMessage log;
                    log.type = "$TakeAG";
                    log.from = player;
                    log.card_str = QString::number(card_id);
                    room->sendLog(log);

                    room->obtainCard(player, card_id);
                    room->setEmotion(player, "good");
                }
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
            if(room->askForSkillInvoke(sunce, objectName(), data)){
                bool success = sunce->pindian(effect.to, objectName(), NULL);
                if(success){
                    if(sunce->hasFlag("drank"))
                        room->setPlayerFlag(sunce, "-drank");
                    room->askForUseCard(sunce, "@@bawang", "@bawang");
                }
            }
        }
        return false;
    }
};

WeidaiCard::WeidaiCard(){
    target_fixed = true;
}

void WeidaiCard::use(Room *room, ServerPlayer *sunce, const QList<ServerPlayer *> &) const{
    if(sunce->hasFlag("drank") || !sunce->hasLordSkill("weidai"))
        return;
    foreach(ServerPlayer *liege, room->getAlivePlayers()){
        if(liege->getKingdom() != "wu")
            continue;
        if(sunce->getHp() > 0 && sunce->hasUsed("Analeptic"))
            return;
        QVariant tohelp = QVariant::fromValue((PlayerStar)sunce);
        QString prompt = QString("@weidai-analeptic:%1").arg(sunce->objectName());
        const Card *analeptic = room->askForCard(liege, ".|spade|2~9|hand", prompt, tohelp);
        if(analeptic){
            LogMessage log;
            log.type = "$DiscardCard";
            log.from = liege;
            log.card_str = analeptic->getEffectIdString();
            room->sendLog(log);

            Analeptic *ana = new Analeptic(analeptic->getSuit(), analeptic->getNumber());
            ana->setSkillName("weidai");
            CardUseStruct use;
            use.card = ana;
            use.from = sunce;
            use.to << use.from;

            room->useCard(use);
            if(sunce->getHp() > 0)
                break;
        }
    }
}

class WeidaiViewAsSkill:public ZeroCardViewAsSkill{
public:
    WeidaiViewAsSkill():ZeroCardViewAsSkill("weidai$"){
    }

    virtual bool isEnabledAtPlay(const Player *player){
        return player->hasLordSkill("weidai")
                && !player->hasUsed("Analeptic")
                && !player->hasUsed("WeidaiCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@weidai";
    }

    virtual const Card *viewAs() const{
        return new WeidaiCard;
    }
};

class Weidai: public TriggerSkill{
public:
    Weidai():TriggerSkill("weidai$"){
        events << Dying;
        default_choice = "ignore";
        view_as_skill = new WeidaiViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("weidai");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *sunce, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        if(dying.who != sunce)
            return false;

        sunce->getRoom()->askForUseCard(sunce, "@@weidai", "@weidai");

        return false;
    }
};

class Longluo: public TriggerSkill{
public:
    Longluo():TriggerSkill("longluo"){
        events << CardLostOnePiece << PhaseChange;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == PhaseChange){
            if(player->getPhase() == Player::Finish){
                int drawnum = player->getMark("longluo");
                if(drawnum > 0 && player->askForSkillInvoke(objectName(), data)){
                    ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
                    target->drawCards(drawnum);
                }
            }
            else if(player->getPhase() == Player::RoundStart){
                player->setMark("longluo", 0);
            }
            return false;
        }
        if(player->getPhase() == Player::Discard){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->to_place == Player::DiscardPile){
                player->addMark("longluo");
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
        return 0;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *zhangzhao = room->findPlayerBySkillName(objectName());
        if(!zhangzhao)
            return false;
        PindianStar pindian = data.value<PindianStar>();
        QStringList choices;
        choices << pindian->from->getGeneralName()
                << pindian->to->getGeneralName() << "cancel";

        LogMessage log;
        log.type = "$Fuzuo_from";
        log.from = pindian->from;
        log.to << pindian->to;
        log.card_str = pindian->from_card->getEffectIdString();
        room->sendLog(log);
        log.type = "$Fuzuo_to";
        log.from = zhangzhao;
        log.card_str = pindian->to_card->getEffectIdString();
        room->sendLog(log);

        QString choice = room->askForChoice(zhangzhao, objectName(), choices.join("+"));
        if(choice == "cancel")
            return false;
        const Card *intervention = room->askForCard(zhangzhao, ".|.|~7|hand", "@fuzuo_card");
        if(intervention){
            log.type = "$Fuzuo";
            log.to.clear();
            ServerPlayer *target = zhangzhao;
            if(choice == pindian->from->getGeneralName())
                target = pindian->from;
            else
                target = pindian->to;
            log.to << target;
            log.card_str = intervention->getEffectIdString();
            room->sendLog(log);

            const Card *pindian_card = target == pindian->from ? pindian->from_card : pindian->to_card;
            int num = pindian_card->getNumber() + intervention->getNumber() / 2;
            Card *use_card = Sanguosha->cloneCard(pindian_card->objectName(), pindian_card->getSuit(), num);
            use_card->setSkillName(objectName());
            use_card->addSubcard(pindian_card);
            if(target == pindian->from)
                pindian->from_card = use_card;
            else
                pindian->to_card = use_card;

            data = QVariant::fromValue(pindian);
        }
        return false;
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
        if(room->askForSkillInvoke(player, objectName(), data)){
            ServerPlayer *target = room->askForPlayerChosen(player, room->getAllPlayers(), objectName());
            if (room->askForChoice(player, objectName(), "draw+throw") == "draw")
                target->drawCards(3);
            else
                room->askForDiscard(target, objectName(), qMin(3,target->getCardCount(true)), false, true);
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
            if(room->askForSkillInvoke(hua, objectName(), data)){
                room->askForUseCard(hua, "slash", "@askforslash");
            }
        }
        return false;
    }
};

class Wenjiu: public TriggerSkill{
public:
    Wenjiu():TriggerSkill("wenjiu"){
        events << DamageProceed << SlashProceed;
        frequency = Compulsory;
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *hua = room->findPlayerBySkillName(objectName());
        if(!hua)
            return false;
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
        else if(event == DamageProceed){
            DamageStruct damage = data.value<DamageStruct>();
            const Card *reason = damage.card;
            if(!reason || damage.from != hua)
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
        QList<ServerPlayer *> tians = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *tianfeng, tians){
            if(tianfeng->getCardCount(true)>=2
               && room->askForSkillInvoke(tianfeng, objectName(), QVariant::fromValue(player))
                && room->askForDiscard(tianfeng, objectName(), 2, false, true)){

                foreach(const Card *jcd, player->getJudgingArea())
                    tianfeng->obtainCard(jcd);
                break;
            }
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
        Room *room = tianfeng->getRoom();
        if(room->getCurrent() == tianfeng)
            return false;
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->inherits("BasicCard") && !card->isVirtualCard()){
            if(room->askForSkillInvoke(tianfeng, objectName(), data)){
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
        events << GameOverJudge;
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *tianfeng, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();

        if(damage){
            if(damage->from == tianfeng)
                return false;
        }else{
            damage = new DamageStruct;
            damage->to = tianfeng;
            data = QVariant::fromValue(damage);
        }

        damage->from = tianfeng;

        LogMessage log;
        log.type = "#YuwenEffect";
        log.from = tianfeng;
        log.arg = objectName();
        tianfeng->getRoom()->sendLog(log);

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
    effect.to->drawCards(1);
    if(effect.from->getMark("jiehuo") == 0)
        effect.from->gainMark("@shouye");
}

class Shouye: public OneCardViewAsSkill{
public:
    Shouye():OneCardViewAsSkill("shouye"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->getMark("shouyeonce") > 0)
            return ! player->hasUsed("ShouyeCard");
        else
            return true;
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
                && target->getMark("@shouye") > 6;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#JiehuoWake";
        log.from = player;
        log.arg = "jiehuo";
        log.arg2 = "shouye";
        room->sendLog(log);

        room->setPlayerMark(player, "jiehuo", 1);
        player->loseAllMarks("@shouye");
        room->setPlayerMark(player, "shouyeonce", 1);
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
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->getMark("forbid_shien") > 0)
            return false;
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->isNDTrick()){
            Room *room = player->getRoom();
            ServerPlayer *shuijing = room->findPlayerBySkillName(objectName());
            if(shuijing->isAlive()){
                if(room->askForSkillInvoke(player, objectName(), QVariant::fromValue(shuijing)))
                    shuijing->drawCards(1);
                else{
                    QString dontaskmeneither = room->askForChoice(player, "forbid_shien", "yes+no");
                    if(dontaskmeneither == "yes"){
                        player->setMark("forbid_shien", 1);
                    }
                }
            }
        }

        return false;
    }
};

WisdomPackage::WisdomPackage()
    :Package("wisdom")
{

    General *wisxuyou,
            *wisjiangwei, *wisjiangwan,
            *wissunce, *wiszhangzhao,
            *wishuaxiong, *wistianfeng, *wisshuijing;

        wisxuyou = new General(this, "wisxuyou", "wei",3,true);
        wisxuyou->addSkill(new Juao);
        wisxuyou->addSkill(new Tanlan);
        wisxuyou->addSkill(new Shicai);

        wisjiangwei = new General(this, "wisjiangwei", "shu",4,true);
        wisjiangwei->addSkill(new Yicai);
        wisjiangwei->addSkill(new Beifa);

        wisjiangwan = new General(this, "wisjiangwan", "shu",3,true);
        wisjiangwan->addSkill(new Houyuan);
        wisjiangwan->addSkill(new Chouliang);

        wissunce = new General(this, "wissunce$", "wu",4,true);
        wissunce->addSkill(new Bawang);
        wissunce->addSkill(new Weidai);

        wiszhangzhao = new General(this, "wiszhangzhao", "wu",3,true);
        wiszhangzhao->addSkill(new Longluo);
        wiszhangzhao->addSkill(new Fuzuo);
        wiszhangzhao->addSkill(new Jincui);

        wishuaxiong = new General(this, "wishuaxiong", "qun",4,true);
        wishuaxiong->addSkill(new Badao);
        wishuaxiong->addSkill(new Wenjiu);

        wistianfeng = new General(this, "wistianfeng", "qun",3,true);
        wistianfeng->addSkill(new Shipo);
        wistianfeng->addSkill(new Gushou);
        wistianfeng->addSkill(new Yuwen);

        wisshuijing = new General(this, "wisshuijing", "qun",4,true);
        wisshuijing->addSkill(new Shouye);
        wisshuijing->addSkill(new Jiehuo);

        skills << new Shien;

        addMetaObject<JuaoCard>();
        addMetaObject<BawangCard>();
        addMetaObject<WeidaiCard>();
        addMetaObject<HouyuanCard>();
        addMetaObject<ShouyeCard>();
}

ADD_PACKAGE(Wisdom)
