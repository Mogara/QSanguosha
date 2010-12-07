#include "god.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"

GongxinCard::GongxinCard(){
}

bool GongxinCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && !to_select->isKongcheng();
}

void GongxinCard::use(const QList<const ClientPlayer *> &targets) const{
    ClientInstance->turn_tag.insert("gongxin_used", true);
}

void GongxinCard::onEffect(const CardEffectStruct &effect) const{
    effect.from->getRoom()->doGongxin(effect.from, effect.to);
}

class Wuhun: public TriggerSkill{
public:
    Wuhun():TriggerSkill("wuhun"){
        events << Damage;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->hasSkill(objectName())){
            Room *room = player->getRoom();
            room->setPlayerMarkDelta(player, "@nightmare", damage.damage);
        }

        return false;
    }
};

class WuhunRevenge: public TriggerSkill{
public:
    WuhunRevenge():TriggerSkill("#wuhun"){
        events << Death;
    }

    virtual int getPriority(ServerPlayer *target) const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill("wuhun");
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *shenguanyu, QVariant &) const{
        Room *room = shenguanyu->getRoom();
        QList<ServerPlayer *> players = room->getOtherPlayers(shenguanyu);
        players << shenguanyu;

        int max = 0;
        foreach(ServerPlayer *player, players){
            int value = player->getMark("@nightmare");
            if(value > max)
                max = value;
        }

        if(max == 0)
            return false;

        QList<ServerPlayer *> foes;
        foreach(ServerPlayer *player, players){
            if(player->getMark("@nightmare") == max)
                foes << player;
        }

        if(foes.isEmpty())
            return false;

        ServerPlayer *foe;
        if(foes.length() == 1)
            foe = foes.first();
        else
            foe = room->askForPlayerChosen(shenguanyu, foes);

        const Card *card = room->getJudgeCard(foe);
        if(!card->inherits("Peach") && !card->inherits("GodSalvation") && foe != shenguanyu)
            room->killPlayer(foe);

        return false;
    }
};

static bool CompareBySuit(int card1, int card2){
    const Card *c1 = Sanguosha->getCard(card1);
    const Card *c2 = Sanguosha->getCard(card2);

    int a = static_cast<int>(c1->getSuit());
    int b = static_cast<int>(c2->getSuit());

    return a < b;
}

class Shelie: public PhaseChangeSkill{
public:
    Shelie():PhaseChangeSkill("shelie"){

    }

    virtual bool onPhaseChange(ServerPlayer *shenlumeng) const{
        if(shenlumeng->getPhase() != Player::Draw)
            return false;

        Room *room = shenlumeng->getRoom();
        if(!room->askForSkillInvoke(shenlumeng, objectName()))
            return false;

        QList<int> card_ids = room->getNCards(5);
        qSort(card_ids.begin(), card_ids.end(), CompareBySuit);
        QStringList card_str;
        foreach(int card_id, card_ids)
            card_str << QString::number(card_id);
        room->broadcastInvoke("fillAG", card_str.join("+"));

        while(!card_ids.isEmpty()){
            int card_id = room->askForAG(shenlumeng, card_ids);
            card_ids.removeOne(card_id);
            room->takeAG(shenlumeng, card_id);

            // throw the rest cards that matches the same suit
            const Card *card = Sanguosha->getCard(card_id);
            Card::Suit suit = card->getSuit();
            QMutableListIterator<int> itor(card_ids);
            while(itor.hasNext()){
                const Card *c = Sanguosha->getCard(itor.next());
                if(c->getSuit() == suit){
                    itor.remove();

                    room->takeAG(NULL, c->getId());
                }
            }
        }

        room->broadcastInvoke("clearAG");

        return true;
    }
};

class Gongxin: public ZeroCardViewAsSkill{
public:
    Gongxin():ZeroCardViewAsSkill("gongxin"){
        default_choice = "discard";
    }

    virtual const Card *viewAs() const{
        return new GongxinCard;
    }

    virtual bool isEnabledAtPlay() const{
        return !ClientInstance->turn_tag.value("gongxin_used", false).toBool();
    }
};

