#include "yjcm-package.h"
#include "skill.h"
#include "standard.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class Yizhong: public TriggerSkill{
public:
    Yizhong():TriggerSkill("yizhong"){
        events << SlashEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getArmor() == NULL;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(effect.slash->isBlack()){
            player->getRoom()->playSkillEffect(objectName());

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
        events << CardDiscarded << CardUsed << FinishJudge;
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
        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(room->getCardPlace(judge->card->getEffectiveId()) == Player::DiscardedPile
               && judge->card->getSuit() == Card::Club)
               clubs << judge->card;
        }

        if(clubs.isEmpty())
            return false;

        ServerPlayer *caozhi = room->findPlayerBySkillName(objectName());
        if(caozhi && caozhi->askForSkillInvoke(objectName(), data)){
            if(player->getGeneralName() == "zhenji")
                room->playSkillEffect("luoying", 2);
            else
                room->playSkillEffect("luoying", 1);

            foreach(const Card *club, clubs)
                caozhi->obtainCard(club);
        }

        return false;
    }
};

class Jiushi: public ZeroCardViewAsSkill{
public:
    Jiushi():ZeroCardViewAsSkill("jiushi"){
        analeptic = new Analeptic(Card::NoSuit, 0);
        analeptic->setSkillName("jiushi");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return analeptic->isAvailable(player) && player->faceUp();
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern.contains("analeptic") && player->faceUp();
    }

    virtual const Card *viewAs() const{
        return analeptic;
    }

    virtual int getEffectIndex(ServerPlayer *, const Card *) const{
        return qrand() % 2 + 1;
    }

private:
    Analeptic *analeptic;
};

class JiushiFlip: public TriggerSkill{
public:
    JiushiFlip():TriggerSkill("#jiushi-flip"){
        events << CardUsed << Predamaged << Damaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->getSkillName() == "jiushi")
                player->turnOver();
        }else if(event == Predamaged){
            player->tag["PredamagedFace"] = player->faceUp();
        }else if(event == Damaged){
            bool faceup = player->tag.value("PredamagedFace").toBool();
            if(!faceup && player->askForSkillInvoke("jiushi", data)){
                player->getRoom()->playSkillEffect("jiushi", 3);
                player->turnOver();
            }
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

                room->playSkillEffect(objectName());

                return true;
            }

            if(effect.to->hasSkill(objectName()) && effect.from){
                LogMessage log;
                log.type = "#WuyanGood";
                log.from = effect.to;
                log.to << effect.from;
                log.arg = effect.card->objectName();

                room->sendLog(log);

                room->playSkillEffect(objectName());

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
    int n = subcardsLength();
    effect.to->drawCards(n);

    if(n == 3){
        QSet<Card::CardType> types;

        foreach(int card_id, effect.card->getSubcards()){
            const Card *card = Sanguosha->getCard(card_id);
            types << card->getTypeId();
        }

        if(types.size() == 1){
            Room *room = effect.from->getRoom();

            LogMessage log;
            log.type = "#JujianRecover";
            log.from = effect.from;
            const Card *card = Sanguosha->getCard(subcards.first());
            log.arg = card->getType();
            room->sendLog(log);

            RecoverStruct recover;
            recover.card = this;
            recover.who = effect.from;
            room->recover(effect.from, recover);
        }
    }
}

class Jujian: public ViewAsSkill{
public:
    Jujian():ViewAsSkill("jujian"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 3;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("JujianCard");
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
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(event == HpRecover){
            RecoverStruct recover = data.value<RecoverStruct>();
            if(recover.who && recover.who != player){
                recover.who->drawCards(recover.recover);

                LogMessage log;
                log.type = "#EnyuanRecover";
                log.from = player;
                log.to << recover.who;
                log.arg = QString::number(recover.recover);

                room->sendLog(log);

                room->playSkillEffect(objectName(), qrand() % 2 + 1);

            }
        }else if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            ServerPlayer *source = damage.from;
            if(source && source != player){
                room->playSkillEffect(objectName(), qrand() % 2 + 3);

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

XuanhuoCard::XuanhuoCard(){
    once = true;
    will_throw = false;
}

void XuanhuoCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);

    Room *room = effect.from->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "he", "xuanhuo");
    const Card *card = Sanguosha->getCard(card_id);
    bool is_public = room->getCardPlace(card_id) != Player::Hand;
    room->moveCardTo(card, effect.from, Player::Hand, is_public ? true : false);

