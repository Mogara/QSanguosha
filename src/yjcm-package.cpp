#include "yjcm-package.h"
#include "skill.h"
#include "standard.h"
#include "maneuvering.h"
#include "client.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"

class Yizhong: public TriggerSkill{
public:
    Yizhong():TriggerSkill("yizhong"){
        events << SlashEffected;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getArmor())
            return false;

        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(effect.slash->isBlack()){
            LogMessage log;
            log.type = "#SkillNullify";
            log.from = player;
            log.arg = objectName();
            log.arg2 = effect.slash->objectName();

            player->getRoom()->sendLog(log);

            return true;
        }

        return false;
    }
};

class Luoying: public TriggerSkill{
public:
    Luoying():TriggerSkill("luoying"){
        events << CardDiscarded << CardUsed;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return ! target->hasSkill(objectName());
    }

    QList<const Card *> getClubs(const Card *card) const{
        QList<const Card *> clubs;

        if(!card->isVirtualCard()){
            if(card->getSuit() == Card::Club)
                clubs << card;

            return clubs;
        }

        foreach(int card_id, card->getSubcards()){
            const Card *c = Sanguosha->getCard(card_id);
            if(c->getSuit() == Card::Club)
                clubs << c;
        }

        return clubs;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        QList<const Card *> clubs;

        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            const SkillCard *skill_card = qobject_cast<const SkillCard *>(use.card);
            if(skill_card && skill_card->subcardsLength() > 0 && skill_card->willThrow()){
                clubs = getClubs(skill_card);
            }
        }else if(event == CardDiscarded){
            const Card *card = data.value<CardStar>();
            if(card->subcardsLength() == 0)
                return false;

            clubs = getClubs(card);
        }

        if(clubs.isEmpty())
            return false;

        ServerPlayer *caozhi = room->findPlayerBySkillName(objectName());
        if(caozhi && caozhi->askForSkillInvoke(objectName(), data)){
            foreach(const Card *club, clubs)
                caozhi->obtainCard(club);
        }

        return false;
    }
};

class Jiushi: public ZeroCardViewAsSkill{
public:
    Jiushi():ZeroCardViewAsSkill("jiushi"){

    }

    virtual bool isEnabledAtPlay() const{
        return Analeptic::IsAvailable() && Self->faceUp();
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern.contains("analeptic") && Self->faceUp();
    }

    virtual const Card *viewAs() const{
        Analeptic *analeptic = new Analeptic(Card::NoSuit, 0);
        analeptic->setSkillName("jiushi");
        return analeptic;
    }
};

class JiushiFlip: public TriggerSkill{
public:
    JiushiFlip():TriggerSkill("#jiushi-flip"){
        events << CardUsed << Damaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->getSkillName() == "jiushi")
                player->turnOver();
        }else if(event == Damaged){
            if(!player->faceUp() && player->askForSkillInvoke("jiushi"))
                player->turnOver();
        }

        return false;
    }
};

class Wuyan: public TriggerSkill{
public:
    Wuyan():TriggerSkill("wuyan"){
        events << CardEffect << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.to == effect.from)
            return false;

        if(effect.card->getTypeId() == Card::Trick){
            Room *room = player->getRoom();

            if((effect.from && effect.from->hasSkill(objectName()))){
                LogMessage log;
                log.type = "#WuyanBad";
                log.from = effect.from;
                log.to << effect.to;
                log.arg = effect.card->objectName();

                room->sendLog(log);

                return true;
            }

            if(effect.to->hasSkill(objectName()) && effect.from){
                LogMessage log;
                log.type = "#WuyanGood";
                log.from = effect.to;
                log.to << effect.from;
                log.arg = effect.card->objectName();

                room->sendLog(log);

                return true;
            }
        }

        return false;
    }
};

JujianCard::JujianCard(){
    once = true;
}

void JujianCard::onEffect(const CardEffectStruct &effect) const{
    int n = effect.card->subcardsLength();
    effect.to->drawCards(n);

    if(n == 3){
        QSet<Card::CardType> types;

        foreach(int card_id, effect.card->getSubcards()){
            const Card *card = Sanguosha->getCard(card_id);
            types << card->getTypeId();
        }

        if(types.size() == 1){
            RecoverStruct recover;
            recover.card = this;
            recover.who = effect.from;
            effect.from->getRoom()->recover(effect.from, recover);
        }
    }
}