void YeyanCard::use(const QList<const ClientPlayer *> &targets) const{
    ClientInstance->tag.insert("yeyan_used", true);
}

void YeyanCard::damage(ServerPlayer *shenzhouyu, ServerPlayer *target, int point) const{
    DamageStruct damage;

    damage.card = NULL;
    damage.from = shenzhouyu;
    damage.to = target;
    damage.damage = point;
    damage.nature = DamageStruct::Fire;

    shenzhouyu->getRoom()->damage(damage);
}

GreatYeyanCard::GreatYeyanCard(){

}

bool GreatYeyanCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty();
}

void GreatYeyanCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    room->loseHp(source, 3);

    damage(source, targets.first(), 3);
}

MediumYeyanCard::MediumYeyanCard(){

}

bool MediumYeyanCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.length() < 2;
}

void MediumYeyanCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->loseHp(source, 3);

    ServerPlayer *first = targets.first();
    ServerPlayer *second = targets.length() >= 2 ? targets.at(1) : NULL;

    damage(source, first, 2);

    if(second)
        damage(source, second, 1);
}

SmallYeyanCard::SmallYeyanCard(){

}

bool SmallYeyanCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.length() < 3;
}

void SmallYeyanCard::onEffect(const CardEffectStruct &effect) const{
    damage(effect.from, effect.to, 1);
}

class GreatYeyan: public ViewAsSkill{
public:
    GreatYeyan(): ViewAsSkill("greatyeyan"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay() const{
        return ! ClientInstance->tag.value("yeyan_used", false).toBool();
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 4)
            return false;

        foreach(CardItem *item, selected){
            if(to_select->getCard()->getSuit() == item->getCard()->getSuit())
                return false;
        }

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 4)
            return NULL;

        GreatYeyanCard *card = new GreatYeyanCard;
        card->addSubcards(cards);

        return card;
    }
};

class MediumYeyan: public GreatYeyan{
public:
    MediumYeyan(){
        setObjectName("mediumyeyan");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 4)
            return NULL;

        MediumYeyanCard *card = new MediumYeyanCard;
        card->addSubcards(cards);

        return card;
    }
};

class SmallYeyan: public ZeroCardViewAsSkill{
public:
    SmallYeyan():ZeroCardViewAsSkill("smallyeyan"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay() const{
        return ! ClientInstance->tag.value("yeyan_used", false).toBool();
    }

    virtual const Card *viewAs() const{
        return new SmallYeyanCard;
    }
};

class Qinyin: public TriggerSkill{
public:
    Qinyin():TriggerSkill("qinyin"){
        events << CardDiscarded;
        default_choice = "down";
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        int discard_num = data.toInt();

        if(player->getPhase() == Player::Discard && discard_num >= 2){
            Room *room = player->getRoom();

            if(!room->askForSkillInvoke(player, objectName()))
                return false;

            QString result = room->askForChoice(player, objectName(), "up+down");
            QList<ServerPlayer *> all_players = room->getAllPlayers();
            if(result == "up"){
                foreach(ServerPlayer *player, all_players){
                    room->recover(player);
                }
            }else if(result == "down"){
                foreach(ServerPlayer *player, all_players){
                    room->loseHp(player);
                }
            }
        }

        return false;
    }
};

class Guixin: public MasochismSkill{
public:
    Guixin():MasochismSkill("guixin"){

    }