    QList<ServerPlayer *> targets = room->getOtherPlayers(effect.to);
    ServerPlayer *target = room->askForPlayerChosen(effect.from, targets, "xuanhuo");
    if(target != effect.from)
        room->moveCardTo(card, target, Player::Hand, false);
}

class Xuanhuo: public OneCardViewAsSkill{
public:
    Xuanhuo():OneCardViewAsSkill("xuanhuo"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("XuanhuoCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        XuanhuoCard *card = new XuanhuoCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Huilei: public TriggerSkill{
public:
    Huilei():TriggerSkill("huilei"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;
        if(killer){
            Room *room = player->getRoom();

            LogMessage log;
            log.type = "#HuileiThrow";
            log.from = player;
            log.to << killer;
            room->sendLog(log);

            killer->throwAllEquips();
            killer->throwAllHandCards();

            QString killer_name = killer->getGeneralName();
            if(killer_name == "zhugeliang" || killer_name == "wolong" || killer_name == "shenzhugeliang")
                room->playSkillEffect(objectName(), 1);
            else
                room->playSkillEffect(objectName(), 2);
        }

        return false;
    }
};

class Xuanfeng: public TriggerSkill{
public:
    Xuanfeng():TriggerSkill("xuanfeng"){
        events << CardLost << CardLostDone;
    }

    virtual QString getDefaultChoice(ServerPlayer *) const{
        return "nothing";
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *lingtong, QVariant &data) const{
        if(event == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Equip)
                lingtong->tag["InvokeXuanfeng"] = true;
        }else if(event == CardLostDone && lingtong->tag.value("InvokeXuanfeng", false).toBool()){
            lingtong->tag.remove("InvokeXuanfeng");
            Room *room = lingtong->getRoom();

            QString choice = room->askForChoice(lingtong, objectName(), "slash+damage+nothing");
            room->playSkillEffect(objectName());

            if(choice == "slash"){
                QList<ServerPlayer *> targets;
                foreach(ServerPlayer *target, room->getAlivePlayers()){
                    if(lingtong->canSlash(target, false))
                        targets << target;
                }

                ServerPlayer *target = room->askForPlayerChosen(lingtong, targets, "xuanfeng-slash");

                Slash *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName(objectName());

                CardUseStruct card_use;
                card_use.card = slash;
                card_use.from = lingtong;
                card_use.to << target;
                room->useCard(card_use, false);
            }else if(choice == "damage"){
                QList<ServerPlayer *> players = room->getOtherPlayers(lingtong), targets;
                foreach(ServerPlayer *p, players){
                    if(lingtong->distanceTo(p) <= 1)
                        targets << p;
                }

                ServerPlayer *target = room->askForPlayerChosen(lingtong, targets, "xuanfeng-damage");

                DamageStruct damage;
                damage.from = lingtong;
                damage.to = target;
                room->damage(damage);
            }
        }

        return false;
    }
};

class Pojun: public TriggerSkill{
public:
    Pojun():TriggerSkill("pojun"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->isDead())
            return false;

        if(damage.card && damage.card->inherits("Slash") &&
           player->askForSkillInvoke(objectName(), data))
        {
            player->getRoom()->playSkillEffect(objectName());

            int x = qMin(5, damage.to->getHp());
            damage.to->drawCards(x);
            damage.to->turnOver();
        }

        return false;
    }
};

XianzhenCard::XianzhenCard(){
    once = true;
    will_throw = false;
}

bool XianzhenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && ! to_select->isKongcheng();
}

void XianzhenCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    const Card *card = Sanguosha->getCard(subcards.first());
    if(effect.from->pindian(effect.to, "xianzhen", card)){
        PlayerStar target = effect.to;
        effect.from->tag["XianzhenTarget"] = QVariant::fromValue(target);
        room->setPlayerFlag(effect.from, "xianzhen_success");
        room->setFixedDistance(effect.from, effect.to, 1);
    }else{
        room->setPlayerFlag(effect.from, "xianzhen_failed");
    }
}

