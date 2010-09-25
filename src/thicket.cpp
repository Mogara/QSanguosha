#include "thicket.h"
#include "general.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"

class Xingshang: public GameStartSkill{
public:
    Xingshang():GameStartSkill("xingshang"){

    }

    virtual void onGameStart(ServerPlayer *player) const{
        player->getRoom()->setLegatee(player);
    }
};

FangzhuCard::FangzhuCard(){
}

void FangzhuCard::onEffect(const CardEffectStruct &effect) const{
    int x = effect.from->getLostHp();

    effect.to->drawCards(x);
    effect.to->turnOver();
    effect.to->getRoom()->setPlayerProperty(effect.to, "faceup", effect.to->faceUp());
}

class FangzhuViewAsSkill: public ZeroCardViewAsSkill{
public:
    FangzhuViewAsSkill():ZeroCardViewAsSkill("fangzhu"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@fangzhu";
    }

    virtual const Card *viewAs() const{
        return new FangzhuCard;
    }
};

class Fangzhu: public MasochismSkill{
public:
    Fangzhu():MasochismSkill("fangzhu"){
        view_as_skill = new FangzhuViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *caopi, const DamageStruct &damage) const{
        Room *room = caopi->getRoom();
        room->askForUseCard(caopi, "@@fangzhu", "@fangzhu");
    }
};

class Songwei: public TriggerSkill{
public:
    Songwei():TriggerSkill("songwei$"){
        events << JudgeOnEffect;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getGeneral()->getKingdom() == "wei" && !target->isLord();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardStar card = data.value<CardStar>();

        if(card->isBlack()){
            Room *room = player->getRoom();
            if(room->askForSkillInvoke(player, objectName())){
                room->getLord()->drawCards(1);
            }
        }

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

class Lieren: public TriggerSkill{
public:
    Lieren():TriggerSkill("lieren"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhurong, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card->inherits("Slash") && damage.to->isAlive() && !damage.to->isKongcheng() && damage.to != zhurong){            
            Room *room = zhurong->getRoom();
            if(room->askForSkillInvoke(zhurong, objectName()) && room->pindian(zhurong, damage.to) && !damage.to->isNude()){
                int card_id = room->askForCardChosen(zhurong, damage.to, "he", objectName());
                room->obtainCard(zhurong, card_id);
            }
        }

        return false;
    }
};

class Zaiqi: public PhaseChangeSkill{
public:
    Zaiqi():PhaseChangeSkill("zaiqi"){

    }

    virtual bool onPhaseChange(ServerPlayer *menghuo) const{
        if(menghuo->getPhase() == Player::Draw && menghuo->isWounded()){
            Room *room = menghuo->getRoom();
            if(room->askForSkillInvoke(menghuo, objectName())){
                int x = menghuo->getLostHp(), i;
                for(i=0; i<x; i++){
                    int card_id = room->drawCard();
                    room->throwCard(card_id);
                    room->getThread()->delay(1);

                    const Card *card = Sanguosha->getCard(card_id);
                    if(card->getSuit() == Card::Heart)
                        room->recover(menghuo);
                    else
                        room->obtainCard(menghuo, card_id);
                }

                return true;
            }
        }

        return false;
    }
};

class Juxiang: public GameStartSkill{
public:
    Juxiang():GameStartSkill("juxiang"){

    }

    virtual void onGameStart(ServerPlayer *player) const{
        player->getRoom()->setZhurong(player);
    }
};


YinghunCard::YinghunCard(){

}

void YinghunCard::onEffect(const CardEffectStruct &effect) const{
    int x = effect.from->getLostHp();
    Room *room = effect.from->getRoom();

    if(x == 1){
        effect.to->drawCards(1);
        room->askForDiscard(effect.to, 1, false, true);
    }else{
        QString choice = room->askForChoice(effect.from, "yinghun", "d1tx+dxt1");
        if(choice == "d1tx"){
            effect.to->drawCards(1);
            x = qMin(x, effect.to->getCardCount(true));
            room->askForDiscard(effect.to, x, false, true);
        }else{
            effect.to->drawCards(x);
            room->askForDiscard(effect.to, 1, false, true);
        }
    }
}

class YinghunViewAsSkill: public ZeroCardViewAsSkill{
public:
    YinghunViewAsSkill():ZeroCardViewAsSkill("yinghun"){
    }