    virtual void onDamaged(ServerPlayer *shencc, const DamageStruct &damage) const{
        Room *room = shencc->getRoom();
        int i, x = damage.damage;
        for(i=0; i<x; i++){
            if(room->askForSkillInvoke(shencc, objectName())){
                room->playSkillEffect(objectName());

                QList<ServerPlayer *> players = room->getOtherPlayers(shencc);
                foreach(ServerPlayer *player, players){
                    if(!player->isAllNude()){
                        int card_id = room->askForCardChosen(shencc, player, "hej", objectName());
                        if(room->getCardPlace(card_id) == Player::Hand)
                            room->moveCardTo(Sanguosha->getCard(card_id), shencc, Player::Hand, false);
                        else
                            room->obtainCard(shencc, card_id);
                    }
                }

                shencc->turnOver();
                room->broadcastProperty(shencc, "faceup");
            }else
                break;
        }
    }
};

Feiying::Feiying()
    :GameStartSkill("feiying")
{
    frequency = Compulsory;
}

void Feiying::onGameStart(ServerPlayer *player) const
{
    player->getRoom()->setPlayerCorrect(player, "F");
};

class KuangbaoStart: public GameStartSkill{
public:
    KuangbaoStart():GameStartSkill("#kuangbao"){
    }

    virtual void onGameStart(ServerPlayer *player) const{
        Room *room = player->getRoom();
        room->setPlayerMarkDelta(player, "@wrath", +2);
    }
};

class Kuangbao: public TriggerSkill{
public:
    Kuangbao():TriggerSkill("kuangbao"){
        events << Damage << Damaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        room->setPlayerMarkDelta(player, "@wrath", damage.damage);

        return false;
    }
};

class Wumou:public TriggerSkill{
public:
    Wumou():TriggerSkill("wumou"){
        frequency = Compulsory;
        events << CardUsed << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
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
            Room *room = player->getRoom();
            int num = player->getMark("@wrath");
            if(num >= 1 && room->askForChoice(player, objectName(), "discard+losehp") == "discard"){
                room->setPlayerMarkDelta(player, "@wrath", -1);
            }else
                room->loseHp(player);
        }

        return false;
    }
};

class Shenfen:public ZeroCardViewAsSkill{
public:
    Shenfen():ZeroCardViewAsSkill("shenfen"){
    }

    virtual bool isEnabledAtPlay() const{
        return Self->getMark("@wrath") >= 6 && !ClientInstance->turn_tag.value("shenfen_used", false).toBool();
    }

    virtual const Card *viewAs() const{
        return new ShenfenCard;
    }
};


ShenfenCard::ShenfenCard(){
    target_fixed = true;
}

void ShenfenCard::use(Room *room, ServerPlayer *shenlubu, const QList<ServerPlayer *> &) const{
    room->setPlayerMarkDelta(shenlubu, "@wrath", -6);

    QList<ServerPlayer *> players = room->getOtherPlayers(shenlubu);
    foreach(ServerPlayer *player, players){
        DamageStruct damage;
        damage.card = this;
        damage.from = shenlubu;
        damage.to = player;

        room->damage(damage);
    }

    foreach(ServerPlayer *player, players){
        player->throwAllEquips();
    }

    foreach(ServerPlayer *player, players){
        if(player->getHandcardNum() <= 4)
            player->throwAllHandCards();
        else
            room->askForDiscard(player, 4);
    }

    shenlubu->turnOver();
    room->broadcastProperty(shenlubu, "faceup");
}

void ShenfenCard::use(const QList<const ClientPlayer *> &targets) const{
    ClientInstance->turn_tag.insert("shenfen_used", true);
}

WuqianCard::WuqianCard(){

}

bool WuqianCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty();
}

void WuqianCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    room->setPlayerMarkDelta(effect.from, "@wrath", -2);
    room->acquireSkill(effect.from, "wushuang", false);
    effect.from->setFlags("wuqian_used");

    effect.to->setFlags("armor_nullified");
}

class WuqianViewAsSkill: public ZeroCardViewAsSkill{
public:
    WuqianViewAsSkill():ZeroCardViewAsSkill("wuqian"){

    }

    virtual bool isEnabledAtPlay() const{
        return Self->getMark("@wrath") >= 2;
    }

    virtual const Card *viewAs() const{
        return new WuqianCard;
    }
};

