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
            room->obtainCard(target, card);
            room->playSkillEffect(objectName());
        }
    }
};

class Hujia:public ZeroCardViewAsSkill{
public:
    Hujia():ZeroCardViewAsSkill("hujia$"){

    }

    virtual const Card *viewAs() const{
        return NULL;
    }

protected:
    virtual bool isEnabledAtPlay() const{
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

            if(can_invoke){
                QList<ServerPlayer *> targets;
                if(room->askForCardWithTargets(zhangliao, "@@tuxi", "@tuxi-card", targets)){
                    room->playSkillEffect(objectName());

                    foreach(ServerPlayer *target, targets){
                        int card_id = target->getRandomHandCard();
                        zhangliao->obtainCard(Sanguosha->getCard(card_id));
                    }

                    return true;
                }
            }
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

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, const QVariant &data) const{
        // FIXME
        // obtain the judge card

        return false;
    }
};

class Yiji:public MasochismSkill{
public:
    Yiji():MasochismSkill("yiji"){

    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{
        // FIXME
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
            // FIXME:
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

class Luoyi:public PhaseChangeSkill{
public:
    Luoyi():PhaseChangeSkill("luoyi"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        switch(target->getPhase()){
        case Player::Draw:{
                if(room->askForSkillInvoke(target, "luoyi")){
                    target->drawCards(1);
                    room->playSkillEffect(objectName());

                    setFlag(target);
                    return true;
                }
                break;
            }
        case Player::Finish:{
                if(target->hasFlag("luoyi"))
                    unsetFlag(target);
                break;
            }
        default:
            ;
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

                    int card_id = room->getJudgeCard(target);
                    const Card *card = Sanguosha->getCard(card_id);
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

class Rende:public ViewAsSkill{
public:
    Rende():ViewAsSkill("rende"){
    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        Card *rende_card = new RendeCard;
        foreach(CardItem *card_item, cards)
            rende_card->addSubcard(card_item->getCard()->getId());
        return rende_card;
    }
};

class Jijiang:public Skill{
public:
    Jijiang():Skill("jijiang$"){

    }
};

class Wusheng:public ViewAsSkill{
public:
    Wusheng()
        :ViewAsSkill("wusheng")
    {
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

        if(to_select->isEquipped())
            return false;

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                // jink as slash
                return to_select->objectName() == "jink";
            }

        case Client::Responsing:{
                QString pattern = ClientInstance->card_pattern;
                if(pattern == "slash")
                    return to_select->inherits("Jink");
                else if(pattern == "jink")
                    return to_select->inherits("Slash");
            }

        default:
            return false;
        }
    }

    virtual bool isEnabledAtResponse() const{
        QString pattern = ClientInstance->card_pattern;
        return pattern == "jink" || pattern == "slash";
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        const Card *card = cards.first()->getCard();
        if(card->objectName() == "slash")
            return new Jink(card->getSuit(), card->getNumber());
        else if(card->objectName() == "jink")
            return new Slash(card->getSuit(), card->getNumber());
        else
            return NULL;
    }
};

class Tieji:public SlashBuffSkill{
public:
    Tieji():SlashBuffSkill("tieji"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        const Slash *slash = effect.slash;
        ServerPlayer *machao = effect.from;

        Room *room = machao->getRoom();
        if(room->askForSkillInvoke(effect.from, "tieji")){
            room->playSkillEffect(objectName());

            int card_id = room->getJudgeCard(machao);
            const Card *card = Sanguosha->getCard(card_id);
            if(card->isRed()){
                SlashResultStruct result;
                result.from = machao;
                result.to = effect.to;
                result.slash = slash;
                result.nature = effect.nature;
                result.success = true;
                room->slashResult(result);
                return true;
            }
        }

        return false;
    }
};

class Mashu:public GameStartSkill{
public:
    Mashu():GameStartSkill("mashu"){

    }

    virtual void onGameStart(ServerPlayer *player) const{
        player->getRoom()->setPlayerCorrect(player, "skill_src", -1);
    }
};

class Guanxing:public PhaseChangeSkill{
public:
    Guanxing():PhaseChangeSkill("guanxing"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        // FIXME
        return false;
    }
};

class Jizhi:public TriggerSkill{
public:
    Jizhi():TriggerSkill("jizhi"){
        frequency = Frequent;
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, const QVariant &data) const{
        if(data.canConvert<CardUseStruct>()){
            CardUseStruct use = data.value<CardUseStruct>();

            if(use.card->inherits("TrickCard") && !use.card->inherits("DelayedTrick")){
                if(player->getRoom()->askForSkillInvoke(player, objectName())){
                    player->drawCards(1);
                }
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

        Card *zhiheng_card = new ZhihengCard;
        foreach(CardItem *card_item, cards)
            zhiheng_card->addSubcard(card_item->getCard()->getId());

        return zhiheng_card;
    }

    virtual bool isEnabledAtPlay() const{
        return ! ClientInstance->turn_tag.value("zhiheng_used", false).toBool();
    }
};

class Jiuyuan:public Skill{
public:
    Jiuyuan():Skill("jiuyuan$"){

    }
};

class Yingzi:public PhaseChangeSkill{
public:
    Yingzi():PhaseChangeSkill("yingzi"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Draw){
            Room *room = target->getRoom();            
            if(room->askForSkillInvoke(target, objectName())){
                room->drawCards(target, 1);
                room->playSkillEffect(objectName());
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
        return !Self->isKongcheng();
    }

    virtual const Card *viewAs() const{
        return new FanjianCard;
    }
};

class Keji: public PhaseChangeSkill{
public:
    Keji():PhaseChangeSkill("keji"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Discard){            
            // FIXME
            return true;
        }else
            return false;
    }
};

class Lianying: public TriggerSkill{
public:
    Lianying():TriggerSkill("lianying"){
        events << CardMove;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *luxun, const QVariant &) const{
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
        return indulgence;
    }
};

class Liuli: public ViewAsSkill{
public:
    Liuli():ViewAsSkill("liuli"){

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

        QRegExp rx("@@liuli-(.+)");
        if(!rx.exactMatch(ClientInstance->card_pattern))
            return NULL;

        QString slash_source = rx.capturedTexts().at(1);
        LiuliCard *card = new LiuliCard;
        card->setSlashSource(slash_source);
        card->addSubcard(cards.first()->getCard()->getId());

        return card;
    }
};

class Jieyin: public ViewAsSkill{
public:
    Jieyin():ViewAsSkill("jieyin"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() > 2)
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;
        else{
            JieyinCard *jieyin_card = new JieyinCard();
            jieyin_card->addSubcard(cards.at(0)->getCard()->getId());
            jieyin_card->addSubcard(cards.at(1)->getCard()->getId());

            return jieyin_card;
        }
    }
};

class Xiaoji: public TriggerSkill{
public:
    Xiaoji():TriggerSkill("xiaoji"){
        events << CardMove;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *sunshangxiang, const QVariant &data) const{
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

class Lijian: public ViewAsSkill{
public:
    Lijian():ViewAsSkill("lijian"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !selected.isEmpty();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        LijianCard *lijian_card = new LijianCard;
        foreach(CardItem *card_item, cards)
            lijian_card->addSubcard(card_item->getCard()->getId());

        return lijian_card;
    }
};

class Biyue: public PhaseChangeSkill{
public:
    Biyue():PhaseChangeSkill("biyue"){

    }

    virtual bool onPhaseChange(ServerPlayer *diaochan) const{
        if(diaochan->getPhase() == Player::Finish){
            if(diaochan->getRoom()->askForSkillInvoke(diaochan, objectName())){
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

    virtual bool isAvailableAtPlay(){
        return false;
    }

    virtual bool isAvailableAtResponse(){
        return ClientInstance->card_pattern == "peach";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && to_select->getCard()->isRed();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 1){
            const Card *first = cards.first()->getCard();
            Peach *peach = new Peach(first->getSuit(), first->getNumber());
            return peach;
        }

        return NULL;
    }
};

#ifndef QT_NO_DEBUG

class Luanji:public ViewAsSkill{
public:
    Luanji():ViewAsSkill("luanji"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return !to_select->isEquipped();
        else if(selected.length() == 1){
            const Card *card = selected.first()->getCard();
            return !to_select->isEquipped() && to_select->getCard()->getSuit() == card->getSuit();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            const Card *first = cards.first()->getCard();
            Card::Suit suit = first->isRed() ? Card::Heart : Card::Spade;
            return new ArcheryAttack(suit, 0);
        }else
            return NULL;
    }
};

#endif

void StandardPackage::addGenerals(){
    General *caocao, *zhangliao, *guojia, *xiahoudun, *simayi, *xuchu, *zhenji;

    caocao = new General(this, "caocao$", "wei");
    caocao->addSkill(new Jianxiong);
    caocao->addSkill(new Hujia);

#ifndef QT_NO_DEBUG

#endif

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
    huangyueying->addSkill(new Skill("qicai"));

    General *sunquan, *zhouyu, *lumeng, *luxun, *ganning, *huanggai, *daqiao, *sunshangxiang;
    sunquan = new General(this, "sunquan$", "wu");
    sunquan->addSkill(new Zhiheng);
    sunquan->addSkill(new Jiuyuan);

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
    daqiao->addSkill(new Skill("liuli"));

    sunshangxiang = new General(this, "sunshangxiang", "wu", 3, false);
    sunshangxiang->addSkill(new Jieyin);
    sunshangxiang->addSkill(new Xiaoji);

    General *lubu, *huatuo, *diaochan;

    lubu = new General(this, "lubu", "qun");
    lubu->addSkill(new Skill("wushuang", Skill::Compulsory));

    huatuo = new General(this, "huatuo", "qun", 3);
    huatuo->addSkill(new Qingnang);
    huatuo->addSkill(new Jijiu);

    diaochan = new General(this, "diaochan", "qun", 3, false);
    diaochan->addSkill(new Lijian);
    diaochan->addSkill(new Biyue);

    // for skill cards    
    metaobjects << &ZhihengCard::staticMetaObject
            << &RendeCard::staticMetaObject
            << &JieyinCard::staticMetaObject
            << &TuxiCard::staticMetaObject
            << &KurouCard::staticMetaObject
            << &LijianCard::staticMetaObject
            << &FanjianCard::staticMetaObject
            << &GuicaiCard::staticMetaObject
            << &QingnangCard::staticMetaObject;

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

    t["@wushuang-slash-1"] = tr("@wushuang-slash-1");
    t["@wushuang-slash-2"] = tr("@wushuang-slash-2");
    t["@wushuang-jink-1"] = tr("@wushuang-jink-1");
    t["@wushuang-jink-2"] = tr("@wushuang-jink-2");
    t["@guicai-card"] = tr("@guicai-card");
    t["@guidao-card"] = tr("@guidao-card");
    t["@liuli-card"] = tr("@liuli-card");

    t["luanji"] = tr("luanji");
}
