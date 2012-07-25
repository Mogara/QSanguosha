#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "serverplayer.h"
#include "room.h"
#include "standard-skillcards.h"
#include "ai.h"

class Jianxiong:public MasochismSkill{
public:
    Jianxiong():MasochismSkill("jianxiong"){
    }

    virtual void onDamaged(ServerPlayer *caocao, const DamageStruct &damage) const{
        Room *room = caocao->getRoom();
        const Card *card = damage.card;
        if(card && room->getCardPlace(card->getEffectiveId()) == Player::PlaceTable){

            QVariant data = QVariant::fromValue(card);
            if(room->askForSkillInvoke(caocao, "jianxiong", data)){
            room->broadcastSkillInvoke(objectName());
                caocao->obtainCard(card);
            }
        }
    }
};

class Hujia:public TriggerSkill{
public:
    Hujia():TriggerSkill("hujia$"){
        events << CardAsked;
        default_choice = "ignore";
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasLordSkill("hujia");
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *caocao, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "jink")
            return false;

        QList<ServerPlayer *> lieges = room->getLieges("wei", caocao);
        if(lieges.isEmpty())
            return false;

        if(!room->askForSkillInvoke(caocao, objectName()))
            return false;

        room->broadcastSkillInvoke(objectName());
        QVariant tohelp = QVariant::fromValue((PlayerStar)caocao);
        foreach(ServerPlayer *liege, lieges){
            const Card *jink = room->askForCard(liege, "jink", "@hujia-jink:" + caocao->objectName(), tohelp);
            if(jink){
                room->provide(jink);
                return true;
            }
        }

        return false;
    }
};

class TuxiViewAsSkill: public ZeroCardViewAsSkill{
public:
    TuxiViewAsSkill():ZeroCardViewAsSkill("tuxi"){
    }

    virtual const Card *viewAs() const{
        return new TuxiCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@tuxi";
    }
};

class Tuxi:public PhaseChangeSkill{
public:
    Tuxi():PhaseChangeSkill("tuxi"){
        view_as_skill = new TuxiViewAsSkill;
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

            if(can_invoke && room->askForUseCard(zhangliao, "@@tuxi", "@tuxi-card"))
                return true;
        }

        return false;
    }
};

class Tiandu:public TriggerSkill{
public:
    Tiandu():TriggerSkill("tiandu"){
        frequency = Frequent;
        default_choice = "no";
        events << FinishJudge;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *guojia, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        CardStar card = judge->card;

        QVariant data_card = QVariant::fromValue(card);
        if(guojia->askForSkillInvoke(objectName(), data_card)){
            if(card->objectName() == "shit"){
                QString result = room->askForChoice(guojia, objectName(), "yes+no");
                if(result == "no")
                    return false;
            }

            guojia->obtainCard(judge->card);
            room->broadcastSkillInvoke(objectName());
            room->getThread()->delay(500);

            return true;
        }

        return false;
    }
};

class Yiji:public MasochismSkill{
public:
    Yiji():MasochismSkill("yiji"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *guojia, const DamageStruct &damage) const{
        Room *room = guojia->getRoom();

        if(!room->askForSkillInvoke(guojia, objectName()))
            return;

        room->broadcastSkillInvoke(objectName());

        int x = damage.damage, i;
        for(i=0; i<x; i++){
            guojia->drawCards(2, true, objectName());
            QList<int> yiji_cards;
            foreach(const Card *card, guojia->getHandcards()){
                if(card->hasFlag(objectName())){
                    room->setCardFlag(card, "-" + objectName());
                    yiji_cards << card->getEffectiveId();
                }
            }

            if(yiji_cards.isEmpty())
                continue;

            while(room->askForYiji(guojia, yiji_cards))
                ; // empty loop
        }

    }
};

class Ganglie:public MasochismSkill{
public:
    Ganglie():MasochismSkill("ganglie"){

    }

    virtual void onDamaged(ServerPlayer *xiahou, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = xiahou->getRoom();
        QVariant source = QVariant::fromValue(from);

        if(from && from->isAlive() && room->askForSkillInvoke(xiahou, "ganglie", source)){
            room->broadcastSkillInvoke(objectName());

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = xiahou;

            room->judge(judge);
            if(judge.isGood()){
                if(!room->askForDiscard(from, objectName(), 2, 2, true)){
                    DamageStruct damage;
                    damage.from = xiahou;
                    damage.to = from;

                    room->setEmotion(xiahou, "good");
                    room->damage(damage);
                }
            }else
                room->setEmotion(xiahou, "bad");
        }
    }
};

