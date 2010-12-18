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

    virtual void onDamaged(ServerPlayer *caocao, const DamageStruct &damage) const{
        Room *room = caocao->getRoom();
        const Card *card = damage.card;
        if(!room->obtainable(card, caocao))
            return;

        QVariant data = QVariant::fromValue(card);
        if(room->askForSkillInvoke(caocao, "jianxiong", data)){
            room->playSkillEffect(objectName());
            caocao->obtainCard(card);
        }
    }
};

class Hujia:public TriggerSkill{
public:
    Hujia():TriggerSkill("hujia$"){
        events << CardAsked;
        default_choice = "ignore";
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *caocao, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "jink")
            return false;

        Room *room = caocao->getRoom();
        QList<ServerPlayer *> lieges = room->getLieges("wei", caocao);
        if(lieges.isEmpty())
            return false;

        if(!room->askForSkillInvoke(caocao, objectName()))
            return false;

        room->playSkillEffect(objectName());
        foreach(ServerPlayer *liege, lieges){
            QString result = room->askForChoice(liege, objectName(), "accept+ignore");
            if(result == "ignore")
                continue;

            const Card *jink = room->askForCard(liege, "jink", "@hujia-jink");
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
        Room *room = guojia->getRoom();
        if(room->askForSkillInvoke(guojia, "tiandu")){
            if(card->objectName() == "shit"){
                QString result = room->askForChoice(guojia, objectName(), "yes+no");
                if(result == "no")
                    return false;
            }

            guojia->obtainCard(card);
            room->playSkillEffect(objectName());
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
        QVariant source = QVariant::fromValue(from);

        if(from && from->isAlive() && room->askForSkillInvoke(xiahou, "ganglie", source)){
            room->playSkillEffect(objectName());

            const Card *card = room->getJudgeCard(xiahou);
            if(card->getSuit() != Card::Heart){
                if(!room->askForDiscard(from, 2, true)){
                    DamageStruct damage;
                    damage.from = xiahou;
                    damage.to = from;

                    room->setEmotion(xiahou, Room::Good);
                    room->damage(damage);
                }else
                    room->setEmotion(xiahou, Room::Bad);
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
        QVariant data = QVariant::fromValue(from);
        if(from && !from->isNude() && room->askForSkillInvoke(simayi, "fankui", data)){
            int card_id = room->askForCardChosen(simayi, from, "he", "fankui");
            if(room->getCardPlace(card_id) == Player::Hand)
                room->moveCardTo(Sanguosha->getCard(card_id), simayi, Player::Hand, false);
            else
                room->obtainCard(simayi, card_id);
            room->playSkillEffect(objectName());
        }
    }
};

class Guicai:public OneCardViewAsSkill{
public:
    Guicai():OneCardViewAsSkill("guicai"){
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@guicai";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new GuicaiCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

class LuoyiBuff: public TriggerSkill{
public:
    LuoyiBuff():TriggerSkill("#luoyi"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *xuchu, QVariant &data) const{
        if(xuchu->hasFlag("luoyi")){
            DamageStruct damage = data.value<DamageStruct>();

            const Card *reason = damage.card;
            if(reason == NULL)
                return false;

            if(damage.chain)
                return false;

            if(reason->inherits("Slash") || reason->inherits("Duel")){
                damage.damage ++;
                data = QVariant::fromValue(damage);
            }
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
            room->playSkillEffect(objectName());

            xuchu->setFlags(objectName());
            return n - 1;
        }else
            return n;
    }
};

class LuoyiClear:public PhaseChangeSkill{
public:
    LuoyiClear():PhaseChangeSkill("#luoyi-clear"){
    }

    virtual bool onPhaseChange(ServerPlayer *xuchu) const{
        if(xuchu->getPhase() == Player::Finish && xuchu->hasFlag("luoyi")){
            xuchu->setFlags("-luoyi");
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

class Qingguo:public OneCardViewAsSkill{
public:
    Qingguo():OneCardViewAsSkill("qingguo"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isBlack() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Jink *jink = new Jink(card->getSuit(), card->getNumber());
        jink->setSkillName(objectName());
        jink->addSubcard(card->getId());
        return jink;
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
        if(Self->hasSkill("guixin2"))
            return Slash::IsAvailable();
        else
            return Self->getRoleEnum() == Player::Lord && Slash::IsAvailable();
    }

    virtual const Card *viewAs() const{
        return new JijiangCard;
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
        return target->isLord() && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *liubei, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "slash")
            return false;

        Room *room = liubei->getRoom();
        QList<ServerPlayer *> lieges = room->getLieges("shu", liubei);
        if(lieges.isEmpty())
            return false;

        if(!room->askForSkillInvoke(liubei, objectName()))
            return false;

        room->playSkillEffect(objectName());        
        foreach(ServerPlayer *liege, lieges){
            const Card *slash = room->askForCard(liege, "slash", "@jijiang-slash");
            if(slash){
                room->provide(slash);
                return true;
            }
        }

        return false;
    }
};

class Wusheng:public OneCardViewAsSkill{
public:
    Wusheng():OneCardViewAsSkill("wusheng"){
    }

    virtual bool isEnabledAtPlay() const{
        return Slash::IsAvailable();
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getCard();

        if(!card->isRed())
            return false;

        if(card == Self->getWeapon() && card->objectName() == "crossbow"){
            return Slash::IsAvailableWithCrossbow();
        }else
            return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *slash = new Slash(card->getSuit(), card->getNumber());
        slash->addSubcard(card->getId());
        slash->setSkillName(objectName());
        return slash;
    }
};

// should be ViewAsSkill
class Longdan:public OneCardViewAsSkill{
public:
    Longdan():OneCardViewAsSkill("longdan"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
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

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
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

class Kongcheng: public TriggerSkill{
public:
    Kongcheng():TriggerSkill("kongcheng"){
        frequency = Compulsory;

        events << CardLost;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->isKongcheng()){
            player->getRoom()->playSkillEffect(objectName());
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

class Yingzi:public DrawCardsSkill{
public:
    Yingzi():DrawCardsSkill("yingzi"){
        frequency = Frequent;
    }

    virtual int getDrawNum(ServerPlayer *zhouyu, int n) const{
        Room *room = zhouyu->getRoom();
        if(room->askForSkillInvoke(zhouyu, objectName())){
            room->playSkillEffect(objectName());
            return n + 1;
        }else
            return n;
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
            if(lumeng->getPhase() == Player::Finish)
                lumeng->setMark("slash_count", 0);
            else if(lumeng->getPhase() == Player::Discard){
                if(lumeng->getMark("slash_count") == 0 && lumeng->getRoom()->askForSkillInvoke(lumeng, objectName())) {
                    lumeng->getRoom()->playSkillEffect(objectName());
                    return true;
                }

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

class Qianxun: public ProhibitSkill{
public:
    Qianxun():ProhibitSkill("qianxun"){

    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("Snatch") || card->inherits("Indulgence");
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

class Qixi: public OneCardViewAsSkill{
public:
    Qixi():OneCardViewAsSkill("qixi"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Dismantlement *dismantlement = new Dismantlement(first->getSuit(), first->getNumber());
        dismantlement->addSubcard(first->getId());
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

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Diamond;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Indulgence *indulgence = new Indulgence(first->getSuit(), first->getNumber());
        indulgence->addSubcard(first->getId());
        indulgence->setSkillName(objectName());
        return indulgence;
    }
};

class LiuliViewAsSkill: public OneCardViewAsSkill{
public:
    LiuliViewAsSkill():OneCardViewAsSkill("liuli"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern.startsWith("@@liuli");;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        if(!ClientInstance->card_pattern.startsWith("@@liuli-"))
            return NULL;

        QString slash_source = ClientInstance->card_pattern;
        slash_source.remove("@@liuli-");

        LiuliCard *liuli_card = new LiuliCard;
        liuli_card->setSlashSource(slash_source);

        const Card *card = card_item->getCard();
        liuli_card->addSubcard(card->getId());
        liuli_card->setIsWeapon(Self->getWeapon() == card);

        return liuli_card;
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

class Lijian: public OneCardViewAsSkill{
public:
    Lijian():OneCardViewAsSkill("lijian"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! ClientInstance->turn_tag.value("lijian_used", false).toBool();
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LijianCard *lijian_card = new LijianCard;
        lijian_card->addSubcard(card_item->getCard()->getId());

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

class Qingnang: public OneCardViewAsSkill{
public:
    Qingnang():OneCardViewAsSkill("qingnang"){

    }

    virtual bool isEnabledAtPlay() const{
        return !ClientInstance->turn_tag.value("qingnang_used", false).toBool();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QingnangCard *qingnang_card = new QingnangCard;
        qingnang_card->addSubcard(card_item->getCard()->getId());

        return qingnang_card;
    }
};

class Jijiu: public OneCardViewAsSkill{
public:
    Jijiu():OneCardViewAsSkill("jijiu"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern.contains("peach") && Self->getPhase() == Player::NotActive;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Peach *peach = new Peach(first->getSuit(), first->getNumber());
        peach->addSubcard(first->getId());
        peach->setSkillName(objectName());
        return peach;
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
    xuchu->addSkill(new LuoyiClear);
    xuchu->addSkill(new LuoyiBuff);

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
    zhugeliang->addSkill(new Kongcheng);

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
    luxun->addSkill(new Qianxun);
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
    t["tiandu:yes"] = tr("tiandu:yes");
    t["tiandu:no"] = tr("tiandu:no");

    t["@jijiang-slash"] = tr("@jijiang-slash");
    t["@hujia-jink"] = tr("@hujia-jink");
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

    // lines
    t["$biyue1"]=tr("$biyue1");
    t["$biyue2"]=tr("$biyue2");
    t["$fanjian1"]=tr("$fanjian1");
    t["$fanjian2"]=tr("$fanjian2");
    t["$fankui"]=tr("$fankui");
    t["$ganglie1"]=tr("$ganglie1");
    t["$guanxing1"]=tr("$guanxing1");
    t["$guanxing2"]=tr("$guanxing2");
    t["$guicai"]=tr("$guicai");
    t["$guidao1"]=tr("$guidao1");
    t["$guidao2"]=tr("$guidao2");
    t["$guose1"]=tr("$guose1");
    t["$guose2"]=tr("$guose2");
    t["$hujia1"]=tr("$hujia1");
    t["$hujia2"]=tr("$hujia2");
    t["$jianxiong"]=tr("$jianxiong");
    t["$jieyin1"]=tr("$jieyin1");
    t["$jieyin2"]=tr("$jieyin2");
    t["$jijiang1"]=tr("$jijiang1");
    t["$jijiang2"]=tr("$jijiang2");
    t["$jijiu1"]=tr("$jijiu1");
    t["$jijiu2"]=tr("$jijiu2");
    t["$jiuyuan1"]=tr("$jiuyuan1");
    t["$jizhi"]=tr("$jizhi");
    t["$keji1"]=tr("$keji1");
    t["$keji2"]=tr("$keji2");
    t["$kongcheng1"]=tr("$kongcheng1");
    t["$kongcheng2"]=tr("$kongcheng2");
    t["$kurou"]=tr("$kurou");
    t["$lianying"]=tr("$lianying");
    t["$lijian1"]=tr("$lijian1");
    t["$lijian2"]=tr("$lijian2");
    t["$liuli1"]=tr("$liuli1");
    t["$liuli2"]=tr("$liuli2");
    t["$longdan1"]=tr("$longdan1");
    t["$longdan2"]=tr("$longdan2");
    t["$luoshen1"]=tr("$luoshen1");
    t["$luoshen2"]=tr("$luoshen2");
    t["$luoyi1"]=tr("$luoyi1");
    t["$luoyi2"]=tr("$luoyi2");
    t["$paoxiao1"]=tr("$paoxiao1");
    t["$qingguo"]=tr("$qingguo");
    t["$qingnang1"]=tr("$qingnang1");
    t["$qingnang2"]=tr("$qingnang2");
    t["$qixi1"]=tr("$qixi1");
    t["$qixi2"]=tr("$qixi2");
    t["$rende1"]=tr("$rende1");
    t["$rende2"]=tr("$rende2");
    t["$tiandu"]=tr("$tiandu");
    t["$tieji1"]=tr("$tieji1");
    t["$tuxi"]=tr("$tuxi");
    t["$wusheng1"]=tr("$wusheng1");
    t["$wusheng2"]=tr("$wusheng2");
    t["$wushuang"]=tr("$wushuang");
    t["$xiaoji1"]=tr("$xiaoji1");
    t["$xiaoji2"]=tr("$xiaoji2");
    t["$yiji"]=tr("$yiji");
    t["$yingzi1"]=tr("$yingzi1");
    t["$yingzi2"]=tr("$yingzi2");
    t["$zhiheng1"]=tr("$zhiheng1");
}
