#include "3d_zhimeng.h"
#include "general.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "general.h"

DujiCard::DujiCard(){
    target_fixed = true;
}

void DujiCard::use(Room *, ServerPlayer *player, const QList<ServerPlayer *> &) const{
    player->addToPile("du", getSubcards().first());
}

class DujiViewAsSkill: public OneCardViewAsSkill{
public:
    DujiViewAsSkill():OneCardViewAsSkill("duji"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getPile("du").isEmpty();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new DujiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Duji: public TriggerSkill{
public:
    Duji():TriggerSkill("duji"){
        view_as_skill = new DujiViewAsSkill;
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *i) const{
        return !i->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::Play)
            return false;

        CardUseStruct use = data.value<CardUseStruct>();
        ServerPlayer *liru = room->findPlayerBySkillName(objectName());
        if(!liru || liru->getPile("du").isEmpty())
            return false;

        if(use.card->inherits("Slash") && !player->hasFlag("drank") && liru->askForSkillInvoke(objectName())){
            room->obtainCard(player, liru->getPile("du").first());
            room->setPlayerFlag(player, "drank");
            if(room->askForChoice(player, objectName(), "turn+lp") == "turn")
                player->turnOver();
            else
                room->loseHp(player);
        }
        return false;
    }
};

class ShiPo: public ProhibitSkill{
public:
    ShiPo():ProhibitSkill("shiPo"){

    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return (card->inherits("Slash") && card->getSuit() == Card::Spade) ||
                (card->inherits("TrickCard") && card->getSuit() == Card::Spade);
    }
};

class Liefu: public TriggerSkill{
public:
    Liefu(): TriggerSkill("liefu"){
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(player->askForSkillInvoke(objectName(), data)){
            LogMessage log;
            log.type = "#Liefu";
            log.from = player;
            log.to << effect.to;
            log.arg = objectName();
            room->sendLog(log);

            room->slashResult(effect, NULL);
            if(room->askForChoice(player, objectName(), "pan+feng") == "pan")
                room->askForDiscard(player, objectName(), qMin(player->getCardCount(true), effect.to->getLostHp()), false, true);
            else
                effect.to->drawCards(qMin(effect.to->getHp(), 5));
        }

        return false;
    }
};

PengriCard::PengriCard(){
}

bool PengriCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->isNude() && to_select->canSlash(Self, false);
}

void PengriCard::use(Room *room, ServerPlayer *player, const QList<ServerPlayer *> &tar) const{
    int card_id = room->askForCardChosen(player, tar.first(), "he", "pengri");
    room->obtainCard(player, card_id, room->getCardPlace(card_id) != Player::Hand);

    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("pengri");
    CardUseStruct use;
    use.card = slash;
    use.from = tar.first();
    use.to << player;
    room->useCard(use);
}

class Pengri:public ZeroCardViewAsSkill{
public:
    Pengri():ZeroCardViewAsSkill("pengri"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("PengriCard");
    }

    virtual const Card *viewAs() const{
        return new PengriCard;
    }
};

class Gangli:public MasochismSkill{
public:
    Gangli():MasochismSkill("gangli"){
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        if(damage.damage != 1 || !damage.from || damage.from == player)
            return;

        if(room->askForSkillInvoke(player, objectName(), QVariant::fromValue(damage))){
            ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(damage.from), objectName());
            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName(objectName());
            CardUseStruct use;
            use.card = slash;
            use.from = damage.from;
            use.to << target;
            room->useCard(use, false);
        }
    }
};

class Yaliang: public TriggerSkill{
public:
    Yaliang():TriggerSkill("yaliang"){
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.to == effect.from)
            return false;

        if(effect.card->isNDTrick()){
            if(player->askForSkillInvoke(objectName(), data)){
                player->drawCards(1);
                LogMessage log;
                log.type = "#Yaliang";
                log.from = effect.to;
                log.to << effect.from;
                log.arg = effect.card->objectName();
                log.arg2 = objectName();
                room->sendLog(log);

                room->playSkillEffect(objectName());
                Slash *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName(objectName());
                CardUseStruct use;
                use.card = slash;
                use.from = effect.from;
                use.to << player;
                room->useCard(use, false);

                return true;
            }
        }
        return false;
    }
};