XianzhenSlashCard::XianzhenSlashCard(){
    target_fixed = true;
}

void XianzhenSlashCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *target = card_use.from->tag["XianzhenTarget"].value<PlayerStar>();
    if(target == NULL || target->isDead())
        return;

    if(!card_use.from->canSlash(target, false))
        return;

    const Card *slash = room->askForCard(card_use.from, "slash", "@xianzhen-slash");
    if(slash){
        CardUseStruct use;
        use.card = slash;
        use.from = card_use.from;
        use.to << target;
        room->useCard(use);
    }
}

class XianzhenViewAsSkill: public ViewAsSkill{
public:
    XianzhenViewAsSkill():ViewAsSkill(""){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("XianzhenCard") || player->hasFlag("xianzhen_success");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(!selected.isEmpty())
            return false;

        if(Self->hasUsed("XianzhenCard"))
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(! Self->hasUsed("XianzhenCard")){
            if(cards.length() != 1)
                return NULL;

            XianzhenCard *card = new XianzhenCard;
            card->addSubcards(cards);
            return card;
        }else if(Self->hasFlag("xianzhen_success")){
            if(!cards.isEmpty())
                return NULL;

            return new XianzhenSlashCard;
        }else
            return NULL;
    }
};

class Xianzhen: public TriggerSkill{
public:
    Xianzhen():TriggerSkill("xianzhen"){
        view_as_skill = new XianzhenViewAsSkill;

        events << PhaseChange << SlashEffect << SlashHit << SlashMissed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *gaoshun, QVariant &data) const{
        ServerPlayer *target = gaoshun->tag["XianzhenTarget"].value<PlayerStar>();

        if(event == PhaseChange){
            if(gaoshun->getPhase() == Player::Finish && target){
                Room *room = gaoshun->getRoom();
                room->setFixedDistance(gaoshun, target, -1);
                gaoshun->tag.remove("XianzhenTarget");
            }
        }else{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            if(effect.to == target){
                if(event == SlashEffect)
                    target->addMark("qinggang");
                else
                    target->removeMark("qinggang");
            }
        }

        return false;
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
    QString choice = room->askForChoice(effect.to, "mingce", "use+draw");
    if(choice == "use"){
        QList<ServerPlayer *> players = room->getOtherPlayers(effect.to), targets;
        foreach(ServerPlayer *player, players){
            if(effect.to->canSlash(player))
                targets << player;
        }

        if(!targets.isEmpty()){
            ServerPlayer *target = room->askForPlayerChosen(effect.from, targets, "mingce");
            room->cardEffect(new Slash(Card::NoSuit, 0), effect.to, target);
        }
    }else if(choice == "draw"){
        effect.to->drawCards(1, true);
    }
}

class Mingce: public OneCardViewAsSkill{
public:
    Mingce():OneCardViewAsSkill("mingce"){
        default_choice = "draw";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("MingceCard");
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

class ZhichiClear: public TriggerSkill{
public:
    ZhichiClear():TriggerSkill("#zhichi-clear"){
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        if(player->getPhase() == Player::NotActive)
            player->getRoom()->setTag("Zhichi", QVariant());

        return false;
    }
};

class Zhichi: public TriggerSkill{
public:
    Zhichi():TriggerSkill("zhichi"){
        events << Damaged << CardEffected;

        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(player->getPhase() != Player::NotActive)
            return false;

        if(event == Damaged){
            room->setTag("Zhichi", player->objectName());

            room->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#ZhichiDamaged";
            log.from = player;
            room->sendLog(log);

        }else if(event == CardEffected){
            if(room->getTag("Zhichi").toString() != player->objectName())
                return false;

            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card->inherits("Slash") || effect.card->getTypeId() == Card::Trick){
                LogMessage log;
                log.type = "#ZhichiAvoid";
                log.from = player;
                room->sendLog(log);

                return true;
            }
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

bool GanluCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

bool GanluCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    switch(targets.length()){
    case 0: return true;
    case 1: {
            int n1 = targets.first()->getEquips().length();
            int n2 = to_select->getEquips().length();
            return qAbs(n1-n2) <= Self->getLostHp();
        }

    default:
        return false;
    }
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

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("GanluCard");
    }

    virtual const Card *viewAs() const{
        return new GanluCard;
    }
};

class Buyi: public TriggerSkill{
public:
    Buyi():TriggerSkill("buyi"){
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return !player->isKongcheng();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *wuguotai = room->findPlayerBySkillName(objectName());

        if(wuguotai && wuguotai->askForSkillInvoke(objectName(), data)){
            const Card *card = NULL;
            if(player == wuguotai)
                card = room->askForCardShow(player, wuguotai, objectName());
            else{
                int card_id = room->askForCardChosen(wuguotai, player, "h", "buyi");
                card = Sanguosha->getCard(card_id);
            }

            room->showCard(player, card->getEffectiveId());

            if(card->getTypeId() != Card::Basic){
                room->throwCard(card);

                room->playSkillEffect(objectName());

                RecoverStruct recover;
                recover.who = wuguotai;
                room->recover(player, recover);
            }
        }

        return false;
    }
};

XinzhanCard::XinzhanCard(){
    target_fixed = true;
    once = true;
}

void XinzhanCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<int> cards = room->getNCards(3), left;
    left = cards;

    QList<int> hearts;
    foreach(int card_id, cards){
        const Card *card = Sanguosha->getCard(card_id);
        if(card->getSuit() == Card::Heart)
            hearts << card_id;
    }

    if(!hearts.isEmpty()){
        room->fillAG(cards, source);

        while(!hearts.isEmpty()){
            int card_id = room->askForAG(source, hearts, true, "xinzhan");
            if(card_id == -1)
                break;

            if(!hearts.contains(card_id))
                continue;

            hearts.removeOne(card_id);
            left.removeOne(card_id);

            source->obtainCard(Sanguosha->getCard(card_id));
            room->showCard(source, card_id);
        }

        source->invoke("clearAG");
    }

    if(!left.isEmpty())
        room->doGuanxing(source, left, true);
 }

class Xinzhan: public ZeroCardViewAsSkill{
public:
    Xinzhan():ZeroCardViewAsSkill("xinzhan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("XinzhanCard") && player->getHandcardNum() > player->getMaxHP();
    }

