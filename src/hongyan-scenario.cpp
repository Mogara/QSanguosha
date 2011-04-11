#include "hongyan-scenario.h"
#include "engine.h"
#include "standard-skillcards.h"
#include "clientplayer.h"
#include "client.h"
#include "carditem.h"

LesbianJieyinCard::LesbianJieyinCard()
    :JieyinCard()
{

}

bool LesbianJieyinCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    return to_select->getGeneral()->isFemale() && to_select->isWounded();
}

class LesbianJieyin: public ViewAsSkill{
public:
    LesbianJieyin():ViewAsSkill("lesbianjieyin"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("LesbianJieyinCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() > 2)
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        LesbianJieyinCard *jieyin_card = new LesbianJieyinCard();
        jieyin_card->addSubcards(cards);

        return jieyin_card;
    }
};

//-----------------------------

LesbianLijianCard::LesbianLijianCard()
    :LijianCard()
{
}

bool LesbianLijianCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!to_select->getGeneral()->isFemale())
        return false;

    if(targets.isEmpty() && to_select->hasSkill("kongcheng") && to_select->isKongcheng()){
        return false;
    }

    return true;
}

class LesbianLijian: public OneCardViewAsSkill{
public:
    LesbianLijian():OneCardViewAsSkill("lesbianlijian"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! Self->hasUsed("LesbianLijianCard");
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LesbianLijianCard *lijian_card = new LesbianLijianCard;
        lijian_card->addSubcard(card_item->getCard()->getId());

        return lijian_card;
    }
};

class HongyanRule: public ScenarioRule{
public:
    HongyanRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(player->hasSkill("jieyin"))
            room->acquireSkill(player, "lesbianjieyin");
        else if(player->hasSkill("lijian"))
            room->acquireSkill(player, "lesbianlijian");

        return false;
    }
};

HongyanScenario::HongyanScenario()
    :Scenario("hongyan_lesbian")
{
    females << "zhenji" << "huangyueying" << "zhurong"
            << "daqiao" << "luxun" << "xiaoqiao"
            << "diaochan" << "caizhaoji" << "sunshangxiang";

    standard_roles << "lord" << "loyalist" << "loyalist"
            << "rebel" << "rebel" << "rebel" << "rebel" << "renegade";

    rule = new HongyanRule(this);

    skills << new LesbianJieyin << new LesbianLijian;

    addMetaObject<LesbianJieyinCard>();
    addMetaObject<LesbianLijianCard>();
}

bool HongyanScenario::exposeRoles() const{
    return false;
}

void HongyanScenario::assign(QStringList &generals, QStringList &roles) const{
    generals = females;
    qShuffle(generals);
    generals.removeLast();

    roles = standard_roles;
    qShuffle(roles);
}

int HongyanScenario::getPlayerCount() const{
    return 8;
}

void HongyanScenario::getRoles(char *roles) const{
    strcpy(roles, "ZCCFFFN");
}

void HongyanScenario::onTagSet(Room *room, const QString &key) const{
    // dummy
}

ADD_SCENARIO(Hongyan)