XunguiCard::XunguiCard(){
    target_fixed = true;
}

void XunguiCard::use(Room *room, ServerPlayer *player, const QList<ServerPlayer *> &) const{
    if(!player->getPile("gui").isEmpty()){
        room->throwCard(player->getPile("gui").first());
        if(player->isWounded()){
            RecoverStruct rev;
            room->recover(player, rev);
        }
    }
    player->addToPile("gui", getSubcards().first());
}

class Xungui: public OneCardViewAsSkill{
public:
    Xungui():OneCardViewAsSkill("xungui"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("XunguiCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isNDTrick();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new XunguiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

DaojuCard::DaojuCard(){
}

bool DaojuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    CardStar card = Sanguosha->getCard(Self->getPile("gui").first());
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card);
}

bool DaojuCard::targetFixed() const{
    CardStar card = Sanguosha->getCard(Self->getPile("gui").first());
    return card && card->targetFixed();
}

bool DaojuCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    CardStar card = Sanguosha->getCard(Self->getPile("gui").first());
    return card && card->targetsFeasible(targets, Self);
}

void DaojuCard::onUse(Room *room, const CardUseStruct &card_use) const{
    CardStar ju = Sanguosha->getCard(card_use.from->getPile("gui").first());

    const Card *first = Sanguosha->getCard(getSubcards().first());
    const Card *second = Sanguosha->getCard(getSubcards().last());
    Card::Suit suit = Card::NoSuit;
    if(first->isBlack() && second->isBlack())
        suit = Card::Spade;
    else if(first->isRed() && second->isRed())
        suit = Card::Heart;
    Card *new_card = Sanguosha->cloneCard(ju->objectName(), suit, (first->getNumber() + second->getNumber()) / 2);
    new_card->setSkillName("daoju");
    new_card->addSubcard(first);
    new_card->addSubcard(second);

    CardUseStruct use;
    use.card = new_card;
    use.from = card_use.from;
    use.to = card_use.to;
    room->useCard(use);
}

class Daoju: public ViewAsSkill{
public:
    Daoju():ViewAsSkill("daoju"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DaojuCard") && !player->getPile("gui").isEmpty();
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return true;
        else if(selected.length() == 1){
            Card::Color color = selected.first()->getFilteredCard()->getColor();
            return to_select->getFilteredCard()->getColor() == color;
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            DaojuCard *card = new DaojuCard();
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return player->getPhase() == Player::Play &&
                !player->getPile("gui").isEmpty() &&
                Sanguosha->getCard(player->getPile("gui").first())->objectName() == "nullification" &&
                pattern == "nullification";
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        if(player->getPhase() != Player::Play || player->getPile("gui").isEmpty())
            return false;
        const Card *null = Sanguosha->getCard(player->getPile("gui").first());
        if(null->objectName() == "nullification" && !player->hasUsed("DaojuCard"))
            return true;
        else
            return false;
    }
};

class Xiandeng: public DrawCardsSkill{
public:
    Xiandeng():DrawCardsSkill("xiandeng"){

    }

    virtual int getDrawNum(ServerPlayer *play2, int n) const{
        Room *room = play2->getRoom();
        if(room->askForSkillInvoke(play2, objectName())){
            room->playSkillEffect(objectName());
            ServerPlayer *target = room->askForPlayerChosen(play2, room->getOtherPlayers(play2), objectName());
            room->setFixedDistance(play2, target, 0);
            play2->setFlags(objectName());
            play2->tag["XD"] = QVariant::fromValue((PlayerStar)target);
            return n - 1;
        }else
            return n;
    }
};

class XDClear: public PhaseChangeSkill{
public:
    XDClear():PhaseChangeSkill("#xdclear"){
    }