    virtual const Card *viewAs() const{
        return new XinzhanCard;
    }
};

YJCMPackage::YJCMPackage():Package("YJCM"){
    General *caozhi = new General(this, "caozhi", "wei", 3);
    caozhi->addSkill(new Luoying);
    caozhi->addSkill(new Jiushi);
    caozhi->addSkill(new JiushiFlip);

    related_skills.insertMulti("jiushi", "#jiushi-flip");

    General *yujin = new General(this, "yujin", "wei");
    yujin->addSkill(new Yizhong);

    General *xushu = new General(this, "xushu", "shu", 3);
    xushu->addSkill(new Wuyan);
    xushu->addSkill(new Jujian);

    General *masu = new General(this, "masu", "shu", 3);
    masu->addSkill(new Xinzhan);
    masu->addSkill(new Huilei);

    General *fazheng = new General(this, "fazheng", "shu", 3);
    fazheng->addSkill(new Enyuan);
    fazheng->addSkill(new Xuanhuo);

    General *lingtong = new General(this, "lingtong", "wu");
    lingtong->addSkill(new Xuanfeng);

    General *xusheng = new General(this, "xusheng", "wu");
    xusheng->addSkill(new Pojun);

    General *wuguotai = new General(this, "wuguotai", "wu", 3, false);
    wuguotai->addSkill(new Ganlu);
    wuguotai->addSkill(new Buyi);

    General *chengong = new General(this, "chengong", "qun", 3);
    chengong->addSkill(new Zhichi);
    chengong->addSkill(new ZhichiClear);
    chengong->addSkill(new Mingce);

    related_skills.insertMulti("zhichi", "#zhichi-clear");

    General *gaoshun = new General(this, "gaoshun", "qun");
    gaoshun->addSkill(new Xianzhen);
    gaoshun->addSkill(new Jiejiu);

    addMetaObject<JujianCard>();
    addMetaObject<MingceCard>();
    addMetaObject<GanluCard>();
    addMetaObject<XianzhenCard>();
    addMetaObject<XianzhenSlashCard>();
    addMetaObject<XuanhuoCard>();
    addMetaObject<XinzhanCard>();
}

ADD_PACKAGE(YJCM)