class Fankui:public MasochismSkill{
public:
    Fankui():MasochismSkill("fankui"){

    }

    virtual void onDamaged(ServerPlayer *simayi, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = simayi->getRoom();
        QVariant data = QVariant::fromValue(from);
        if(from && !from->isNude() && room->askForSkillInvoke(simayi, "fankui", data)){
            room->broadcastSkillInvoke(objectName());
            int card_id = room->askForCardChosen(simayi, from, "he", "fankui");
            CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, simayi->objectName());
            room->obtainCard(simayi, Sanguosha->getCard(card_id), reason, room->getCardPlace(card_id) != Player::PlaceHand);
        }
    }
};

class GuicaiViewAsSkill:public OneCardViewAsSkill{
public:
    GuicaiViewAsSkill():OneCardViewAsSkill(""){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@guicai";
    }

    virtual bool viewFilter(const Card* to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *card = new GuicaiCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class Guicai: public TriggerSkill{
public:
    Guicai():TriggerSkill("guicai"){
        view_as_skill = new GuicaiViewAsSkill;

        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && TriggerSkill::triggerable(target) && !target->isKongcheng();
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@guicai-card" << judge->who->objectName()
                << objectName() << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");
        const Card *card = room->askForCard(player, "@guicai", prompt, data, AskForRetrial);

        if(player->hasSkill("jilve"))
            room->retrial(card, player, judge, "jilve");
        else
            room->retrial(card, player, judge, objectName());

        return false;
    }
};

class LuoyiBuff: public TriggerSkill{
public:
    LuoyiBuff():TriggerSkill("#luoyi"){
        events << ConfirmDamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasFlag("luoyi") && target->isAlive();
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *xuchu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        const Card *reason = damage.card;
        if(reason == NULL)
            return false;

        if(reason->isKindOf("Slash") || reason->isKindOf("Duel")){
            LogMessage log;
            log.type = "#LuoyiBuff";
            log.from = xuchu;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            xuchu->getRoom()->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class Luoyi: public DrawCardsSkill{
public:
    Luoyi():DrawCardsSkill("luoyi"){

    }

    virtual int getDrawNum(ServerPlayer *xuchu, int n) const{
        Room *room = xuchu->getRoom();
        if(room->askForSkillInvoke(xuchu, objectName())){
            room->broadcastSkillInvoke(objectName());

            xuchu->setFlags(objectName());
            return n - 1;
        }else
            return n;
    }
};

class Luoshen:public TriggerSkill{
public:
    Luoshen():TriggerSkill("luoshen"){
        events << EventPhaseStart << FinishJudge;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *zhenji, QVariant &data) const{
        if(triggerEvent == EventPhaseStart && zhenji->getPhase() == Player::Start){
            int n = 0;
            while(zhenji->askForSkillInvoke("luoshen")){
                if(n == 0)
                    room->broadcastSkillInvoke(objectName());
                n++;

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade|club):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = zhenji;
                judge.time_consuming = true;

                room->judge(judge);
                if(judge.isBad())
                    break;
            }

        }else if(triggerEvent == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == objectName()){
                if(judge->card->isBlack()){
                    zhenji->obtainCard(judge->card);
                    return true;
                }
            }
        }

        return false;
    }
};

class Qingguo:public OneCardViewAsSkill{
public:
    Qingguo():OneCardViewAsSkill("qingguo"){

    }

    virtual bool viewFilter(const Card* to_select) const{
        return to_select->isBlack() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        
        Jink *jink = new Jink(originalCard->getSuit(), originalCard->getNumber());
        jink->setSkillName(objectName());
        jink->addSubcard(originalCard->getId());
        return jink;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "jink";
    }
};

class RendeViewAsSkill:public ViewAsSkill{
public:
    RendeViewAsSkill():ViewAsSkill("rende"){
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if(ServerInfo.GameMode == "04_1v3"
           && selected.length() + Self->getMark("rende") >= 2)
           return false;
        else
            return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        RendeCard *rende_card = new RendeCard;
        rende_card->addSubcards(cards);
        return rende_card;
    }
};

class Rende: public PhaseChangeSkill{
public:
    Rende():PhaseChangeSkill("rende"){
        view_as_skill = new RendeViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::NotActive
                && target->hasUsed("RendeCard");
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        target->getRoom()->setPlayerMark(target, "rende", 0);

        return false;
    }
};

class JijiangViewAsSkill:public ZeroCardViewAsSkill{
public:
    JijiangViewAsSkill():ZeroCardViewAsSkill("jijiang$"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasLordSkill("jijiang") && Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return new JijiangCard;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "slash" && !ClientInstance->hasNoTargetResponsing() && !player->hasFlag("jijiang_failed");
    }
};

class Jijiang: public TriggerSkill{
public:
    Jijiang():TriggerSkill("jijiang$"){
        events << CardAsked;
        default_choice = "ignore";

        view_as_skill = new JijiangViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasLordSkill("jijiang");
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *liubei, QVariant &data) const{
        if (liubei == NULL) return false;
        QString pattern = data.toString();
        if(pattern != "slash")
            return false;
                
        QList<ServerPlayer *> lieges = room->getLieges("shu", liubei);
        if(lieges.isEmpty())
            return false;

        if(!room->askForSkillInvoke(liubei, objectName()))
            return false;

        room->broadcastSkillInvoke(objectName(), getEffectIndex(liubei, NULL));

        QVariant tohelp = QVariant::fromValue((PlayerStar)liubei);
        foreach(ServerPlayer *liege, lieges){
            const Card *slash = room->askForCard(liege, "slash", "@jijiang-slash:" + liubei->objectName(), tohelp);
            if(slash){
                room->provide(slash);
                return true;
            }
        }

        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *player, const Card *) const{
        int r = 1 + qrand() % 2;
        if(player->getGeneralName() == "liushan" || player->getGeneral2Name() == "liushan")
            r += 2;

        return r;
    }
};

class Wusheng:public OneCardViewAsSkill{
public:
    Wusheng():OneCardViewAsSkill("wusheng"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "slash";
    }

    virtual bool viewFilter(const Card* card) const{
        if(!card->isRed())
            return false;

        if(Self->getWeapon() && card->getEffectiveId() == Self->getWeapon()->getId() && card->objectName() == "Crossbow")
            return Self->canSlashWithoutCrossbow();
        else
            return true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        
        Card *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
        slash->addSubcard(originalCard->getId());
        slash->setSkillName(objectName());
        return slash;
    }
};

class Longdan:public OneCardViewAsSkill{
public:
    Longdan():OneCardViewAsSkill("longdan"){

    }

    virtual bool viewFilter(const Card* to_select) const{
        const Card *card = to_select;

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                // jink as slash
                return card->isKindOf("Jink");
            }

        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                if(pattern == "slash")
                    return card->isKindOf("Jink");
                else if(pattern == "jink")
                    return card->isKindOf("Slash");
            }

        default:
            return false;
        }
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "jink" || pattern == "slash";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        if(originalCard->isKindOf("Slash")){
            Jink *jink = new Jink(originalCard->getSuit(), originalCard->getNumber());
            jink->addSubcard(originalCard);
            jink->setSkillName(objectName());
            return jink;
        }else if(originalCard->isKindOf("Jink")){
            Slash *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
            slash->addSubcard(originalCard);
            slash->setSkillName(objectName());
            return slash;
        }else
            return NULL;
    }
};