    virtual bool onPhaseChange(ServerPlayer *yuejin) const{
        if(yuejin->getPhase() == Player::NotActive){
            PlayerStar target = yuejin->tag["XD"].value<PlayerStar>();
            if(target){
                yuejin->getRoom()->setFixedDistance(yuejin, target, -1);
                yuejin->tag.remove("XD");
            }
        }
        return false;
    }
};

class Xia0guo: public TriggerSkill{
public:
    Xia0guo():TriggerSkill("xia0guo"){
        events << SlashMissed;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash->getSuit() != Card::Heart && !effect.jink->isVirtualCard()){
            if(player->askForSkillInvoke(objectName(), data)){
                room->playSkillEffect(objectName());
                effect.to->obtainCard(effect.jink);

                QString name = effect.slash->isBlack() ? "supply_shortage" : "indulgence";
                Card *new_card = Sanguosha->cloneCard(name, effect.slash->getSuit(), effect.slash->getNumber());
                new_card->setSkillName("xia0guo");
                new_card->addSubcard(effect.slash);

                if(!effect.from->isProhibited(effect.to, new_card) && !effect.to->containsTrick(name)){
                    CardUseStruct use;
                    use.card = new_card;
                    use.from = effect.from;
                    use.to << effect.to;
                    room->useCard(use);
                }
            }
        }
        return false;
    }
};

class Jingrui:public OneCardViewAsSkill{
public:
    Jingrui():OneCardViewAsSkill("jingrui"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getHandcardNum() >= player->getHp() && Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return player->getHandcardNum() >= player->getHp() &&
                (pattern == "jink" || pattern == "slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        bool clone_jink = false;
        if(ClientInstance->getStatus() == Client::Responsing)
            clone_jink = ClientInstance->getPattern() == "jink";
        if(clone_jink){
            Jink *jink = new Jink(card->getSuit(), card->getNumber());
            jink->addSubcard(card);
            jink->setSkillName(objectName());
            return jink;
        }
        else{
            Slash *slash = new Slash(card->getSuit(), card->getNumber());
            slash->addSubcard(card);
            slash->setSkillName(objectName());
            return slash;
        }
    }
};

Zha0xinCard::Zha0xinCard(){
    target_fixed = true;
}

void Zha0xinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &tar) const{
    room->showAllCards(source);
    int samecount = 0, unsamecount = 0;
    foreach(const Card *card1, source->getHandcards()){
        foreach(const Card *card2, source->getHandcards()){
            if(card1 == card2)
                continue;
            if(card1->getSuit() == card2->getSuit())
                samecount ++;
            else
                unsamecount ++;
        }
    }
    if(samecount == 0){
        ServerPlayer *i = room->askForPlayerChosen(source, room->getAlivePlayers(), "zha0xin");
        room->loseHp(i);
    }
    else if(unsamecount == 0){
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *i, room->getOtherPlayers(source))
            if(!i->isNude())
                targets << i;
        if(!targets.isEmpty()){
            ServerPlayer *t = room->askForPlayerChosen(source, targets, "zha0xin");
            int card_id = room->askForCardChosen(source, t, "he", "zha0xin");
            room->obtainCard(source, card_id, room->getCardPlace(card_id) != Player::Hand);
        }
    }
}

class Zha0xin: public ZeroCardViewAsSkill{
public:
    Zha0xin():ZeroCardViewAsSkill("zha0xin"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->getHandcardNum() < player->getHp())
            return false;
        return !player->hasUsed("Zha0xinCard");
    }

    virtual const Card *viewAs() const{
        return new Zha0xinCard;
    }
};

class Huaiyi: public TriggerSkill{
public:
    Huaiyi():TriggerSkill("huaiyi"){
        events << HpChanged;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, Room*, ServerPlayer *player, QVariant &data) const{
        if(player->askForSkillInvoke(objectName()))
            player->drawCards(1);
        return false;
    }
};

