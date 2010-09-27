#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "serverplayer.h"
#include "room.h"

class Jianxiong:public MasochismSkill{
public:
    Jianxiong():MasochismSkill("jianxiong"){
    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{
        Room *room = target->getRoom();
        const Card *card = damage.card;
        if(!room->obtainable(card, target))
            return;

        if(room->askForSkillInvoke(target, "jianxiong")){
            room->playSkillEffect(objectName());
            room->obtainCard(target, card);            
        }
    }
};

class Hujia:public TriggerSkill{
public:
    Hujia():TriggerSkill("hujia$"){
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName()) && target->isAlive();
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *caocao, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "jink")
            return false;

        Room *room = caocao->getRoom();

        if(!room->askForSkillInvoke(caocao, objectName()))
            return false;

        room->playSkillEffect(objectName());

        QList<ServerPlayer *> lieges = room->getLieges(caocao);
        foreach(ServerPlayer *liege, lieges){
            QString result = room->askForChoice(liege, objectName(), "accept+ignore");
            if(result == "ignore")
                continue;

            const Card *jink = room->askForCard(liege, "jink", "hujia-jink");
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
    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@tuxi";
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

        events << JudgeOnEffect;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *guojia, QVariant &data) const{
        CardStar card = data.value<CardStar>();
        guojia->obtainCard(card);
        guojia->getRoom()->playSkillEffect(objectName());

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

        room->playSkillEffect(objectName());
        int n = damage.damage * 2;
        guojia->drawCards(n);
        QList<int> yiji_cards = guojia->handCards().mid(guojia->getHandcardNum() - n);

        while(room->askForYiji(guojia, yiji_cards))
            ; // empty loop
    }
};

class Ganglie:public MasochismSkill{
public:
    Ganglie():MasochismSkill("ganglie"){

    }

    virtual void onDamaged(ServerPlayer *xiahou, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = xiahou->getRoom();
        if(from && room->askForSkillInvoke(xiahou, "ganglie")){
            room->playSkillEffect(objectName());

            const Card *card = room->getJudgeCard(xiahou);
            if(card->getSuit() != Card::Heart){
                if(!room->askForDiscard(from, 2, true)){
                    DamageStruct damage;
                    damage.card = NULL;
                    damage.damage = 1;
                    damage.from = xiahou;
                    damage.to = from;

                    room->damage(damage);
                }
            }
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
        if(from && !from->isNude() && room->askForSkillInvoke(simayi, "fankui")){
            int card_id = room->askForCardChosen(simayi, from, "he", "fankui");
            if(room->getCardPlace(card_id) == Player::Hand)
                room->moveCardTo(Sanguosha->getCard(card_id), simayi, Player::Hand, false);
            else
                room->obtainCard(simayi, card_id);
            room->playSkillEffect(objectName());
        }
    }
};

class Guicai:public ViewAsSkill{
public:
    Guicai():ViewAsSkill("guicai"){
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@guicai";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        Card *card = new GuicaiCard;
        card->addSubcard(cards.first()->getCard()->getId());

        return card;
    }
};

class Luoyi:public TriggerSkill{
public:
    Luoyi():TriggerSkill("luoyi"){
        events << PhaseChange << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *xuchu, QVariant &data) const{
        Room *room = xuchu->getRoom();

        if(event == PhaseChange){
            switch(xuchu->getPhase()){
            case Player::Draw:{
                    if(room->askForSkillInvoke(xuchu, "luoyi")){
                        xuchu->drawCards(1);
                        room->playSkillEffect(objectName());

                        setFlag(xuchu);
                        return true;
                    }
                    break;
                }
            case Player::Finish:{
                    if(xuchu->hasFlag("luoyi"))
                        unsetFlag(xuchu);
                    break;
                }
            default:
                ;
            }
        }else if(event == Predamage){
            if(xuchu->hasFlag("luoyi")){
                DamageStruct damage = data.value<DamageStruct>();

                const Card *reason = damage.card;
                if(reason->inherits("Slash") || reason->inherits("Duel")){
                    damage.damage ++;
                    data = QVariant::fromValue(damage);
                }
            }

            return false;
        }

        return false;
    }
};

class Luoshen:public PhaseChangeSkill{
public:
    Luoshen():PhaseChangeSkill("luoshen"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Start){
            Room *room = target->getRoom();
            forever{
                if(room->askForSkillInvoke(target, objectName())){
                    room->playSkillEffect(objectName());

                    const Card *card = room->getJudgeCard(target);
                    if(card->isBlack())
                        target->obtainCard(card);
                    else
                        break;
                }else
                    break;
            }
        }