class Jujian: public ViewAsSkill{
public:
    Jujian():ViewAsSkill("jujian"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 3 && !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay() const{
        return ! ClientInstance->hasUsed("JujianCard");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        JujianCard *card = new JujianCard;
        card->addSubcards(cards);
        return card;
    }
};

class Enyuan: public TriggerSkill{
public:
    Enyuan():TriggerSkill("enyuan"){
        events << HpRecover << Damaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(event == HpRecover){
            RecoverStruct recover = data.value<RecoverStruct>();
            if(recover.who){
                recover.who->drawCards(recover.recover);

                LogMessage log;
                log.type = "#EnyuanRecover";
                log.from = player;
                log.to << recover.who;
                log.arg = recover.recover;

                room->sendLog(log);
            }
        }else if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            ServerPlayer *source = damage.from;
            if(source){
                const Card *card = room->askForCard(source, ".H", "@enyuan", false);
                if(card){
                    room->showCard(source, card->getEffectiveId());
                    player->obtainCard(card);
                }else{
                    room->loseHp(source);
                }
            }
        }

        return false;
    }
};

class Huilei: public TriggerSkill{
public:
    Huilei():TriggerSkill("huilei"){
        events << Death;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        QString killer_name = data.toString();
        if(!killer_name.isEmpty()){
            Room *room = player->getRoom();
            ServerPlayer *killer = room->findChild<ServerPlayer *>(killer_name);
            killer->throwAllEquips();
            killer->throwAllHandCards();
        }

        return false;
    }
};

class Xuanfeng: public TriggerSkill{
public:
    Xuanfeng():TriggerSkill("xuanfeng"){
        events << CardLost;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardMoveStruct move = data.value<CardMoveStruct>();

        if(move.from_place == Player::Equip && player->askForSkillInvoke(objectName(), data)){
            Room *room = player->getRoom();

            QList<ServerPlayer *> players = room->getOtherPlayers(player);

            ServerPlayer *target = room->askForPlayerChosen(player, players, objectName());

            CardEffectStruct effect;
            effect.card = new Slash(Card::NoSuit, 0);
            effect.from = player;
            effect.to = target;

            room->cardEffect(effect);
        }

        return false;
    }
};

class Pojun: public TriggerSkill{
public:
    Pojun():TriggerSkill("pojun"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->askForSkillInvoke(objectName(), data)){
            DamageStruct damage = data.value<DamageStruct>();

            damage.to->drawCards(damage.to->getLostHp());
            damage.to->turnOver();
        }

        return false;
    }
};

XianzhenCard::XianzhenCard(){
    once = true;
}

void XianzhenCard::onEffect(const CardEffectStruct &effect) const{

}

class Xianzhen: public OneCardViewAsSkill{
public:
    Xianzhen():OneCardViewAsSkill("xianzhen"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! ClientInstance->hasUsed("XianzhenCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        XianzhenCard *card = new XianzhenCard;
        card->addSubcard(card_item->getCard());

        return card;
    }
};

class Jiejiu: public FilterSkill{
public:
    Jiejiu():FilterSkill("jiejiu"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->objectName() == "analeptic";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *c = card_item->getCard();
        Slash *slash = new Slash(c->getSuit(), c->getNumber());
        slash->setSkillName(objectName());
        slash->addSubcard(card_item->getCard());

        return slash;
    }
};

MingceCard::MingceCard(){
    once = true;
    will_throw = false;
}

void MingceCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);

    Room *room = effect.to->getRoom();
    QString choice = room->askForChoice(effect.to, "mingce", "nothing+use+draw");
    if(choice == "use"){
        room->throwCard(this);
        QList<ServerPlayer *> players = room->getOtherPlayers(effect.to), targets;
        foreach(ServerPlayer *player, players){
            if(effect.to->canSlash(player))
                targets << player;
        }

        if(!targets.isEmpty()){
            ServerPlayer *target = room->askForPlayerChosen(effect.to, targets, "mingce");
            room->cardEffect(new Slash(Card::NoSuit, 0), effect.to, target);
        }
    }else if(choice == "draw"){
        room->throwCard(this);
        effect.to->drawCards(1, true);
    }
}

class Mingce: public OneCardViewAsSkill{
public:
    Mingce():OneCardViewAsSkill("mingce"){
        default_choice = "nothing";
    }

