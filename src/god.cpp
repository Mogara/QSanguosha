#include "god.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"

GongxinCard::GongxinCard(){

}

bool GongxinCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
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
        events << Death;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        // FIXME:

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
            room->takeAG(shenlumeng, card_id);

            const Card *card = Sanguosha->getCard(card_id);

            // quick-and-dirty
            shenlumeng->addCard(card, Player::Hand);
            room->setCardMapping(card_id, shenlumeng, Player::Hand);

            LogMessage log;
            log.type = "$TakeAG";
            log.from = shenlumeng;
            log.card_str = QString::number(card_id);
            room->sendLog(log);

            Card::Suit suit = card->getSuit();
            card_ids.removeOne(card_id);
            QMutableListIterator<int> itor(card_ids);
            while(itor.hasNext()){
                const Card *c = Sanguosha->getCard(itor.next());
                if(c->getSuit() == suit){
                    itor.remove();

                    room->setCardMapping(c->getId(), NULL, Player::DiscardedPile);
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
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        int discard_num = data.toInt();

        if(player->getPhase() == Player::Discard && discard_num >= 2){
            Room *room = player->getRoom();
            QString result = room->askForChoice(player, objectName(), "up+down+no");
            if(result == "no"){
                return false;
            }

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

class Feiying: public GameStartSkill{
public:
    Feiying():GameStartSkill("feiying"){
        frequency = Compulsory;
    }

    virtual void onGameStart(ServerPlayer *player) const{
        player->getRoom()->setPlayerCorrect(player, "F");
    }
};

class Baonu: public TriggerSkill{
public:
    Baonu():TriggerSkill("baonu"){
        events << GameStart<< Damage << Damaged;
    }

    virtual int getPriority(ServerPlayer *target) const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == GameStart){
            player->setMark("anger", 2);
        }else{
            if(player->isAlive()){
                DamageStruct damage = data.value<DamageStruct>();
                int value = player->getMark("anger");
                value += damage.damage;
                player->getRoom()->setPlayerMark(player, "anger", value);
            }
        }

        return false;
    }
};

class Wumo: public TriggerSkill{
public:
    Wumo():TriggerSkill("wumo"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("TrickCard") && !use.card->inherits("DelayedTrick")){
            Room *room = player->getRoom();
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
        return Self->getMark("baonu") >= 6;
    }

    virtual const Card *viewAs() const{
        return new ShenfenCard;
    }
};

ShenfenCard::ShenfenCard(){
    target_fixed = true;
}

void ShenfenCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->turnOver();
    room->broadcastProperty(source, "faceup");

    int value = source->getMark("baonu");
    value -= 6;
    room->setPlayerMark(source, "baonu", value);

    QList<ServerPlayer *> players = room->getOtherPlayers(source);

    foreach(ServerPlayer *player, players){
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        damage.from = source;
        damage.to = player;
        damage.nature = DamageStruct::Normal;

        room->damage(damage);
    }

    foreach(ServerPlayer *player, players){
        player->throwAllEquips();
    }

    foreach(ServerPlayer *player, players){
        int discard_num = qMin(player->getHandcardNum(), 4);
        room->askForDiscard(player, discard_num);
    }
}

GodPackage::GodPackage()
    :Package("god")
{
    t["god"] = tr("god");

    General *shenguanyu, *shenlumeng;

    shenguanyu = new General(this, "shenguanyu", "shu", 5);
    shenguanyu->addSkill(new Wuhun);

    shenlumeng = new General(this, "shenlumeng", "wu", 3);
    shenlumeng->addSkill(new Shelie);
    shenlumeng->addSkill(new Gongxin);

    t["shenguanyu"] = tr("shenguanyu");
    t["shenlumeng"] = tr("shenlumeng");

    t["wuhun"] = tr("wuhun");
    t["shelie"] = tr("shelie");
    t["gongxin"] = tr("gongxin");

    t[":wuhun"] = tr(":wuhun");
    t[":shelie"] = tr(":shelie");
    t[":gongxin"] = tr(":gongxin");

    t["shelie:yes"] = tr("shelie:yes");
    t[":gongxin:"] = tr(":gongxin:");
    t["gongxin:discard"] = tr("gongxin:discard");
    t["gongxin:put"] = tr("gongxin:put");

    General *shenzhouyu, *shenzhugeliang;

    shenzhouyu = new General(this, "shenzhouyu", "wu");
    shenzhouyu->addSkill(new Qinyin);
    shenzhouyu->addSkill(new GreatYeyan);
    shenzhouyu->addSkill(new MediumYeyan);
    shenzhouyu->addSkill(new SmallYeyan);

    shenzhugeliang = new General(this, "shenzhugeliang", "shu", 3);

    t["shenzhouyu"] = tr("shenzhouyu");
    t["shenzhugeliang"] = tr("shenzhugeliang");

    t[":qinyin"] = tr(":qinyin");
    t[":greatyeyan"] = tr(":greatyeyan");
    t[":mediumyeyan"] = tr(":mediumyeyan");
    t[":smallyeyan"] = tr(":smallyeyan");
    t[":qixing"] = tr(":qixing");
    t[":kuangfeng"] = tr(":kuangfeng");
    t[":dawu"] = tr(":dawu");

    t[":qinyin:"] = tr(":qinyin:");
    t["qinyin:up"] = tr("qinyin:up");
    t["qinyin:down"] = tr("qinyin:down");
    t["qinyin:no"] = tr("qinyin:no");
    t["greatyeyan"] = tr("greatyeyan");
    t["mediumyeyan"] = tr("mediumyeyan");
    t["smallyeyan"] = tr("smallyeyan");

    General *shencaocao, *shenlubu;

    shencaocao = new General(this, "shencaocao$", "wei", 3);
    shencaocao->addSkill(new Guixin);
    shencaocao->addSkill(new Feiying);

    shenlubu = new General(this, "shenlubu", "qun", 5);
    shenlubu->addSkill(new Baonu);
    shenlubu->addSkill(new Shenfen);

    t["shencaocao"] = tr("shencaocao");
    t["shenlubu"] = tr("shenlubu");

    t["guixin"] = tr("guixin");
    t["feiying"] = tr("feiying");
    t["baonu"] = tr("baonu");
    t["wumou"] = tr("wumou");
    t["wuqian"] = tr("wuqian");
    t["shenfen"] = tr("shenfen");

    t[":guixin"] = tr(":guixin");
    t[":feiying"] = tr(":feiying");
    t[":baonu"] = tr(":baonu");
    t[":wumou"] = tr(":wumou");
    t[":wuqian"] = tr(":wuqian");
    t[":shenfen"] = tr(":shenfen");

    addMetaObject<GongxinCard>();
    addMetaObject<GreatYeyanCard>();
    addMetaObject<ShenfenCard>();
    addMetaObject<GreatYeyanCard>();
    addMetaObject<MediumYeyanCard>();
    addMetaObject<SmallYeyanCard>();
}

ADD_PACKAGE(God)