        return false;
    }
};

class Qingguo:public ViewAsSkill{
public:
    Qingguo():ViewAsSkill("qingguo"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && to_select->getCard()->isBlack() && (!to_select->isEquipped());
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;
        else{
            const Card *card = cards.first()->getCard();
            Jink *jink = new Jink(card->getSuit(), card->getNumber());
            jink->setSkillName(objectName());
            jink->addSubcard(card->getId());
            return jink;
        }
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "jink";
    }
};

class RendeViewAsSkill:public ViewAsSkill{
public:
    RendeViewAsSkill():ViewAsSkill("rende"){
    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        SkillCard *rende_card = new RendeCard;
        rende_card->addSubcards(cards);
        return rende_card;
    }
};

class Rende: public PhaseChangeSkill{
public:
    Rende():PhaseChangeSkill("rende"){
        view_as_skill = new RendeViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Start)
            target->setMark("rende", 0);

        return false;
    }
};

class JijiangViewAsSkill:public ZeroCardViewAsSkill{
public:
    JijiangViewAsSkill():ZeroCardViewAsSkill("jijiang$"){

    }

    virtual bool isEnabledAtPlay() const{
        return Self->getRole() == "lord" && Slash::IsAvailable();
    }

    virtual const Card *viewAs() const{
        return new JijiangCard;
    }
};

class Jijiang: public TriggerSkill{
public:
    Jijiang():TriggerSkill("jijiang$"){
        events << CardAsked;

        view_as_skill = new JijiangViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->isLord() && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *liubei, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "slash")
            return false;

        Room *room = liubei->getRoom();
        if(!room->askForSkillInvoke(liubei, objectName()))
            return false;

        room->playSkillEffect(objectName());

        QList<ServerPlayer *> lieges = room->getLieges(liubei);
        foreach(ServerPlayer *liege, lieges){
            const Card *slash = room->askForCard(liege, "slash", "jijiang-slash");
            if(slash){
                room->provide(slash);
                return true;
            }
        }

        return false;
    }
};

class Wusheng:public ViewAsSkill{
public:
    Wusheng()
        :ViewAsSkill("wusheng")
    {
    }

    virtual bool isEnabledAtPlay() const{
        return Slash::IsAvailable();
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "slash";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && to_select->getCard()->isRed();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;
        else{
            const Card *card = cards.first()->getCard();
            Card *slash = new Slash(card->getSuit(), card->getNumber());
            slash->addSubcard(card->getId());
            slash->setSkillName(objectName());
            return slash;
        }
    }
};

// should be ViewAsSkill
class Longdan:public ViewAsSkill{
public:
    Longdan():ViewAsSkill("longdan"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(!selected.isEmpty())
            return false;

        const Card *card = to_select->getCard();       

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                // jink as slash
                return card->inherits("Jink");
            }

        case Client::Responsing:{
                QString pattern = ClientInstance->card_pattern;
                if(pattern == "slash")
                    return card->inherits("Jink");
                else if(pattern == "jink")
                    return card->inherits("Slash");
            }