class Tieji:public TriggerSkill{
public:
    Tieji():TriggerSkill("tieji"){
        events << TargetConfirmed << SlashProceed << CardFinished;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{

        if(triggerEvent == TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(!use.card->isKindOf("Slash") || use.from != player)
                   return false;

            foreach(ServerPlayer *target, use.to){
                bool invoke = player->askForSkillInvoke("tieji", QVariant::fromValue(target));
                JudgeStruct judge;
                if(invoke){
                    room->broadcastSkillInvoke(objectName());

                    judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                    judge.good = true;
                    judge.reason = objectName();
                    judge.who = player;
                    judge.play_animation = true;

                    room->judge(judge);
                }
                QVariantList tiejiList = target->tag["Tieji"].toList();
                tiejiList << (invoke && judge.isGood());
                target->tag["Tieji"] = tiejiList;
                target->setFlags("TiejiTarget");
            }
        }
        else if(triggerEvent == SlashProceed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            effect.to->setFlags("-TiejiTarget");
            QVariantList tiejiList = effect.to->tag["Tieji"].toList();
            if(!tiejiList.isEmpty()){
                bool hit = tiejiList.takeFirst().toBool();
                effect.to->tag["Tieji"] = tiejiList;
                if(hit){
                    room->slashResult(effect, NULL);
                    return true;
                }
            }
        }
        else if(triggerEvent == CardFinished){
            CardUseStruct use = data.value<CardUseStruct>();
            foreach(ServerPlayer *target, use.to){
                if(target->hasFlag("TiejiTarget"))
                    target->tag.remove("Tieji");
            }
        }
        return false;
    }
};

class Guanxing:public PhaseChangeSkill{
public:
    Guanxing():PhaseChangeSkill("guanxing"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const{
        if(zhuge->getPhase() == Player::Start &&
           zhuge->askForSkillInvoke(objectName()))
        {
            Room *room = zhuge->getRoom();
            room->broadcastSkillInvoke(objectName());

            int n = qMin(5, room->alivePlayerCount());
            room->askForGuanxing(zhuge, room->getNCards(n, false), false);
        }

        return false;
    }
};

class Kongcheng: public ProhibitSkill{
public:
    Kongcheng():ProhibitSkill("kongcheng"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        if(card->isKindOf("Slash") || card->isKindOf("Duel"))
            return to->isKongcheng();
        else
            return false;
    }
};

class KongchengEffect: public TriggerSkill{
public:
    KongchengEffect():TriggerSkill("#kongcheng-effect"){
        frequency = Compulsory;

        events << CardsMoveOneTime;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->isKongcheng()){
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            if(move->from == player && move->from_places.contains(Player::PlaceHand))
                room->broadcastSkillInvoke("kongcheng");
        }