    virtual const Card *viewAs() const{
        return new YinghunCard;        
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@yinghun";
    }
};

class Yinghun: public PhaseChangeSkill{
public:
    Yinghun():PhaseChangeSkill("yinghun"){
        view_as_skill = new YinghunViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *sunjian) const{
        if(sunjian->getPhase() == Player::Start && sunjian->isWounded()){
            Room *room = sunjian->getRoom();
            room->askForUseCard(sunjian, "@@yinghun", "@yinghun-card:" + sunjian->getGeneralName());
        }

        return false;
    }
};

HaoshiCard::HaoshiCard(){

}

bool HaoshiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    return to_select->getHandcardNum() == Self->getMark("haoshi");
}

void HaoshiCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);
}

class HaoshiViewAsSkill: public ViewAsSkill{
public:
    HaoshiViewAsSkill():ViewAsSkill("haoshi"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(to_select->isEquipped())
            return false;

        int length = Self->getHandcardNum() / 2;
        return selected.length() < length;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        HaoshiCard *card = new HaoshiCard;        
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@haoshi!";
    }
};

class Haoshi: public PhaseChangeSkill{
public:
    Haoshi():PhaseChangeSkill("haoshi"){
    }

    virtual int getPriority(ServerPlayer *) const{
        return -1;
    }

    virtual bool onPhaseChange(ServerPlayer *lusu) const{
        if(lusu->getPhase() == Player::Draw){
            Room *room = lusu->getRoom();
            if(room->askForSkillInvoke(lusu, objectName())){
                lusu->drawCards(2);

                if(lusu->getHandcardNum() > 5){
                    QList<ServerPlayer *> other_players = room->getOtherPlayers(lusu);
                    int least = 1000;
                    foreach(ServerPlayer *player, other_players)
                        least = qMin(player->getHandcardNum(), least);
                    room->setPlayerMark(lusu, "haoshi", least);
                    room->askForUseCard(lusu, "@@haoshi!", "@haoshi");
                }
            }
        }

        return false;
    }
};

DimengCard::DimengCard(){

}

bool DimengCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(to_select == Self)
        return false;

    if(targets.isEmpty())
        return true;

    if(targets.length() == 1){
        int max_diff = Self->getHandcardNum();
        return max_diff >= qAbs(to_select->getHandcardNum() - targets.first()->getHandcardNum());
    }

    return false;
}

bool DimengCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    return targets.length() == 2;
}

void DimengCard::use(const QList<const ClientPlayer *> &targets) const{
    ClientInstance->turn_tag.insert("dimeng_used", true);
}

void DimengCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *a = targets.at(0);
    ServerPlayer *b = targets.at(1);

    int n1 = a->getHandcardNum();
    int n2 = b->getHandcardNum();

    // make sure n1 >= n2
    if(n1 < n2){
        qSwap(a, b);
        qSwap(n1, n2);
    }

    int diff = n1 - n2;
    if(diff != 0){
        room->askForDiscard(source, diff, false);
    }

    DummyCard *card1 = a->wholeHandCards();
    DummyCard *card2 = b->wholeHandCards();

    room->moveCardTo(card1, b, Player::Hand, false);    
    room->getThread()->delay();
    room->moveCardTo(card2, a, Player::Hand, false);

    delete card1;
    delete card2;
}

class Dimeng: public ZeroCardViewAsSkill{
public:
    Dimeng():ZeroCardViewAsSkill("dimeng"){

    }

    virtual const Card *viewAs() const{
        return new DimengCard;
    }

    virtual bool isEnabledAtPlay() const{
        return !ClientInstance->turn_tag.value("dimeng_used", false).toBool();
    }
};

class Luanwu: public ZeroCardViewAsSkill{
public:
    Luanwu():ZeroCardViewAsSkill("luanwu"){

    }

    virtual const Card *viewAs() const{
        return new LuanwuCard;
    }

    virtual bool isEnabledAtPlay() const{
        return !ClientInstance->tag.value("luanwu_used", false).toBool();
    }
};

LuanwuCard::LuanwuCard(){
    target_fixed = true;
}

void LuanwuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<ServerPlayer *> players = room->getOtherPlayers(source);
    foreach(ServerPlayer *player, players){        
        room->cardEffect(this, source, player);
    }
}

void LuanwuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    QList<ServerPlayer *> players = room->getOtherPlayers(effect.to);
    QList<int> distance_list;
    int nearest = 1000;
    foreach(ServerPlayer *player, players){
        int distance = effect.to->distanceTo(player);
        distance_list << distance;

        nearest = qMin(nearest, distance);
    }    


    QList<ServerPlayer *> luanwu_targets;
    int i;
    for(i=0; i<distance_list.length(); i++){
        if(distance_list.at(i) == nearest && effect.to->canSlash(players.at(i))){
            luanwu_targets << players.at(i);
        }
    }

    const Card *slash = NULL;
    ServerPlayer *to_slash = NULL;
    if(!luanwu_targets.isEmpty() && (slash = room->askForCard(effect.to, "slash", "luanwu-slash"))
        && (to_slash = room->askForPlayerChosen(effect.to, luanwu_targets))) {
        room->cardEffect(slash, effect.to, to_slash);
    }else
        room->loseHp(effect.to);
}

void LuanwuCard::use(const QList<const ClientPlayer *> &) const{
    ClientInstance->tag.insert("luanwu_used", true);
}

class Jiuchi: public ViewAsSkill{
public:
    Jiuchi():ViewAsSkill("jiuchi"){
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern.contains("analeptic");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && !to_select->isEquipped() && to_select->getCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        const Card *first = cards.first()->getCard();
        Analeptic *analeptic = new Analeptic(first->getSuit(), first->getNumber());
        analeptic->addSubcard(first->getId());

        return analeptic;
    }
};

class Roulin: public TriggerSkill{
public:
    Roulin():TriggerSkill("roulin"){
        events << SlashProceed;

        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{        
        return target->hasSkill(objectName()) || target->getGeneral()->isFemale();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.from->hasSkill(objectName()) && effect.to->getGeneral()->isFemale()){
            // dongzhuo slash female
            ServerPlayer *dongzhuo = effect.from;
            ServerPlayer *female = effect.to;
            Room *room = dongzhuo->getRoom();

            bool jinked = false;
            const Card *jink = room->askForCard(female, "jink", "@rouli1-jink-1");
            if(jink && room->askForCard(female, "jink", "@rouli1-jink-2"))
                jinked = true;

            SlashResultStruct result;
            result.fill(effect, !jinked);
            room->slashResult(result);
            return true;

        }else if(effect.from->getGeneral()->isFemale() && effect.to->hasSkill(objectName())){
            // female slash dongzhuo

            ServerPlayer *female = effect.from;
            ServerPlayer *dongzhuo = effect.to;
            Room *room = female->getRoom();

            bool jinked = false;
            const Card *jink = room->askForCard(dongzhuo, "jink", "@roulin2-jink-1");
            if(jink && room->askForCard(dongzhuo, "jink", "@roulin2-jink-2"))
                jinked = true;

            SlashResultStruct result;
            result.fill(effect, !jinked);
            room->slashResult(result);

            return true;
        }

        return false;
    }
};

class Benghuai: public PhaseChangeSkill{
public:
    Benghuai():PhaseChangeSkill("benghuai"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *dongzhuo) const{
        bool trigger_this = false;
        Room *room = dongzhuo->getRoom();

        if(dongzhuo->getPhase() == Player::Finish){            
            QList<ServerPlayer *> players = room->getOtherPlayers(dongzhuo);
            foreach(ServerPlayer *player, players){
                if(dongzhuo->getHp() > player->getHp()){
                    trigger_this = true;
                    break;
                }
            }
        }

        if(trigger_this){
            QString result = room->askForChoice(dongzhuo, "benghuai", "hp+max_hp");

            if(result == "hp"){
                room->loseHp(dongzhuo);
            }else{
                dongzhuo->setMaxHP(dongzhuo->getMaxHP() - 1);
                if(dongzhuo->getMaxHP() == 0){
                    room->getThread()->trigger(Death, dongzhuo);
                }

                room->broadcastProperty(dongzhuo, "max_hp");
                room->broadcastProperty(dongzhuo, "hp");
            }
        }

        return false;
    }
};