class Wuqian: public PhaseChangeSkill{
public:
    Wuqian():PhaseChangeSkill("wuqian"){
        view_as_skill = new WuqianViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *shenlubu) const{
        if(shenlubu->getPhase() == Player::Finish){
            Room *room = shenlubu->getRoom();
            if(shenlubu->hasFlag("wuqian_used")){
                shenlubu->setFlags("-wuqian_used");
                QList<ServerPlayer *> players = room->getAllPlayers();
                foreach(ServerPlayer *player, players){
                    if(player->hasFlag("armor_nullified"))
                        player->setFlags("-armor_nullified");
                }

                const General *general2 = shenlubu->getGeneral2();
                if(general2 == NULL || !general2->hasSkill("wushuang"))
                    shenlubu->loseSkill("wushuang");
            }
        }

        return false;
    }
};

WushenSlash::WushenSlash(Card::Suit suit, int number)
    :Slash(suit, number)
{

}

class Wushen: public FilterSkill{
public:
    Wushen():FilterSkill("wushen"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        WushenSlash *slash = new WushenSlash(card->getSuit(), card->getNumber());
        slash->addSubcard(card_item->getCard()->getId());
        slash->setSkillName(objectName());

        return slash;
    }
};

class QixingStart: public GameStartSkill{
public:
    QixingStart():GameStartSkill("#qixing"){

    }

    virtual void onGameStart(ServerPlayer *player) const{
        // FIXME:
    }
};

class Qixing: public PhaseChangeSkill{
public:
    Qixing():PhaseChangeSkill("qixing"){
        frequency = Frequent;
    }

    virtual int getPriority(ServerPlayer *target) const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getMark("@star") > 0;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Draw){

        }

        return false;
    }
};

KuangfengCard::KuangfengCard(){

}

bool KuangfengCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty();
}

void KuangfengCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->setPlayerMarkDelta(effect.to, "@gale", +1);
}

class KuangfengViewAsSkill: public ZeroCardViewAsSkill{
public:
    KuangfengViewAsSkill():ZeroCardViewAsSkill("kuangfeng"){

    }

    virtual bool isEnabledAtPlay() const{
        return Self->getMark("@star") > 0;
    }

    virtual const Card *viewAs() const{
        return new KuangfengCard;
    }
};

class Kuangfeng: public TriggerSkill{
public:
    Kuangfeng():TriggerSkill("kuangfeng"){
        view_as_skill = new KuangfengViewAsSkill;

        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@gale") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Fire){
            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

DawuCard::DawuCard(){

}

bool DawuCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.length() < Self->getMark("@star");
}

void DawuCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->setPlayerMarkDelta(effect.to, "@fog", +1);
}

class DawuViewAsSkill: public ZeroCardViewAsSkill{
public:
    DawuViewAsSkill():ZeroCardViewAsSkill("dawu"){

    }

    virtual bool isEnabledAtPlay() const{
        return Self->getMark("@star") > 0;
    }

    virtual const Card *viewAs() const{
        return new DawuCard;
    }
};

class Dawu: public TriggerSkill{
public:
    Dawu():TriggerSkill("dawu"){
        view_as_skill = new DawuViewAsSkill;

        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@dawu") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature != DamageStruct::Thunder)
            return true;
        else
            return false;
    }
};