    virtual bool isEnabledAtPlay() const{
        return ! ClientInstance->hasUsed("MingceCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *c = to_select->getCard();
        return c->getTypeId() == Card::Equip || c->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        MingceCard *card = new MingceCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class DongchaClear: public TriggerSkill{
public:
    DongchaClear():TriggerSkill("#dongcha-clear"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        player->getRoom()->setTag("ChengongDongcha", QVariant());

        return false;
    }
};

class ChengongDongcha: public TriggerSkill{
public:
    ChengongDongcha():TriggerSkill("chengong_dongcha"){
        events << Damaged << CardEffected;

        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(event == Damaged){
            room->setTag("ChengongDongcha", player->objectName());
        }else if(event == CardEffected){
            Room *room = player->getRoom();
            if(room->getTag("ChengongDongcha").toString() != player->objectName())
                return false;

            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card->inherits("Slash") || effect.card->getTypeId() == Card::Trick)
                return true;
        }

        return false;
    }
};

GanluCard::GanluCard(){
    once = true;
}

void GanluCard::swapEquip(ServerPlayer *first, ServerPlayer *second, int index) const{
    const EquipCard *e1 = first->getEquip(index);
    const EquipCard *e2 = second->getEquip(index);

    Room *room = first->getRoom();

    if(e1)
        first->obtainCard(e1);

    if(e2)
        room->moveCardTo(e2, first, Player::Equip);

    if(e1)
        room->moveCardTo(e1, second, Player::Equip);
}

bool GanluCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    return targets.length() == 2;
}

bool GanluCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.length() < 2 && ! to_select->getEquips().isEmpty();
}

void GanluCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *first = targets.first();
    ServerPlayer *second = targets.at(1);

    int i;
    for(i=0; i<4; i++)
        swapEquip(first, second, i);

    LogMessage log;
    log.type = "#GanluSwap";
    log.from = source;
    log.to = targets;
    room->sendLog(log);
}

class Ganlu: public ZeroCardViewAsSkill{
public:
    Ganlu():ZeroCardViewAsSkill("ganlu"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! ClientInstance->hasUsed("GanluCard");
    }

    virtual const Card *viewAs() const{
        return new GanluCard;
    }
};

static QString BuyiCallback(const Card *card, Room *){
    return card->isRed() ? "good" : "bad";
}

class Buyi: public TriggerSkill{
public:
    Buyi():TriggerSkill("buyi"){
        events << Dying;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->askForSkillInvoke(objectName(), data)){
            Room *room = player->getRoom();

            if(room->judge(player, BuyiCallback) == "good"){
                DyingStruct dying = data.value<DyingStruct>();

                CardUseStruct use;
                Peach *peach = new Peach(Card::NoSuit, 0);
                peach->setSkillName(objectName());
                use.card = peach;
                use.from = player;
                use.to << dying.who;

                room->useCard(use);
            }
        }

        return false;
    }
};

YJCMPackage::YJCMPackage():Package("YJCM"){
    General *caozhi = new General(this, "caozhi", "wei", 3);
    caozhi->addSkill(new Luoying);
    caozhi->addSkill(new Jiushi);
    caozhi->addSkill(new JiushiFlip);

    General *yujin = new General(this, "yujin", "wei");
    yujin->addSkill(new Yizhong);

    General *xushu = new General(this, "xushu", "shu", 3);
    xushu->addSkill(new Wuyan);
    xushu->addSkill(new Jujian);

    General *masu = new General(this, "masu", "shu", 3);
    masu->addSkill(new Huilei);

    General *fazheng = new General(this, "fazheng", "shu", 3);
    fazheng->addSkill(new Enyuan);

    General *lingtong = new General(this, "lingtong", "wu");
    lingtong->addSkill(new Xuanfeng);

    General *xusheng = new General(this, "xusheng", "wu");
    xusheng->addSkill(new Pojun);

    General *wuguotai = new General(this, "wuguotai", "wu", 3, false);
    wuguotai->addSkill(new Ganlu);
    wuguotai->addSkill(new Buyi);

    General *chengong = new General(this, "chengong", "qun", 3);
    chengong->addSkill(new ChengongDongcha);
    chengong->addSkill(new Mingce);

    General *gaoshun = new General(this, "gaoshun", "qun");
    gaoshun->addSkill(new Jiejiu);

    addMetaObject<JujianCard>();
    addMetaObject<MingceCard>();
    addMetaObject<GanluCard>();
    addMetaObject<XianzhenCard>();
}

ADD_PACKAGE(YJCM)