class Baonue: public TriggerSkill{
public:
    Baonue():TriggerSkill("baonue$"){
        events << Damage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if(target->getGeneral()->getKingdom() != "qun")
            return false;

        if(target->isLord())
            return false;

        return target->getRoom()->getLord()->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *dongzhuo = room->getLord();
        if(dongzhuo->isWounded() && room->askForSkillInvoke(player, objectName())){
            const Card *card = room->getJudgeCard(player);
            if(card->getSuit() == Card::Spade){
                room->recover(dongzhuo);
            }
        }

        return false;
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
    menghuo->addSkill(new Zaiqi);

    zhurong = new General(this, "zhurong", "shu", 4, false);
    zhurong->addSkill(new Juxiang);
    zhurong->addSkill(new Lieren);

    sunjian = new General(this, "sunjian", "wu");
    sunjian->addSkill(new Yinghun);

    lusu = new General(this, "lusu", "wu", 3);
    lusu->addSkill(new Haoshi);
    lusu->addSkill(new Dimeng);

    jiaxu = new General(this, "jiaxu", "qun", 3);
    jiaxu->addSkill(new Skill("wansha", Skill::Compulsory));
    jiaxu->addSkill(new Skill("weimu", Skill::Compulsory));
    jiaxu->addSkill(new Luanwu);

    dongzhuo = new General(this, "dongzhuo$", "qun", 8);
    dongzhuo->addSkill(new Jiuchi);
    dongzhuo->addSkill(new Roulin);
    dongzhuo->addSkill(new Benghuai);
    dongzhuo->addSkill(new Baonue);

    t["thicket"] = tr("thicket");

    t["caopi"] = tr("caopi");
    t["xuhuang"] = tr("xuhuang");
    t["menghuo"] = tr("menghuo");
    t["zhurong"] = tr("zhurong");
    t["sunjian"] = tr("sunjian");
    t["lusu"] = tr("lusu");
    t["jiaxu"] = tr("jiaxu");
    t["dongzhuo"] = tr("dongzhuo");

    // skills
    t["xingshang"] = tr("xingshang");
    t["fangzhu"] = tr("fangzhu");
    t["songwei"] = tr("songwei");
    t["duanliang"] = tr("duanliang");
    t["huoshou"] = tr("huoshou");
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

    t[":xingshang"] = tr(":xingshang");
    t[":fangzhu"] = tr(":fangzhu");
    t[":songwei"] = tr(":songwei");
    t[":duanliang"] = tr(":duanliang");
    t[":huoshou"] = tr(":huoshou");
    t[":zaiqi"] = tr(":zaiqi");
    t[":juxiang"] = tr(":juxiang");
    t[":lieren"] = tr(":lieren");
    t[":yinghun"] = tr(":yinghun");
    t[":haoshi"] = tr(":haoshi");
    t[":dimeng"] = tr(":dimeng");
    t[":wansha"] = tr(":wansha");
    t[":weimu"] = tr(":weimu");
    t[":luanwu"] = tr(":luanwu");
    t[":jiuchi"] = tr(":jiuchi");
    t[":roulin"] = tr(":roulin");
    t[":benghuai"] = tr(":benghuai");
    t[":baonue"] = tr(":baonue");

    // skill descriptive texts
    t[":benghuai:"] = tr(":benghuai:");
    t["benghuai:hp"] = tr("benghuai:hp");
    t["benghuai:max_hp"] = tr("benghuai:max_hp");
    t["guixin:yes"] = tr("guixin:yes");
    t["baonue:yes"] = tr("baonue:yes");
    t["yinghun:d1tx"] = tr("yinghun:d1tx");
    t["yinghun:dxt1"] = tr("yinghun:dxt1");
    t["zaiqi:yes"] = tr("zaiqi:yes");
    t["lieren:yes"] = tr("lieren:yes");
    t["songwei:yes"] = tr("songwei:yes");
    t[":yinghun:"] = tr(":yinghun:");

    t["@roulin1-jink-1"] = tr("@roulin1-jink-1");
    t["@roulin1-jink-2"] = tr("@roulin1-jink-2");
    t["@roulin2-jink-1"] = tr("@roulin2-jink-1");
    t["@roulin2-jink-2"] = tr("@roulin2-jink-2");

    addMetaObject<DimengCard>();
    addMetaObject<LuanwuCard>();
    addMetaObject<YinghunCard>();
    addMetaObject<FangzhuCard>();
    addMetaObject<HaoshiCard>();

    skills << new HaoshiViewAsSkill;
}

ADD_PACKAGE(Thicket)