GodPackage::GodPackage()
    :Package("god")
{
    t["god"] = tr("god");

    General *shenguanyu = new General(this, "shenguanyu", "god", 5);
    shenguanyu->addSkill(new Wushen);
    shenguanyu->addSkill(new Wuhun);
    shenguanyu->addSkill(new WuhunRevenge);

    General *shenlumeng = new General(this, "shenlumeng", "god", 3);
    shenlumeng->addSkill(new Shelie);
    shenlumeng->addSkill(new Gongxin);

    t["shenguanyu"] = tr("shenguanyu");
    t["shenlumeng"] = tr("shenlumeng");

    t["wushen"] = tr("wushen");
    t["wuhun"] = tr("wuhun");
    t["shelie"] = tr("shelie");
    t["gongxin"] = tr("gongxin");

    t[":wushen"] = tr(":wushen");
    t[":wuhun"] = tr(":wuhun");
    t[":shelie"] = tr(":shelie");
    t[":gongxin"] = tr(":gongxin");

    t["shelie:yes"] = tr("shelie:yes");
    t[":gongxin:"] = tr(":gongxin:");
    t["gongxin:discard"] = tr("gongxin:discard");
    t["gongxin:put"] = tr("gongxin:put");

    General *shenzhouyu = new General(this, "shenzhouyu", "god");
    shenzhouyu->addSkill(new Qinyin);
    shenzhouyu->addSkill(new GreatYeyan);
    shenzhouyu->addSkill(new MediumYeyan);
    shenzhouyu->addSkill(new SmallYeyan);

    General *shenzhugeliang = new General(this, "shenzhugeliang", "god", 3);
    shenzhugeliang->addSkill(new Qixing);
    shenzhugeliang->addSkill(new QixingStart);
    shenzhugeliang->addSkill(new Kuangfeng);
    shenzhugeliang->addSkill(new Dawu);

    t["shenzhouyu"] = tr("shenzhouyu");
    t["shenzhugeliang"] = tr("shenzhugeliang");

    t[":qinyin"] = tr(":qinyin");
    t[":greatyeyan"] = tr(":greatyeyan");
    t[":mediumyeyan"] = tr(":mediumyeyan");
    t[":smallyeyan"] = tr(":smallyeyan");
    t[":qixing"] = tr(":qixing");
    t[":kuangfeng"] = tr(":kuangfeng");
    t[":dawu"] = tr(":dawu");

    t["qinyin:yes"] = tr("qinyin:yes");
    t[":qinyin:"] = tr(":qinyin:");   
    t["qinyin:up"] = tr("qinyin:up");
    t["qinyin:down"] = tr("qinyin:down");
    t["greatyeyan"] = tr("greatyeyan");
    t["mediumyeyan"] = tr("mediumyeyan");
    t["smallyeyan"] = tr("smallyeyan");

    General *shencaocao = new General(this, "shencaocao$", "god", 3);
    shencaocao->addSkill(new Guixin);
    shencaocao->addSkill(new Feiying);

    General *shenlubu = new General(this, "shenlubu", "god", 5);
    shenlubu->addSkill(new Kuangbao);
    shenlubu->addSkill(new KuangbaoStart);
    shenlubu->addSkill(new Wumou);
    shenlubu->addSkill(new Wuqian);
    shenlubu->addSkill(new Shenfen);

    t["shencaocao"] = tr("shencaocao");
    t["shenlubu"] = tr("shenlubu");

    t["guixin"] = tr("guixin");
    t["feiying"] = tr("feiying");
    t["kuangbao"] = tr("kuangbao");
    t["wumou"] = tr("wumou");
    t["wuqian"] = tr("wuqian");
    t["shenfen"] = tr("shenfen");

    t[":guixin"] = tr(":guixin");
    t[":feiying"] = tr(":feiying");
    t[":kuangbao"] = tr(":kuangbao");
    t[":wumou"] = tr(":wumou");
    t[":wuqian"] = tr(":wuqian");
    t[":shenfen"] = tr(":shenfen");

    t["wumou:discard"] = tr("wumou:discard");
    t["wumou:losehp"] = tr("womou:losehp");

    t["#GetMark"] = tr("#GetMark");
    t["#LoseMark"] = tr("#LoseMark");

    // marks
    t["@nightmare"] = tr("@nightmare");
    t["@wrath"] = tr("@wrath");
    t["@star"] = tr("@star");
    t["@fog"] = tr("@fog");
    t["@gale"] = tr("@gale");

    addMetaObject<GongxinCard>();
    addMetaObject<GreatYeyanCard>();
    addMetaObject<ShenfenCard>();
    addMetaObject<GreatYeyanCard>();
    addMetaObject<MediumYeyanCard>();
    addMetaObject<SmallYeyanCard>();
    addMetaObject<WushenSlash>();
    addMetaObject<KuangfengCard>();
    addMetaObject<DawuCard>();
    addMetaObject<WuqianCard>();

    t["$guixin"]=tr("$guixin");
}

ADD_PACKAGE(God)