        return false;
    }
};

class Jizhi:public TriggerSkill{
public:
    Jizhi():TriggerSkill("jizhi"){
        frequency = Frequent;
        events << CardUsed << CardResponsed;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *yueying, QVariant &data) const{
        if (yueying == NULL) return false;
        CardStar card = NULL;
        if(triggerEvent == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(triggerEvent == CardResponsed)
            card = data.value<CardStar>();

        if(card->isNDTrick()){            
            if(room->askForSkillInvoke(yueying, objectName())){
                room->broadcastSkillInvoke(objectName());
                yueying->drawCards(1);
            }
        }

        return false;
    }
};

class Zhiheng:public ViewAsSkill{
public:
    Zhiheng():ViewAsSkill("zhiheng"){

    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *) const{
        return true;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        ZhihengCard *zhiheng_card = new ZhihengCard;
        zhiheng_card->addSubcards(cards);

        return zhiheng_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ZhihengCard");
    }
};

class Jiuyuan: public TriggerSkill{
public:
    Jiuyuan():TriggerSkill("jiuyuan$"){
        events << Dying << AskForPeachesDone << TargetConfirmed << HpRecover;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasLordSkill(objectName());
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *sunquan, QVariant &data) const{
        switch(triggerEvent){
        case Dying: {
                foreach(ServerPlayer *wu, room->getOtherPlayers(sunquan)){
                    if(wu->getKingdom() == "wu"){
                        room->broadcastSkillInvoke("jiuyuan", 1);
                        break;
                    }
                }

                break;
            }

        case TargetConfirmed: {
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->isKindOf("Peach") && use.from && use.from->getKingdom() == "wu"
                    && sunquan != use.from && sunquan->hasFlag("dying"))
            {
                room->setPlayerFlag(sunquan, "jiuyuan");
                room->setCardFlag(use.card, "jiuyuan");
            }
            break;
        }

        case HpRecover: {
            RecoverStruct rec = data.value<RecoverStruct>();
            if(rec.card && rec.card->hasFlag("jiuyuan"))
            {
                int index = rec.who->getGeneral()->isMale() ? 2 : 3;

                room->broadcastSkillInvoke("jiuyuan", index);

                LogMessage log;
                log.type = "#JiuyuanExtraRecover";
                log.from = sunquan;
                log.to << rec.who;
                log.arg = objectName();
                room->sendLog(log);

                rec.recover ++;

                data = QVariant::fromValue(rec);
            }
            break;
        }

        case AskForPeachesDone:{
                if(sunquan->hasFlag("jiuyuan")){
                    room->setPlayerFlag(sunquan, "-jiuyuan");
                    if(sunquan->getHp() > 0){
                        room->getThread()->delay(2000);
                        room->broadcastSkillInvoke("jiuyuan", 4);
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

class Yingzi:public DrawCardsSkill{
public:
    Yingzi():DrawCardsSkill("yingzi"){
        frequency = Frequent;
    }

    virtual int getDrawNum(ServerPlayer *zhouyu, int n) const{
        Room *room = zhouyu->getRoom();
        if(room->askForSkillInvoke(zhouyu, objectName())){
            room->broadcastSkillInvoke(objectName());
            return n + 1;
        }else
            return n;
    }
};

class Fanjian:public ZeroCardViewAsSkill{
public:
    Fanjian():ZeroCardViewAsSkill("fanjian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && ! player->hasUsed("FanjianCard");
    }

    virtual const Card *viewAs() const{
        return new FanjianCard;
    }
};

class Keji: public TriggerSkill{
public:
    Keji():TriggerSkill("keji"){
        events << EventPhaseChanging << CardResponsed;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *lvmeng, QVariant &data) const{
        if(triggerEvent == CardResponsed && lvmeng->getPhase() == Player::Play){
            CardStar card_star = data.value<CardStar>();
            if(card_star->isKindOf("Slash"))
                lvmeng->setFlags("keji_use_slash");
        }
        else if(triggerEvent == EventPhaseChanging)
        {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if(change.to == Player::Discard){
                if(!lvmeng->hasFlag("keji_use_slash") &&
                        lvmeng->getSlashCount() == 0 &&
                        lvmeng->askForSkillInvoke("keji"))
                {
                    room->broadcastSkillInvoke("keji");

                    lvmeng->skip(Player::Discard);
                }
            }
        }
        return false;
    }
};

class Lianying: public TriggerSkill{
public:
    Lianying():TriggerSkill("lianying"){
        events << CardsMoveOneTime;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , Room* room, ServerPlayer *luxun, QVariant &data) const{
        CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
        if(move->from == luxun && move->from_places.contains(Player::PlaceHand) && luxun->isKongcheng()){
            if(room->askForSkillInvoke(luxun, objectName(), data)){
                room->broadcastSkillInvoke(objectName());
                luxun->drawCards(1);
            }
        }
        return false;
    }
};

class Qixi: public OneCardViewAsSkill{
public:
    Qixi():OneCardViewAsSkill("qixi"){

    }

    virtual bool viewFilter(const Card* to_select) const{
        return to_select->isBlack();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Dismantlement *dismantlement = new Dismantlement(originalCard->getSuit(), originalCard->getNumber());
        dismantlement->addSubcard(originalCard->getId());
        dismantlement->setSkillName(objectName());
        return dismantlement;
    }
};

class Kurou: public ZeroCardViewAsSkill{
public:
    Kurou():ZeroCardViewAsSkill("kurou"){

    }

    virtual const Card *viewAs() const{
        return new KurouCard;
    }
};

class Guose: public OneCardViewAsSkill{
public:
    Guose():OneCardViewAsSkill("guose"){

    }

    virtual bool viewFilter(const Card* to_select) const{
        return to_select->getSuit() == Card::Diamond;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Indulgence *indulgence = new Indulgence(originalCard->getSuit(), originalCard->getNumber());
        indulgence->addSubcard(originalCard->getId());
        indulgence->setSkillName(objectName());
        return indulgence;
    }
};

class LiuliViewAsSkill: public OneCardViewAsSkill{
public:
    LiuliViewAsSkill():OneCardViewAsSkill("liuli"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@liuli";
    }

    virtual bool viewFilter(const Card *) const{
        return true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        LiuliCard *liuli_card = new LiuliCard;
        liuli_card->addSubcard(originalCard);

        return liuli_card;
    }
};

class Liuli: public TriggerSkill{
public:
    Liuli():TriggerSkill("liuli"){
        view_as_skill = new LiuliViewAsSkill;

        events << TargetConfirming;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *daqiao, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();

        if(use.card && use.card->isKindOf("Slash") && use.to.contains(daqiao) && !daqiao->isNude() && room->alivePlayerCount() > 2){
            QList<ServerPlayer *> players = room->getOtherPlayers(daqiao);
            players.removeOne(use.from);

            bool can_invoke = false;
            foreach(ServerPlayer *p, players){
                if(daqiao->inMyAttackRange(p)){
                    can_invoke = true;
                    break;
                }
            }

            if(can_invoke){
                QString prompt = "@liuli:" + use.from->objectName();
                room->setPlayerFlag(use.from, "slash_source");
                if(room->askForUseCard(daqiao, "@@liuli", prompt)){
                    foreach(ServerPlayer *p, players){
                        if(p->hasFlag("liuli_target")){
                            use.to.insert(use.to.indexOf(daqiao), p);
                            use.to.removeOne(daqiao);

                            data = QVariant::fromValue(use);

                            room->setPlayerFlag(use.from, "-slash_source");
                            room->setPlayerFlag(p, "-liuli_target");
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }
};

class Jieyin: public ViewAsSkill{
public:
    Jieyin():ViewAsSkill("jieyin"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("JieyinCard");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if(selected.length() > 1)
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        JieyinCard *jieyin_card = new JieyinCard();
        jieyin_card->addSubcards(cards);

        return jieyin_card;
    }
};

class Xiaoji: public TriggerSkill{
public:
    Xiaoji():TriggerSkill("xiaoji"){
        events << CardsMoveOneTime;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *sunshangxiang, QVariant &data) const{
        CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
        if(move->from == sunshangxiang && move->from_places.contains(Player::PlaceEquip)){
            int n = 0;
            foreach(Player::Place place, move->from_places)
                if(place == Player::PlaceEquip)
                    n ++;

            for(int i = 0; i < n; i++)
            if(room->askForSkillInvoke(sunshangxiang, objectName())){
                if(i == 0)
                    room->broadcastSkillInvoke(objectName());
                sunshangxiang->drawCards(2);
            }
        }

        return false;
    }
};

class Wushuang:public TriggerSkill{
public:
    Wushuang():TriggerSkill("wushuang"){
        frequency = Compulsory;
        events << TargetConfirmed << SlashProceed;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(triggerEvent == TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->isKindOf("Slash") && use.from == player){
                room->setCardFlag(use.card, "WushuangInvke");
            }
            else if(use.card->isKindOf("Duel")){
                room->setCardFlag(use.card, "WushuangInvke");
            }
        }
        else if(triggerEvent == SlashProceed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(!effect.slash->hasFlag("WushuangInvke"))
                return false;
            room->broadcastSkillInvoke(objectName());

            QString slasher = player->objectName();

            const Card *first_jink = NULL, *second_jink = NULL;
            first_jink = room->askForCard(effect.to, "jink", "@wushuang-jink-1:" + slasher, QVariant(), CardUsed);
            if(first_jink)
                second_jink = room->askForCard(effect.to, "jink", "@wushuang-jink-2:" + slasher, QVariant(), CardUsed);

            Card *jink = NULL;
            if(first_jink && second_jink){
                jink = new DummyCard;
                jink->addSubcard(first_jink);
                jink->addSubcard(second_jink);
            }

            room->slashResult(effect, jink);

            return true;
        }
        return false;
    }
};

class Lijian: public OneCardViewAsSkill{
public:
    Lijian():OneCardViewAsSkill("lijian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LijianCard");
    }

    virtual bool viewFilter(const Card *) const{
        return true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        LijianCard *lijian_card = new LijianCard;
        lijian_card->addSubcard(originalCard->getId());

        return lijian_card;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 0;
    }
};

class Biyue: public PhaseChangeSkill{
public:
    Biyue():PhaseChangeSkill("biyue"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *diaochan) const{
        if(diaochan->getPhase() == Player::Finish){
            Room *room = diaochan->getRoom();
            if(room->askForSkillInvoke(diaochan, objectName())){
                room->broadcastSkillInvoke(objectName());
                diaochan->drawCards(1);
            }
        }

        return false;
    }
};

class Qingnang: public OneCardViewAsSkill{
public:
    Qingnang():OneCardViewAsSkill("qingnang"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QingnangCard");
    }

    virtual bool viewFilter(const Card* to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        QingnangCard *qingnang_card = new QingnangCard;
        qingnang_card->addSubcard(originalCard->getId());

        return qingnang_card;
    }
};

class Jijiu: public OneCardViewAsSkill{
public:
    Jijiu():OneCardViewAsSkill("jijiu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern.contains("peach") && player->getPhase() == Player::NotActive;
    }

    virtual bool viewFilter(const Card* to_select) const{
        return to_select->isRed();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Peach *peach = new Peach(originalCard->getSuit(), originalCard->getNumber());
        peach->addSubcard(originalCard->getId());
        peach->setSkillName(objectName());
        return peach;
    }
};

class Qianxun: public ProhibitSkill{
public:
    Qianxun():ProhibitSkill("qianxun"){

    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->isKindOf("Snatch") || card->isKindOf("Indulgence");
    }
};

class Mashu: public DistanceSkill{
public:
    Mashu():DistanceSkill("mashu")
    {
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(from->hasSkill(objectName()))
            return -1;
        else
            return 0;
    }
};

void StandardPackage::addGenerals(){
    General *caocao, *zhangliao, *guojia, *xiahoudun, *simayi, *xuchu, *zhenji;

    caocao = new General(this, "caocao$", "wei");
    caocao->addSkill(new Jianxiong);
    caocao->addSkill(new Hujia);

    simayi = new General(this, "simayi", "wei", 3);
    simayi->addSkill(new Fankui);
    simayi->addSkill(new Guicai);

    xiahoudun = new General(this, "xiahoudun", "wei");
    xiahoudun->addSkill(new Ganglie);

    zhangliao = new General(this, "zhangliao", "wei");
    zhangliao->addSkill(new Tuxi);

    xuchu = new General(this, "xuchu", "wei");
    xuchu->addSkill(new Luoyi);
    xuchu->addSkill(new LuoyiBuff);
    related_skills.insertMulti("luoyi", "#luoyi");

    guojia = new General(this, "guojia", "wei", 3);
    guojia->addSkill(new Tiandu);
    guojia->addSkill(new Yiji);

    zhenji = new General(this, "zhenji", "wei", 3, false);
    zhenji->addSkill(new Luoshen);
    zhenji->addSkill(new Qingguo);

    General *liubei, *guanyu, *zhangfei, *zhaoyun, *machao, *zhugeliang, *huangyueying;
    liubei = new General(this, "liubei$", "shu");
    liubei->addSkill(new Rende);
    liubei->addSkill(new Jijiang);

    guanyu = new General(this, "guanyu", "shu");
    guanyu->addSkill(new Wusheng);

    zhangfei = new General(this, "zhangfei", "shu");
    zhangfei->addSkill(new Skill("paoxiao", Skill::Compulsory));

    zhugeliang = new General(this, "zhugeliang", "shu", 3);
    zhugeliang->addSkill(new Guanxing);
    zhugeliang->addSkill(new Kongcheng);
    zhugeliang->addSkill(new KongchengEffect);
    related_skills.insertMulti("kongcheng", "#kongcheng-effect");

    zhaoyun = new General(this, "zhaoyun", "shu");
    zhaoyun->addSkill(new Longdan);

    machao = new General(this, "machao", "shu");
    machao->addSkill(new Tieji);
    machao->addSkill(new Mashu);
    machao->addSkill(new SPConvertSkill("fanqun", "machao", "sp_machao"));

    huangyueying = new General(this, "huangyueying", "shu", 3, false);
    huangyueying->addSkill(new Jizhi);
    huangyueying->addSkill(new Skill("qicai", Skill::Compulsory));

    General *sunquan, *zhouyu, *lvmeng, *luxun, *ganning, *huanggai, *daqiao, *sunshangxiang;
    sunquan = new General(this, "sunquan$", "wu");
    sunquan->addSkill(new Zhiheng);
    sunquan->addSkill(new Jiuyuan);

    ganning = new General(this, "ganning", "wu");
    ganning->addSkill(new Qixi);

    lvmeng = new General(this, "lvmeng", "wu");
    lvmeng->addSkill(new Keji);

    huanggai = new General(this, "huanggai", "wu");
    huanggai->addSkill(new Kurou);

    zhouyu = new General(this, "zhouyu", "wu", 3);
    zhouyu->addSkill(new Yingzi);
    zhouyu->addSkill(new Fanjian);

    daqiao = new General(this, "daqiao", "wu", 3, false);
    daqiao->addSkill(new Guose);
    daqiao->addSkill(new Liuli);

    luxun = new General(this, "luxun", "wu", 3);
    luxun->addSkill(new Qianxun);
    luxun->addSkill(new Lianying);

    sunshangxiang = new General(this, "sunshangxiang", "wu", 3, false);
    sunshangxiang->addSkill(new SPConvertSkill("chujia", "sunshangxiang", "sp_sunshangxiang"));
    sunshangxiang->addSkill(new Jieyin);
    sunshangxiang->addSkill(new Xiaoji);

    General *lvbu, *huatuo, *diaochan;

    huatuo = new General(this, "huatuo", "qun", 3);
    huatuo->addSkill(new Qingnang);
    huatuo->addSkill(new Jijiu);

    lvbu = new General(this, "lvbu", "qun");
    lvbu->addSkill(new Wushuang);

    diaochan = new General(this, "diaochan", "qun", 3, false);
    diaochan->addSkill(new Lijian);
    diaochan->addSkill(new Biyue);
    diaochan->addSkill(new SPConvertSkill("tuoqiao", "diaochan", "sp_diaochan"));

    // for skill cards
    addMetaObject<ZhihengCard>();
    addMetaObject<RendeCard>();
    addMetaObject<TuxiCard>();
    addMetaObject<JieyinCard>();
    addMetaObject<KurouCard>();
    addMetaObject<LijianCard>();
    addMetaObject<FanjianCard>();
    addMetaObject<GuicaiCard>();
    addMetaObject<QingnangCard>();
    addMetaObject<LiuliCard>();
    addMetaObject<JijiangCard>();
}

class Zhiba: public Zhiheng{
public:
    Zhiba(){
        setObjectName("zhiba");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->usedTimes("ZhihengCard") < (player->getLostHp() + 1);
    }
};

class SuperGuanxing: public Guanxing{
public:
    SuperGuanxing():Guanxing(){
        setObjectName("super_guanxing");
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const{
        if(zhuge->getPhase() == Player::Start &&
           zhuge->askForSkillInvoke(objectName()))
        {
            Room *room = zhuge->getRoom();
            room->broadcastSkillInvoke("guanxing");

            room->askForGuanxing(zhuge, room->getNCards(5, false), false);
        }

        return false;
    }
};

class Longnu: public TriggerSkill{
public:
    Longnu():TriggerSkill("longnu"){
        events << CardsMoveOneTime << EventPhaseChanging;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *sp_shenzhaoyun, QVariant &data) const{
        if(triggerEvent == EventPhaseChanging)
        {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if(change.to == Player::Start)
                sp_shenzhaoyun->skip(Player::Draw);
        }
        else
        {
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            if(move->from != sp_shenzhaoyun)
                return false;
            if(sp_shenzhaoyun->getHandcardNum()<4 && sp_shenzhaoyun->getPhase() != Player::Discard && room->askForSkillInvoke(sp_shenzhaoyun,objectName()))
                sp_shenzhaoyun->drawCards(4-sp_shenzhaoyun->getHandcardNum());
        }
        return false;
    }
};

YihunCard::YihunCard(){
    once = true;
}

bool YihunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;
    if(targets.isEmpty())
        return true;
    else if(targets.length() == 1)
        return to_select->getHp() != targets.first()->getHp();
    else
        return false;
}

bool YihunCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void YihunCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> selecteds = targets;
    ServerPlayer *from = selecteds.first()->getHp() < selecteds.last()->getHp() ? selecteds.takeFirst() : selecteds.takeLast();
    ServerPlayer *to = selecteds.takeFirst();
    int maxhp1 = from->getMaxHp();
    int maxhp2 = to->getMaxHp();
    int hp1 = from->getHp();
    int hp2 = to->getHp();
    room->setPlayerProperty(from, "maxhp", maxhp2);
    room->setPlayerProperty(to, "maxhp", maxhp1);
    room->setPlayerProperty(from, "hp", hp2);
    room->setPlayerProperty(to, "hp", hp1);
    RecoverStruct rev;
    rev.recover = 1;
    if ((hp2-hp1) > 1)
        room->recover(source, rev);
}

class Yihun: public ZeroCardViewAsSkill{
public:
    Yihun():ZeroCardViewAsSkill("yihun"){
    }

    virtual const Card *viewAs() const{
        return new YihunCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YihunCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }
};

class Chushi: public TriggerSkill{
public:
    Chushi():TriggerSkill("chushi"){
        events << DamageForseen;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        LogMessage log;
        log.type = "#Chushi";
        log.from = player;
        log.arg = QString::number(damage.damage);
        log.arg2 = objectName();
        room->sendLog(log);
        room->loseHp(player);
        return true;
    }
};

class Shihun: public TriggerSkill{
public:
    Shihun():TriggerSkill("shihun"){
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && TriggerSkill::triggerable(target) && target->faceUp();
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *nanhua = room->findPlayerBySkillName(objectName());
        DyingStruct dying_data = data.value<DyingStruct>();
        if(dying_data.who != nanhua)
            return false;
        if(!nanhua || !room->askForSkillInvoke(nanhua, objectName(), data))
            return false;
        ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
        int hp = target->getHp();
        RecoverStruct rev;
        rev.recover = qMin(hp,player->getMaxHp()-player->getHp());
        room->recover(player, rev);
        player->turnOver();
        room->loseHp(target,rev.recover);
        return false;
    }
};

TestPackage::TestPackage()
    :Package("test")
{
    // for test only
    General *zhiba_sunquan = new General(this, "zhibasunquan$", "wu", 4, true, true);
    zhiba_sunquan->addSkill(new Zhiba);
    zhiba_sunquan->addSkill("jiuyuan");

    General *wuxing_zhuge = new General(this, "wuxingzhuge", "shu", 3, true, true);
    wuxing_zhuge->addSkill(new SuperGuanxing);
    wuxing_zhuge->addSkill("kongcheng");
    wuxing_zhuge->addSkill("#kongcheng-effect");

    General *sp_shenzhaoyun = new General(this, "sp_shenzhaoyun", "god", 1, true, true);
    sp_shenzhaoyun->addSkill(new Longnu);
    sp_shenzhaoyun->addSkill("longhun");

    General *nanhua = new General(this, "nanhua", "god", 3, true, true);
    nanhua->addSkill(new Yihun);
    nanhua->addSkill(new Chushi);
    nanhua->addSkill(new Shihun);


    new General(this, "sujiang", "god", 5, true, true);
    new General(this, "sujiangf", "god", 5, false, true);

    new General(this, "anjiang", "god", 4,true, true, true);
    addMetaObject<YihunCard>();
}

ADD_PACKAGE(Test)
