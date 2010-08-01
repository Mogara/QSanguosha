#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"

class Jianxiong:public Skill{
public:
    Jianxiong():Skill("jianxiong+"){
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

class Tiandu:public Skill{
public:
    Tiandu():Skill("tiandu+"){

    }
};

class Yiji:public Skill{
public:
    Yiji():Skill("yiji+"){

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

class Luoshen:public Skill{
public:
    Luoshen():Skill("luoshen+"){

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
        // FIXME
        return NULL;
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
            return false;
        else{
            const Card *card = cards.first()->getCard();
            return Sanguosha->cloneCard("slash", card->getSuit(), card->getNumber());
        }
    }
};

class Paoxiao:public Skill{
public:
    Paoxiao():Skill("paoxiao+"){
    }

    virtual void trigger(TriggerReason reason, const QString &data) const{
        if(reason == GameStart)
            ClientInstance->tag["unlimited_slash"] = true;
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

class Mashu:public Skill{
public:
    Mashu():Skill("mashu!"){

    }
};

class Guanxing:public Skill{
public:
    Guanxing():Skill("guanxing+"){

    }
};

class Kongcheng:public Skill{
public:
    Kongcheng():Skill("kongcheng!"){

    }
};

class Jizhi:public Skill{
public:
    Jizhi():Skill("jizhi+"){

    }

    void trigger(TriggerReason reason, const QVariant &data) const{

    }
};

class Qicai:public Skill{
public:
    Qicai():Skill("qicai!"){

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
        return NULL;
    }
};

class Jiuyuan:public Skill{
public:
    Jiuyuan():Skill("jiuyuan$"){

    }
};

class Yingzi:public Skill{
public:
    Yingzi():Skill("yingzi+"){

    }

    virtual void trigger(TriggerReason reason, const QVariant &data) const{
        if(reason == PhaseChange && ClientInstance->getPlayer()->getPhase() == Player::Draw){
            ClientInstance->askForCards(1);
            playEffect();
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

}
