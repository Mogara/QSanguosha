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
        // FIXME
        // obtain the card that cause damage
    }
};

class Hujia:public Skill{
public:
    Hujia():Skill("hujia$"){

    }
};

class Tuxi:public Skill{
public:
    Tuxi():Skill("tuxi"){

    }
};

class Tiandu:public PassiveSkill{
public:
    Tiandu():PassiveSkill("tiandu"){
        frequency = Frequent;
    }

    virtual void getTriggerEvents(QList<Room::TriggerEvent> &events) const{
        events << Room::JudgeOnEffect;
    }

    virtual bool trigger(Room::TriggerEvent event, ServerPlayer *player, const QVariant &data) const{
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

class Ganglie:public Skill{
public:
    Ganglie():Skill("ganglie"){

    }
};

class Fankui:public Skill{
public:
    Fankui():Skill("fankui"){

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
    Qingguo():ViewAsSkill("qingguo", false){

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
    Rende():ViewAsSkill("rende", false){
    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        Card *rende_card = new RendeCard;
        foreach(CardItem *card_item, cards)
            rende_card->addSubcard(card_item->getCard()->getID());
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
        :ViewAsSkill("wusheng", true)
    {
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
            slash->addSubcard(card->getID());
            return slash;
        }
    }
};

class Paoxiao:public EnvironSkill{
public:
    Paoxiao():EnvironSkill("paoxiao"){
    }

    virtual bool trigger(Room::TriggerEvent event, ServerPlayer *player, const QVariant &data) const{
        Room *room = getRoom(player);

        ActiveRecord *record = new ActiveRecord;
        record->method = "setPlayerFlag";
        record->target = player;
        record->data = "paoxiao";

        room->enqueueRecord(record);

        return false;
    }
};

class Longdan:public Skill{
public:
    Longdan():Skill("longdan"){

    }
};

class Tieji:public Skill{
public:
    Tieji():Skill("tieji"){

    }
};

class Mashu:public EnvironSkill{
public:
    Mashu():EnvironSkill("mashu"){

    }

    virtual bool trigger(Room::TriggerEvent event, ServerPlayer *player, const QVariant &data) const{
        Room *room = getRoom(player);

        ActiveRecord *record = new ActiveRecord;
        record->method = "setPlayerProperty";
        record->target = player;

        QVariantList list;
        list << "correct" << "skill_src:-1";

        record->data = list;

        room->enqueueRecord(record);

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

class Kongcheng:public Skill{
public:
    Kongcheng():Skill("kongcheng"){

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
    Zhiheng():ViewAsSkill("zhiheng", true){

    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        Card *zhiheng_card = new ZhihengCard;
        foreach(CardItem *card_item, cards)
            zhiheng_card->addSubcard(card_item->getCard()->getID());

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
            Room *room = getRoom(target);

            ActiveRecord *ask = new ActiveRecord;
            ask->method = "askForSkillInvoke";
            ask->target = target;
            ask->data = "yingzi:yes+no";

            room->enqueueRecord(ask);
        }

        return false;
    }

    virtual void onOption(ServerPlayer *target, const QString &option) const{
        Room *room = getRoom(target);
        if(option == "yes"){
            room->drawCards(target, 1);
            room->playSkillEffect(objectName());
        }
    }
};

class Fanjian:public Skill{
public:
    Fanjian():Skill("fanjian"){

    }
};

#ifndef QT_NO_DEBUG

class Luanji:public ViewAsSkill{
public:
    Luanji():ViewAsSkill("luanji", false){
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
            return Sanguosha->cloneCard("archery_attack", suit, 0);
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
    luxun = new General(this, "luxun", "wu", 3);
    ganning = new General(this, "ganning", "wu");
    huanggai = new General(this, "huanggai", "wu");
    daqiao = new General(this, "daqiao", "wu", 3, false);
    sunshangxiang = new General(this, "sunshangxiang", "wu", 3, false);

    General *lubu, *huatuo, *diaochan;
    lubu = new General(this, "lubu", "qun");
    huatuo = new General(this, "huatuo", "qun", 3);
    diaochan = new General(this, "diaochan", "qun", 3, false);

    // for skill cards    
    metaobjects << &ZhihengCard::staticMetaObject
            << &RendeCard::staticMetaObject;

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

    t["luanji"] = tr("luanji");
}
