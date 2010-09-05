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

class Tuxi:public Skill{
public:
    Tuxi():Skill("tuxi"){

    }
};

class Tiandu:public TriggerSkill{
public:
    Tiandu():TriggerSkill("tiandu"){
        frequency = Frequent;
    }

    virtual void getTriggerEvents(QList<TriggerEvent> &events) const{
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

class Guicai:public Skill{
public:
    Guicai():Skill("guicai"){

    }
};

class Luoyi:public Skill{
public:
    Luoyi():Skill("luoyi"){

    }
};

class Luoshen:public PhaseChangeSkill{
public:
    Luoshen():PhaseChangeSkill("luoshen"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        // FIXME
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
            return Sanguosha->cloneCard("jink", card->getSuit(), card->getNumber());
        }
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

class Paoxiao:public FlagSkill{
public:
    Paoxiao():FlagSkill("paoxiao"){
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
                    return to_select->objectName() == "jink";
                else if(pattern == "jink")
                    return to_select->objectName() == "slash";
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

class Tieji:public Skill{
public:
    Tieji():Skill("tieji"){

    }
};

class Mashu:public GameStartSkill{
public:
    Mashu():GameStartSkill("mashu"){

    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, const QVariant &data) const{
        player->getRoom()->setPlayerCorrect(player, "skill_src", -1);
        return false;
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

class Kongcheng:public FlagSkill{
public:
    Kongcheng():FlagSkill("kongcheng"){
    }
};

class Jizhi:public Skill{
public:
    Jizhi():Skill("jizhi"){

    }
};

class Qicai:public Skill{
public:
    Qicai():Skill("qicai"){

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

class Fanjian:public Skill{
public:
    Fanjian():Skill("fanjian"){

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

class Qianxun: public FlagSkill{
public:
    Qianxun():FlagSkill("qianxun"){

    }
};

class Lianying: public Skill{
public:
    Lianying():Skill("lianying"){

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
        else{
            const Card *first = cards.first()->getCard();
            Indulgence *indulgence = new Indulgence(first->getSuit(), first->getNumber());
            indulgence->addSubcard(first->getId());
            return indulgence;
        }
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

class Wushuang: public FlagSkill{
public:
    Wushuang():FlagSkill("wushuang"){

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
    caocao->addSkill(new Wusheng);
    caocao->addSkill(new Yingzi);
    caocao->addSkill(new Luanji);
    caocao->addSkill(new Rende);
    caocao->addSkill(new Zhiheng);
    caocao->addSkill(new Kurou);
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
    zhangfei->addSkill(new Paoxiao);

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
    huangyueying->addSkill(new Qicai);

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
    luxun->addSkill(new Qianxun);
    luxun->addSkill(new Lianying);

    ganning = new General(this, "ganning", "wu");
    ganning->addSkill(new Qixi);

    huanggai = new General(this, "huanggai", "wu");
    huanggai->addSkill(new Kurou);

    daqiao = new General(this, "daqiao", "wu", 3, false);
    daqiao->addSkill(new Guose);

    sunshangxiang = new General(this, "sunshangxiang", "wu", 3, false);
    sunshangxiang->addSkill(new Jieyin);

    General *lubu, *huatuo, *diaochan;

    lubu = new General(this, "lubu", "qun");
    lubu->addSkill(new Wushuang);

    huatuo = new General(this, "huatuo", "qun", 3);

    diaochan = new General(this, "diaochan", "qun", 3, false);
    diaochan->addSkill(new Lijian);

    // for skill cards    
    metaobjects << &ZhihengCard::staticMetaObject
            << &RendeCard::staticMetaObject
            << &JieyinCard::staticMetaObject
            << &TuxiCard::staticMetaObject
            << &KurouCard::staticMetaObject
            << &LijianCard::staticMetaObject;

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
    t["yingzi:no"] = t["nothing"];
    t["jianxiong:yes"] = tr("jianxiong:yes");
    t["jianxiong:no"] = t["nothing"];
    t["fankui:yes"] = tr("fankui:yes");
    t["fankui:nothing"] = t["nothing"];

    t["@wushuang-slash-1"] = tr("@wushuang-slash-1");
    t["@wushuang-slash-2"] = tr("@wushuang-slash-2");

    t["luanji"] = tr("luanji");
}
