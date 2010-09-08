#include "thicket.h"
#include "general.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "maneuvering.h"

class Xingshang: public GameStartSkill{
public:
    Xingshang():GameStartSkill("xingshang"){

    }

    virtual void onGameStart(ServerPlayer *player) const{
        player->getRoom()->setLegatee(player);
    }
};

class Fangzhu: public MasochismSkill{
public:
    Fangzhu():MasochismSkill("fangzhu"){

    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{
        // exile somebody
    }
};

class Songwei: public TriggerSkill{
public:
    Songwei():TriggerSkill("songwei$"){

    }

    virtual void getTriggerEvents(QList<TriggerEvent> &events) const{
        events << JudgeOnEffect;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, const QVariant &data) const{
        return false;
    }
};

class Duanliang: public ViewAsSkill{
public:
    Duanliang():ViewAsSkill("duanliang"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(!selected.isEmpty())
            return false;

        const Card *card = to_select->getCard();
        return card->isBlack() && !card->inherits("TrickCard");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        const Card *first = cards.first()->getCard();

        SupplyShortage *card = new SupplyShortage(first->getSuit(), first->getNumber());
        card->addSubcard(first->getId());

        return card;
    }
};

class Huoshou: public GameStartSkill{
public:
    Huoshou():GameStartSkill("huoshou"){

    }

    void onGameStart(ServerPlayer *player) const{
        player->getRoom()->setMenghuo(player);
    }
};

class Guixin: public MasochismSkill{
public:
    Guixin():MasochismSkill("guixin"){

    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{

    }
};

class Feiying: public GameStartSkill{
public:
    Feiying():GameStartSkill("feiying"){

    }

    virtual void onGameStart(ServerPlayer *player) const{
        player->getRoom()->setPlayerCorrect(player, "skill_dest", +1);
    }
};

ThicketPackage::ThicketPackage()
    :Package("thicket")
{
    General *caopi, *xuhuang, *menghuo, *zhurong, *sunjian, *lusu, *jiaxu, *dongzhuo;

    caopi = new General(this, "caopi$", "wei", 3);
    caopi->addSkill(new Xingshang);
    caopi->addSkill(new Fangzhu);
    caopi->addSkill(new Songwei);

    xuhuang = new General(this, "xuhuang", "wei");
    xuhuang->addSkill(new Duanliang);

    menghuo = new General(this, "menghuo", "shu");
    menghuo->addSkill(new Huoshou);

    zhurong = new General(this, "zhurong", "shu", 4, false);

    sunjian = new General(this, "sunjian", "wu");
    lusu = new General(this, "lusu", "wu", 3);

    jiaxu = new General(this, "jiaxu", "qun", 3);
    dongzhuo = new General(this, "dongzhuo$", "qun", 8);

    // two gods !!
    General *shencaocao, *shenlubu;

    shencaocao = new General(this, "shencaocao$", "wei", 3);
    shencaocao->addSkill(new Guixin);
    shencaocao->addSkill(new Feiying);

    shenlubu = new General(this, "shenlubu", "qun", 5);

    t["thicket"] = tr("thicket");

    t["caopi"] = tr("caopi");
    t["xuhuang"] = tr("xuhuang");
    t["menghuo"] = tr("menghuo");
    t["zhurong"] = tr("zhurong");
    t["sunjian"] = tr("sunjian");
    t["lusu"] = tr("lusu");
    t["jiaxu"] = tr("jiaxu");
    t["dongzhuo"] = tr("dongzhuo");

    t["shencaocao"] = tr("shencaocao");
    t["shenlubu"] = tr("shenlubu");

    // skills
    t["xingshang"] = tr("xingshang");
    t["fangzhu"] = tr("fangzhu");
    t["songwei"] = tr("songwei");
    t["duanliang"] = tr("duanliang");
    t["huoshuo"] = tr("huoshuo");
    t["zaiqi"] = tr("zaiqi");
    t["juxiang"] = tr("juxiang");
    t["lieren"] = tr("lieren");
    t["yinghun"] = tr("yinghun");
    t["haoshi"] = tr("haoshi");
    t["dimeng"] = tr("dimeng");
    t["wansha"] = tr("wansha");
    t["weimu"] = tr("weimu");
    t["luanwu"] = tr("luanwu");
    t["jiuchi"] = tr("jiuchi");
    t["roulin"] = tr("roulin");
    t["benghuai"] = tr("benghuai");
    t["baonue"] = tr("baonue");

    t["guixin"] = tr("guixin");
    t["feiying"] = tr("feiying");    
}

extern "C" {
    Q_DECL_EXPORT Package *NewThicket(){
        return new ThicketPackage;
    }
}