        default:
            return false;
        }
    }

    virtual bool isEnabledAtPlay() const{
        return Slash::IsAvailable();
    }

    virtual bool isEnabledAtResponse() const{
        QString pattern = ClientInstance->card_pattern;
        return pattern == "jink" || pattern == "slash";
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        const Card *card = cards.first()->getCard();
        if(card->inherits("Slash")){
            Jink *jink = new Jink(card->getSuit(), card->getNumber());
            jink->addSubcard(card->getId());
            jink->setSkillName(objectName());
            return jink;
        }else if(card->inherits("Jink")){
            Slash *slash = new Slash(card->getSuit(), card->getNumber());
            slash->addSubcard(card->getId());
            slash->setSkillName(objectName());
            return slash;
        }else
            return NULL;
    }
};

class Tieji:public SlashBuffSkill{
public:
    Tieji():SlashBuffSkill("tieji"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *machao = effect.from;

        Room *room = machao->getRoom();
        if(room->askForSkillInvoke(effect.from, "tieji")){
            room->playSkillEffect(objectName());

            const Card *card = room->getJudgeCard(machao);
            if(card->isRed()){
                SlashResultStruct result;
                result.fill(effect, true);
                room->slashResult(result);
                return true;
            }
        }

        return false;
    }
};

Mashu::Mashu()
    :GameStartSkill("mashu")
{
    frequency = Compulsory;
}

void Mashu::onGameStart(ServerPlayer *player) const
{
    player->getRoom()->setPlayerCorrect(player, "M");
}