class Yinsi: public OneCardViewAsSkill{
public:
    Yinsi():OneCardViewAsSkill("yinsi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Analeptic::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("analeptic");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("EquipCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Analeptic *analeptic = new Analeptic(card->getSuit(), card->getNumber());
        analeptic->setSkillName(objectName());
        analeptic->addSubcard(card->getId());

        return analeptic;
    }
};

ChanxianCard::ChanxianCard(){
    once = true;
    will_throw = false;
}

bool ChanxianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void ChanxianCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);

    Room *room = effect.to->getRoom();
    QString choice = room->askForChoice(effect.to, "chanxian", "slash+hsals");
    if(choice == "slash"){
        QList<ServerPlayer *> players = room->getOtherPlayers(effect.to), targets;
        foreach(ServerPlayer *player, players){
            if(effect.to->canSlash(player) && player != effect.from)
                targets << player;
        }
        if(!targets.isEmpty()){
            ServerPlayer *target = room->askForPlayerChosen(effect.from, targets, "chanxian");
            const Card *slash = room->askForCard(effect.to, "slash", "@chanxian:" + target->objectName(), QVariant(), NonTrigger);
            if(slash){
                room->cardEffect(slash, effect.to, target);
                return;
            }
            else
                choice = "hsals";
        }
        else
            choice = "hsals";
    }
    if(choice == "hsals"){
        choice = room->askForChoice(effect.from, "chanxian", "get+hit");
        if(choice == "get"){
            int card_id = room->askForCardChosen(effect.from, effect.to, "he", "chanxian");
            room->obtainCard(effect.from, card_id, room->getCardPlace(card_id) != Player::Hand);
        }
        else{
            DamageStruct damage;
            damage.from = effect.from;
            damage.to = effect.to;
            room->damage(damage);
        }
    }
}

class Chanxian: public OneCardViewAsSkill{
public:
    Chanxian():OneCardViewAsSkill("chanxian"){
        default_choice = "slash";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ChanxianCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Diamond;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ChanxianCard *card = new ChanxianCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Yanhe:public PhaseChangeSkill{
public:
    Yanhe():PhaseChangeSkill("yanhe"){
    }

    virtual bool onPhaseChange(ServerPlayer *zhugejin) const{
        if(zhugejin->getPhase() == Player::Start &&
           zhugejin->isWounded() &&
           zhugejin->askForSkillInvoke(objectName())){
            Room *room = zhugejin->getRoom();
            room->playSkillEffect(objectName());
            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *player, room->getOtherPlayers(zhugejin)){
                if(player->hasEquip())
                    targets << player;
            }
            if(!targets.isEmpty()){
                ServerPlayer *target = room->askForPlayerChosen(zhugejin, targets, "yanhe");
                for(int i = 0; i < zhugejin->getLostHp(); i ++){
                    int card_id = room->askForCardChosen(zhugejin, target, "e", "yanhe");
                    room->obtainCard(target, card_id);
                    if(!target->hasEquip())
                        break;
                }
            }
        }
        return false;
    }
};

class Youqi: public PhaseChangeSkill{
public:
    Youqi():PhaseChangeSkill("youqi"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("youqi") == 0
                && target->getPhase() == Player::Judge
                && target->getHp() == 1;
    }

    virtual bool onPhaseChange(ServerPlayer *sunce) const{
        Room *room = sunce->getRoom();

        LogMessage log;
        log.type = "#YouqiWake";
        log.from = sunce;
        log.arg = objectName();
        room->sendLog(log);

        room->playSkillEffect(objectName());
        //room->broadcastInvoke("animate", "lightbox:$youqi:5000");
        //room->getThread()->delay(5000);

        room->loseMaxHp(sunce);

        room->acquireSkill(sunce, "dimeng");
        room->acquireSkill(sunce, "kongcheng");

        room->setPlayerMark(sunce, "youqi", 1);

        return false;
    }
};

QuanjianCard::QuanjianCard(){
    once = true;
    will_throw = false;
}

bool QuanjianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void QuanjianCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);

    Room *room = effect.to->getRoom();
    const Card *card = effect.to->getRandomHandCard();
    room->showCard(effect.to, card->getEffectiveId());
    if(card->inherits("Jink")){
        effect.from->drawCards(1);
        effect.to->drawCards(1);
    }
}