class Guanxing:public PhaseChangeSkill{
public:
    Guanxing():PhaseChangeSkill("guanxing"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const{
        if(zhuge->getPhase() == Player::Start){
            Room *room = zhuge->getRoom();
            if(room->askForSkillInvoke(zhuge, objectName())){
                room->playSkillEffect(objectName());
                room->doGuanxing(zhuge);
            }
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

    virtual bool trigger(TriggerEvent event, ServerPlayer *yueying, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;

            // special case
            if(use.card->inherits("IronChain") && use.to.isEmpty())
                return false;

        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->inherits("TrickCard") && !card->inherits("DelayedTrick")){
            Room *room = yueying->getRoom();
            if(room->askForSkillInvoke(yueying, objectName())){
                room->playSkillEffect(objectName());
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

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        ZhihengCard *zhiheng_card = new ZhihengCard;
        zhiheng_card->addSubcards(cards);

        return zhiheng_card;
    }

    virtual bool isEnabledAtPlay() const{
        return ! ClientInstance->turn_tag.value("zhiheng_used", false).toBool();
    }
};

class Yingzi:public PhaseChangeSkill{
public:
    Yingzi():PhaseChangeSkill("yingzi"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *zhouyu) const{
        if(zhouyu->getPhase() == Player::Draw){
            Room *room = zhouyu->getRoom();
            if(room->askForSkillInvoke(zhouyu, objectName())){
                zhouyu->drawCards(3);
                room->playSkillEffect(objectName());

                return true;
            }
        }

        return false;
    }
};

class Fanjian:public ZeroCardViewAsSkill{
public:
    Fanjian():ZeroCardViewAsSkill("fanjian"){

    }

    virtual bool isEnabledAtPlay() const{
        return !Self->isKongcheng() && !ClientInstance->turn_tag.value("fanjian_used", false).toBool();
    }

    virtual const Card *viewAs() const{
        return new FanjianCard;
    }
};

class Keji: public TriggerSkill{
public:
    Keji():TriggerSkill("keji"){
        events << PhaseChange << CardUsed << CardResponsed;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *lumeng, QVariant &data) const{
        if(event == PhaseChange){
            if(lumeng->getPhase() == Player::Start)
                lumeng->setMark("slash_count", 0);
            else if(lumeng->getPhase() == Player::Discard){
                if(lumeng->getMark("slash_count") == 0 && lumeng->getRoom()->askForSkillInvoke(lumeng, objectName()))
                    return true;
            }
        }else if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->inherits("Slash"))
                lumeng->addMark("slash_count");
        }else if(event == CardResponsed){
            CardStar card_star = data.value<CardStar>();
            if(card_star->inherits("Slash"))
                lumeng->addMark("slash_count");
        }

        return false;
    }
};


class Lianying: public TriggerSkill{
public:
    Lianying():TriggerSkill("lianying"){
        events << CardLost;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *luxun, QVariant &) const{
        if(luxun->isKongcheng()){
            Room *room = luxun->getRoom();
            if(room->askForSkillInvoke(luxun, objectName())){
                room->playSkillEffect(objectName());

                luxun->drawCards(1);
            }
        }

        return false;
    }
};

class Qixi: public ViewAsSkill{
public:
    Qixi():ViewAsSkill("qixi"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(!selected.isEmpty())
            return false;

        return to_select->getCard()->isBlack();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;
        else{
            const Card *first = cards.first()->getCard();
            Dismantlement *dismantlement = new Dismantlement(first->getSuit(), first->getNumber());
            dismantlement->addSubcard(first->getId());
            dismantlement->setSkillName(objectName());
            return dismantlement;
        }
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

class Guose: public ViewAsSkill{
public:
    Guose():ViewAsSkill("guose"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(!selected.isEmpty())
            return false;

        return to_select->getCard()->getSuit() == Card::Diamond;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        const Card *first = cards.first()->getCard();
        Indulgence *indulgence = new Indulgence(first->getSuit(), first->getNumber());
        indulgence->addSubcard(first->getId());
        indulgence->setSkillName(objectName());
        return indulgence;
    }
};

class LiuliViewAsSkill: public ViewAsSkill{
public:
    LiuliViewAsSkill():ViewAsSkill("liuli"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty();
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern.startsWith("@@liuli");;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;        

        if(!ClientInstance->card_pattern.startsWith("@@liuli-"))
            return NULL;

        QString slash_source = ClientInstance->card_pattern;
        slash_source.remove("@@liuli-");

        LiuliCard *card = new LiuliCard;
        card->setSlashSource(slash_source);
        card->addSubcards(cards);

        return card;
    }
};

class Liuli: public TriggerSkill{
public:
    Liuli():TriggerSkill("liuli"){
        view_as_skill = new LiuliViewAsSkill;

        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *daqiao, QVariant &data) const{
        Room *room = daqiao->getRoom();

        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("Slash") && !daqiao->isNude() && room->alivePlayerCount() > 2){
            QList<ServerPlayer *> players = room->getOtherPlayers(daqiao);
            players.removeOne(effect.from);

            bool can_invoke = false;
            foreach(ServerPlayer *player, players){
                if(daqiao->inMyAttackRange(player)){
                    can_invoke = true;
                    break;
                }
            }

            if(can_invoke){
                QString prompt = "@liuli-card:" + effect.from->getGeneralName();
                if(room->askForUseCard(daqiao, "@@liuli-" + effect.from->objectName(), prompt)){
                    foreach(ServerPlayer *player, players){
                        if(player->hasFlag("liuli_target")){
                            room->setPlayerFlag(player, "-liuli_target");
                            effect.to = player;
                            room->cardEffect(effect);
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

    virtual bool isEnabledAtPlay() const{
        return !ClientInstance->turn_tag.value("jieyin_used", false).toBool();
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() > 2)
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
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
        events << CardLost;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *sunshangxiang, QVariant &data) const{
        if(data.canConvert<CardMoveStruct>()){
            CardMoveStruct move = data.value<CardMoveStruct>();
            if(move.from_place == Player::Equip){
                Room *room = sunshangxiang->getRoom();
                if(room->askForSkillInvoke(sunshangxiang, objectName())){
                    room->playSkillEffect(objectName());
                    sunshangxiang->drawCards(2);
                }
            }
        }

        return false;
    }
};

class Wushuang: public TriggerSkill{
public:
    Wushuang():TriggerSkill("wushuang"){
        events << SlashProceed;

        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *lubu, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = lubu->getRoom();
        QString slasher = lubu->getGeneralName();
        bool jinked = false;
        room->playSkillEffect(objectName());
        const Card *first_jink = room->askForCard(effect.to, "jink", "@wushuang-jink-1:" + slasher);
        if(first_jink && room->askForCard(effect.to, "jink", "@wushuang-jink-2:" + slasher))
            jinked = true;

        SlashResultStruct result;
        result.fill(effect, !jinked);
        room->slashResult(result);

        return true;
    }
};

class Lijian: public ViewAsSkill{
public:
    Lijian():ViewAsSkill("lijian"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        LijianCard *lijian_card = new LijianCard;
        lijian_card->addSubcards(cards);

        return lijian_card;
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
                room->playSkillEffect(objectName());
                diaochan->drawCards(1);
            }
        }

        return false;
    }
};

class Qingnang: public ViewAsSkill{
public:
    Qingnang():ViewAsSkill("qingnang"){

    }

    virtual bool isEnabledAtPlay() const{
        return !ClientInstance->turn_tag.value("qingnang_used", false).toBool();
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        QingnangCard *qingnang_card = new QingnangCard;
        qingnang_card->addSubcard(cards.first()->getCard()->getId());

        return qingnang_card;
    }
};

class Jijiu: public ViewAsSkill{
public:
    Jijiu():ViewAsSkill("jijiu"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern.contains("peach") && Self->getPhase() == Player::NotActive;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && to_select->getCard()->isRed();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 1){
            const Card *first = cards.first()->getCard();
            Peach *peach = new Peach(first->getSuit(), first->getNumber());
            peach->addSubcard(first->getId());
            peach->setSkillName(objectName());
            return peach;
        }

        return NULL;
    }
};

void StandardPackage::addGenerals(){
    General *caocao, *zhangliao, *guojia, *xiahoudun, *simayi, *xuchu, *zhenji;

    caocao = new General(this, "caocao$", "wei");
    caocao->addSkill(new Jianxiong);
    caocao->addSkill(new Hujia);

    zhangliao = new General(this, "zhangliao", "wei");
    zhangliao->addSkill(new Tuxi);

    guojia = new General(this, "guojia", "wei", 3);
    guojia->addSkill(new Tiandu);
    guojia->addSkill(new Yiji);

    xiahoudun = new General(this, "xiahoudun", "wei");
    xiahoudun->addSkill(new Ganglie);

    simayi = new General(this, "simayi", "wei", 3);
    simayi->addSkill(new Fankui);
    simayi->addSkill(new Guicai);

    xuchu = new General(this, "xuchu", "wei");
    xuchu->addSkill(new Luoyi);

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
    zhangfei->addSkill(new Skill("paoxiao"));

    zhaoyun = new General(this, "zhaoyun", "shu");
    zhaoyun->addSkill(new Longdan);

    machao = new General(this, "machao", "shu");
    machao->addSkill(new Tieji);
    machao->addSkill(new Mashu);

    zhugeliang = new General(this, "zhugeliang", "shu", 3);
    zhugeliang->addSkill(new Guanxing);
    zhugeliang->addSkill(new Skill("kongcheng", Skill::Compulsory));

    huangyueying = new General(this, "huangyueying", "shu", 3, false);
    huangyueying->addSkill(new Jizhi);
    huangyueying->addSkill(new Skill("qicai", Skill::Compulsory));

    General *sunquan, *zhouyu, *lumeng, *luxun, *ganning, *huanggai, *daqiao, *sunshangxiang;
    sunquan = new General(this, "sunquan$", "wu");
    sunquan->addSkill(new Zhiheng);
    sunquan->addSkill(new Skill("jiuyuan$", Skill::Compulsory));

    zhouyu = new General(this, "zhouyu", "wu", 3);
    zhouyu->addSkill(new Yingzi);
    zhouyu->addSkill(new Fanjian);

    lumeng = new General(this, "lumeng", "wu");
    lumeng->addSkill(new Keji);

    luxun = new General(this, "luxun", "wu", 3);
    luxun->addSkill(new Skill("qianxun", Skill::Compulsory));
    luxun->addSkill(new Lianying);

    ganning = new General(this, "ganning", "wu");
    ganning->addSkill(new Qixi);

    huanggai = new General(this, "huanggai", "wu");
    huanggai->addSkill(new Kurou);

    daqiao = new General(this, "daqiao", "wu", 3, false);
    daqiao->addSkill(new Guose);
    daqiao->addSkill(new Liuli);

    sunshangxiang = new General(this, "sunshangxiang", "wu", 3, false);
    sunshangxiang->addSkill(new Jieyin);
    sunshangxiang->addSkill(new Xiaoji);

    General *lubu, *huatuo, *diaochan;

    lubu = new General(this, "lubu", "qun");
    lubu->addSkill(new Wushuang);

    huatuo = new General(this, "huatuo", "qun", 3);
    huatuo->addSkill(new Qingnang);
    huatuo->addSkill(new Jijiu);

    diaochan = new General(this, "diaochan", "qun", 3, false);
    diaochan->addSkill(new Lijian);
    diaochan->addSkill(new Biyue);

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

#ifndef QT_NO_DEBUG

    addMetaObject<CheatCard>();

#endif
    
    // for translation
    t["wei"] = tr("wei");
    t["shu"] = tr("shu");
    t["wu"] = tr("wu");
    t["qun"] = tr("qun");

    t["caocao"] = tr("caocao");
    t["zhangliao"] = tr("zhangliao");
    t["guojia"] = tr("guojia");
    t["xiahoudun"] = tr("xiahoudun");
    t["simayi"] = tr("simayi");
    t["xuchu"] = tr("xuchu");
    t["zhenji"] = tr("zhenji");

    t["liubei"] = tr("liubei");
    t["guanyu"] = tr("guanyu");
    t["zhangfei"] = tr("zhangfei");
    t["zhaoyun"] = tr("zhaoyun");
    t["machao"] = tr("machao");
    t["zhugeliang"] = tr("zhugeliang");
    t["huangyueying"] = tr("huangyueying");

    t["sunquan"] = tr("sunquan");
    t["zhouyu"] = tr("zhouyu");
    t["lumeng"] = tr("lumeng");
    t["luxun"] = tr("luxun");
    t["ganning"] = tr("ganning");
    t["huanggai"] = tr("huanggai");
    t["daqiao"] = tr("daqiao");
    t["sunshangxiang"] = tr("sunshangxiang");

    t["lubu"] = tr("lubu");
    t["huatuo"] = tr("huatuo");
    t["diaochan"] = tr("diaochan");

    t["jianxiong"] = tr("jianxiong");
    t["hujia"] = tr("hujia");
    t["tuxi"] = tr("tuxi");
    t["tiandu"] = tr("tiandu");
    t["yiji"] = tr("yiji");
    t["ganglie"] = tr("ganglie");
    t["fankui"] = tr("fankui");
    t["guicai"] = tr("guicai");
    t["luoyi"] = tr("luoyi");
    t["luoshen"] = tr("luoshen");
    t["qingguo"] = tr("qingguo");

    t["rende"] = tr("rende");
    t["jijiang"] = tr("jijiang");
    t["wusheng"] = tr("wusheng");
    t["paoxiao"] = tr("paoxiao");
    t["longdan"] = tr("longdan");
    t["tieji"] = tr("tieji");
    t["mashu"] = tr("mashu");
    t["guanxing"] = tr("guanxing");
    t["kongcheng"] = tr("kongcheng");
    t["jizhi"] = tr("jizhi");
    t["qicai"] = tr("qicai");

    t["zhiheng"] = tr("zhiheng");
    t["jiuyuan"] = tr("jiuyuan");
    t["yingzi"] = tr("yingzi");
    t["fanjian"] = tr("fanjian");
    t["keji"] = tr("keji");
    t["qianxun"] = tr("qianxun");
    t["lianying"] = tr("lianying");
    t["qixi"] = tr("qixi");
    t["kurou"] = tr("kurou");
    t["guose"] = tr("guose");
    t["liuli"] = tr("liuli");
    t["jieyin"] = tr("jieyin");
    t["xiaoji"] = tr("xiaoji");

    t["wushuang"] = tr("wushuang");
    t["qingnang"] = tr("qingnang");
    t["jijiu"] = tr("jijiu");
    t["lijian"] = tr("lijian");
    t["biyue"] = tr("biyue");

    t[":jianxiong"] = tr(":jianxiong");
    t[":hujia"] = tr(":hujia");
    t[":tuxi"] = tr(":tuxi");
    t[":tiandu"] = tr(":tiandu");
    t[":yiji"] = tr(":yiji");
    t[":ganglie"] = tr(":ganglie");
    t[":fankui"] = tr(":fankui");
    t[":guicai"] = tr(":guicai");
    t[":luoyi"] = tr(":luoyi");
    t[":luoshen"] = tr(":luoshen");
    t[":qingguo"] = tr(":qingguo");

    t[":rende"] = tr(":rende");
    t[":jijiang"] = tr(":jijiang");
    t[":wusheng"] = tr(":wusheng");
    t[":paoxiao"] = tr(":paoxiao");
    t[":longdan"] = tr(":longdan");
    t[":tieji"] = tr(":tieji");
    t[":mashu"] = tr(":mashu");
    t[":guanxing"] = tr(":guanxing");
    t[":kongcheng"] = tr(":kongcheng");
    t[":jizhi"] = tr(":jizhi");
    t[":qicai"] = tr(":qicai");

    t[":zhiheng"] = tr(":zhiheng");
    t[":jiuyuan"] = tr(":jiuyuan");
    t[":yingzi"] = tr(":yingzi");
    t[":fanjian"] = tr(":fanjian");
    t[":keji"] = tr(":keji");
    t[":qianxun"] = tr(":qianxun");
    t[":lianying"] = tr(":lianying");
    t[":qixi"] = tr(":qixi");
    t[":kurou"] = tr(":kurou");
    t[":guose"] = tr(":guose");
    t[":liuli"] = tr(":liuli");
    t[":jieyin"] = tr(":jieyin");
    t[":xiaoji"] = tr(":xiaoji");

    t[":wushuang"] = tr(":wushuang");
    t[":qingnang"] = tr(":qingnang");
    t[":jijiu"] = tr(":jijiu");
    t[":lijian"] = tr(":lijian");
    t[":biyue"] = tr(":biyue");

    // passive skill description
    t["yingzi:yes"] = tr("yingzi:yes");
    t["jianxiong:yes"] = tr("jianxiong:yes");
    t["fankui:yes"] = tr("fankui:yes");
    t["biyue:yes"] = tr("biyue:yes");
    t["luoyi:yes"] = tr("luoyi:yes");
    t["tieji:yes"] = tr("tieji:yes");
    t["ganglie:yes"] = tr("ganglie:yes");
    t["hujia:yes"] = tr("hujia:yes");
    t[":hujia:"] = tr(":hujia:");
    t["hujia:accept"] = tr("hujia:accept");
    t["hujia:ignore"] = tr("hujia:ignore");
    t[":jijiang:"] = tr(":jijiang:");
    t["jijiang:accept"] = tr("jijiang:accept");
    t["jijiang:ignore"] = tr("jijiang:ignore");

    t["@jijiang-slash"] = tr("@jijiang-slash");
    t["@wushuang-slash-1"] = tr("@wushuang-slash-1");
    t["@wushuang-slash-2"] = tr("@wushuang-slash-2");
    t["@wushuang-jink-1"] = tr("@wushuang-jink-1");
    t["@wushuang-jink-2"] = tr("@wushuang-jink-2");
    t["@guicai-card"] = tr("@guicai-card");
    t["@guidao-card"] = tr("@guidao-card");
    t["@liuli-card"] = tr("@liuli-card");
    t["@tuxi-card"] = tr("@tuxi-card");
    t["@leiji"] = tr("@leiji");

    t["#GuanxingResult"] = tr("#GuanxingResult");
}