class Quanjian: public OneCardViewAsSkill{
public:
    Quanjian():OneCardViewAsSkill("quanjian"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QuanjianCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Jink");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QuanjianCard *card = new QuanjianCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

SijieCard::SijieCard(){

}

bool SijieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return !to_select->isNude();
}

void SijieCard::onEffect(const CardEffectStruct &effect) const{
    int discard = qMax(1, effect.to->getLostHp());
    int all = effect.to->getCardCount(true);
    effect.from->getRoom()->askForDiscard(effect.to, "sijie", qMin(discard, all), false, true);
}

class SijieViewAsSkill: public ZeroCardViewAsSkill{
public:
    SijieViewAsSkill():ZeroCardViewAsSkill("sijie"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@sijie";
    }

    virtual const Card *viewAs() const{
        return new SijieCard;
    }
};

class Sijie: public MasochismSkill{
public:
    Sijie():MasochismSkill("sijie"){
        view_as_skill = new SijieViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *jushou, const DamageStruct &damage) const{
        Room *room = jushou->getRoom();
        int x = damage.damage, i;
        for(i=0; i<x; i++){
            if(!room->askForUseCard(jushou, "@@sijie", "@sijie"))
                break;
        }
    }
};

SanDZhimengPackage::SanDZhimengPackage()
    :Package("sand_zhimeng")
{
    General *diyliru = new General(this, "diyliru", "qun", 3);
    diyliru->addSkill(new Duji);
    diyliru->addSkill(new ShiPo);

    General *diypanfeng = new General(this, "diypanfeng", "qun");
    diypanfeng->addSkill(new Liefu);

    General *diychengyu = new General(this, "diychengyu", "wei", 3);
    diychengyu->addSkill(new Pengri);
    diychengyu->addSkill(new Gangli);

    General *diyjiangwan = new General(this, "diyjiangwan", "shu", 3);
    diyjiangwan->addSkill(new Yaliang);
    diyjiangwan->addSkill(new Xungui);
    diyjiangwan->addSkill(new Daoju);

    General *diyyuejin = new General(this, "diyyuejin", "wei");
    diyyuejin->addSkill(new Xiandeng);
    diyyuejin->addSkill(new XDClear);
    related_skills.insertMulti("xiandeng", "#xdclear");
    diyyuejin->addSkill(new Xia0guo);

    General *diychendao = new General(this, "diychendao", "shu");
    diychendao->addSkill(new Jingrui);

    General *diysimazhao = new General(this, "diysimazhao", "wei", 3);
    diysimazhao->addSkill(new Zha0xin);
    diysimazhao->addSkill(new Huaiyi);

    General *diysunluban = new General(this, "diysunluban", "wu", 3, false);
    diysunluban->addSkill(new Yinsi);
    diysunluban->addSkill(new Chanxian);

    General *diyzhugejin = new General(this, "diyzhugejin", "wu");
    diyzhugejin->addSkill(new Yanhe);
    diyzhugejin->addSkill(new Youqi);

    General *diyjushou = new General(this, "diyjushou", "qun", 3);
    diyjushou->addSkill(new Quanjian);
    diyjushou->addSkill(new Sijie);

    addMetaObject<DujiCard>();
    addMetaObject<PengriCard>();
    addMetaObject<XunguiCard>();
    addMetaObject<DaojuCard>();
    addMetaObject<Zha0xinCard>();
    addMetaObject<ChanxianCard>();
    addMetaObject<QuanjianCard>();
    addMetaObject<SijieCard>();
}

ADD_PACKAGE(SanDZhimeng)
