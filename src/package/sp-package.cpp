#include "sp-package.h"
#include "general.h"
#include "skill.h"
#include "standard-skillcards.h"
#include "carditem.h"
#include "engine.h"
#include "standard.h"
#include "maneuvering.h"
#include "wisdompackage.h"

class SPMoonSpearSkill: public WeaponSkill{
public:
    SPMoonSpearSkill():WeaponSkill("sp_moonspear"){
        events << CardFinished << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;

        CardStar card = NULL;
        if(event == CardFinished){
            CardUseStruct card_use = data.value<CardUseStruct>();
            card = card_use.card;

            if(card == player->tag["MoonSpearSlash"].value<CardStar>()){
                card = NULL;
            }
        }else if(event == CardResponsed){
            card = data.value<CardStar>();
            player->tag["MoonSpearSlash"] = data;
        }

        if(card == NULL || !card->isBlack())
            return false;

        Room *room = player->getRoom();
        if(!room->askForSkillInvoke(player, objectName(), data))
            return false;
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
            if(player->inMyAttackRange(tmp))
                targets << tmp;
        }
        if(targets.isEmpty()) return false;
        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
        if(!room->askForCard(target, "jink", "@moon-spear-jink")){
            DamageStruct damage;
            damage.from = player;
            damage.to = target;
            room->damage(damage);
        }
        return false;
    }
};

class SPMoonSpear: public Weapon{
public:
    SPMoonSpear(Suit suit = Card::Diamond, int number = 12)
        :Weapon(suit, number, 3){
        setObjectName("sp_moonspear");
        skill = new SPMoonSpearSkill;
    }
};

class JileiClear: public PhaseChangeSkill{
public:
    JileiClear():PhaseChangeSkill("#jilei-clear"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::NotActive){
            Room *room = target->getRoom();
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *player, players){
                if(player->hasFlag("jilei")){
                    player->jilei(".");
                    player->invoke("jilei", ".");

                    LogMessage log;
                    log.type = "#JileiClear";
                    log.from = player;
                    room->sendLog(log);
                }
            }
        }

        return false;
    }
};


class Jilei: public TriggerSkill{
public:
    Jilei():TriggerSkill("jilei"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yangxiu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from == NULL)
           return false;

        Room *room = yangxiu->getRoom();
        if(room->askForSkillInvoke(yangxiu, objectName(), data)){
            QString choice = room->askForChoice(yangxiu, objectName(), "basic+equip+trick");
            room->playSkillEffect(objectName());

            damage.from->jilei(choice);
            damage.from->invoke("jilei", choice);
            damage.from->setFlags("jilei");

            LogMessage log;
            log.type = "#Jilei";
            log.from = yangxiu;
            log.to << damage.from;
            log.arg = choice;
            room->sendLog(log);
        }

        return false;
    }
};

class Danlao: public TriggerSkill{
public:
    Danlao():TriggerSkill("danlao"){
        events << CardUsed << CardEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            ServerPlayer *yangxiu = room->findPlayerBySkillName(objectName());
            if(!yangxiu || use.to.length() <= 1 ||
                    !use.to.contains(yangxiu) ||
                    !use.card->inherits("TrickCard") || use.card->inherits("Collateral") ||
                    !room->askForSkillInvoke(yangxiu, objectName(), data))
                return false;

            yangxiu->tag["Danlao"] = use.card->getEffectiveId();

            room->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#DanlaoAvoid";
            log.from = yangxiu;
            log.arg = use.card->objectName();
            log.arg2 = objectName();

            room->sendLog(log);

            yangxiu->drawCards(1);
        }
        else{
            if(!player->isAlive() || !player->hasSkill(objectName()))
                return false;

            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(player->tag["Danlao"].isNull() || player->tag["Danlao"].toInt() != effect.card->getEffectiveId())
                return false;

            player->tag["Danlao"] = QVariant(QString());
            return true;
        }

        return false;
    }
};


class Yongsi: public TriggerSkill{
public:
    Yongsi():TriggerSkill("yongsi"){
        events << DrawNCards << PhaseChange;
        frequency = Compulsory;
    }

    int getKingdoms(ServerPlayer *yuanshu) const{
        QSet<QString> kingdom_set;
        Room *room = yuanshu->getRoom();
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            kingdom_set << p->getKingdom();
        }

        return kingdom_set.size();
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yuanshu, QVariant &data) const{
        if(event == DrawNCards){
            int x = getKingdoms(yuanshu);
            data = data.toInt() + x;

            Room *room = yuanshu->getRoom();
            LogMessage log;
            log.type = "#YongsiGood";
            log.from = yuanshu;
            log.arg = QString::number(x);
            log.arg2 = objectName();
            room->sendLog(log);

            room->playSkillEffect("yongsi", x);

        }else if(event == PhaseChange && yuanshu->getPhase() == Player::Discard){
            int x = getKingdoms(yuanshu);
            int total = yuanshu->getEquips().length() + yuanshu->getHandcardNum();
            Room *room = yuanshu->getRoom();

            if(total <= x){
                yuanshu->throwAllHandCards();
                yuanshu->throwAllEquips();

                LogMessage log;
                log.type = "#YongsiWorst";
                log.from = yuanshu;
                log.arg = QString::number(total);
                log.arg2 = objectName();
                room->sendLog(log);

            }else if(yuanshu->hasFlag("jilei")){
                QSet<const Card *> jilei_cards;
                QList<const Card *> handcards = yuanshu->getHandcards();
                foreach(const Card *card, handcards){
                    if(yuanshu->isJilei(card))
                        jilei_cards << card;
                }
                int total = handcards.size() - jilei_cards.size() + yuanshu->getEquips().length();
                if(x > total){
                    // show all his cards
                    room->showAllCards(yuanshu);
                    LogMessage log;
                    log.type = "#YongsiBad";
                    log.from = yuanshu;
                    log.arg = QString::number(total);
                    log.arg2 = objectName();
                    room->sendLog(log);
                    yuanshu->throwAllEquips();
                    foreach(const Card *card, handcards.toSet() - jilei_cards){
                        room->throwCard(card);
                    }
                }
            }else{
                room->askForDiscard(yuanshu, "yongsi", x, false, true);

                LogMessage log;
                log.type = "#YongsiBad";
                log.from = yuanshu;
                log.arg = QString::number(x);
                log.arg2 = objectName();
                room->sendLog(log);
            }
        }

        return false;
    }
};

WeidiCard::WeidiCard(){
    target_fixed = true;
}

void WeidiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *yuanshu = card_use.from;

    QStringList choices;
    if(yuanshu->hasLordSkill("jijiang")&&Slash::IsAvailable(yuanshu))
        choices << "jijiang";

    if(yuanshu->hasLordSkill("weidai")&&Analeptic::IsAvailable(yuanshu))
        choices << "weidai";

    if(choices.isEmpty())
        return;

    QString choice;
    if(choices.length() == 1)
        choice = choices.first();
    else
        choice = room->askForChoice(yuanshu, "weidi", "jijiang+weidai");

    if(choice == "jijiang"){
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer* target, room->getOtherPlayers(yuanshu)){
            if(yuanshu->canSlash(target))
                targets << target;
        }

        ServerPlayer* target = room->askForPlayerChosen(yuanshu, targets, "jijiang");
        if(target){
            CardUseStruct use;
            use.card = new JijiangCard;
            use.from = yuanshu;
            use.to << target;
            room->useCard(use);
        }
    }else{
        CardUseStruct use;
        use.card = new WeidaiCard;
        use.from = yuanshu;
        room->useCard(use);
    }
}

class Weidi:public ZeroCardViewAsSkill{
public:
    Weidi():ZeroCardViewAsSkill("weidi"){
        frequency = Compulsory;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return (player->hasLordSkill("jijiang") && Slash::IsAvailable(player))
                ||(player->hasLordSkill("weidai") && Analeptic::IsAvailable(player));
    }

    virtual const Card *viewAs() const{
        return new WeidiCard;
    }
};


class Yicong: public DistanceSkill{
public:
    Yicong():DistanceSkill("yicong"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->hasSkill(objectName()) && from->getHp() > 2)
            correct --;
        if(to->hasSkill(objectName()) && to->getHp() <= 2)
            correct ++;

        return correct;
    }
};

class Xiuluo: public PhaseChangeSkill{
public:
    Xiuluo():PhaseChangeSkill("xiuluo"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && !target->isKongcheng()
                && !target->getJudgingArea().isEmpty();
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        bool once_success = false;
        do{
            once_success = false;

            if(!target->askForSkillInvoke(objectName()))
                return false;

            Room *room = target->getRoom();
            int card_id = room->askForCardChosen(target, target, "j", objectName());
            const Card *card = Sanguosha->getCard(card_id);

            QString suit_str = card->getSuitString();
            QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
            QString prompt = QString("@xiuluo:::%1").arg(suit_str);
            if(room->askForCard(target, pattern, prompt, QVariant(), CardDiscarded)){
                room->throwCard(card);
                once_success = true;
            }
        }while(!target->getCards("j").isEmpty() && once_success);

        return false;
    }
};

class Shenwei: public DrawCardsSkill{
public:
    Shenwei():DrawCardsSkill("#shenwei-draw"){
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        return n + 2;
    }
};

class ShenweiKeep: public MaxCardsSkill{
public:
    ShenweiKeep():MaxCardsSkill("shenwei"){
    }

    virtual int getExtra(const Player *target) const{
        if(target->hasSkill(objectName()))
            return 2;
        else
            return 0;
    }
};

class Danji: public PhaseChangeSkill{
public:
    Danji():PhaseChangeSkill("danji"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("danji") == 0
                && target->getHandcardNum() > target->getHp();
    }

    virtual bool onPhaseChange(ServerPlayer *guanyu) const{
        Room *room = guanyu->getRoom();
        ServerPlayer *the_lord = room->getLord();
        if(the_lord && the_lord->isCaoCao()){
            LogMessage log;
            log.type = "#DanjiWake";
            log.from = guanyu;
            log.arg = QString::number(guanyu->getHandcardNum());
            log.arg2 = QString::number(guanyu->getHp());
            room->sendLog(log);

            guanyu->setMark("danji", 1);

            room->loseMaxHp(guanyu);
            room->acquireSkill(guanyu, "mashu");
        }

        return false;
    }
};

YuanhuCard::YuanhuCard(){
    mute = true;
    will_throw = false;
}

bool YuanhuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    const Card *card = Sanguosha->getCard(subcards.first());
    const EquipCard *equip = qobject_cast<const EquipCard *>(card);
    int equip_index = static_cast<int>(equip->location());
    return to_select->getEquip(equip_index) == NULL;
}

void YuanhuCard::onUse(Room *room, const CardUseStruct &card_use) const{
    int index = -1;
    if (card_use.to.first() == card_use.from)
        index = 5;
    else if (card_use.to.first()->isCaoCao())
        index = 4;
    else {
        const Card *card = Sanguosha->getCard(card_use.card->getSubcards().first());
        if (card->isKindOf("Weapon"))
            index = 1;
        else if (card->isKindOf("Armor"))
            index = 2;
        else if (card->isKindOf("Horse"))
            index = 3;
    }
    room->playSkillEffect("yuanhu", index);
    SkillCard::onUse(room, card_use);
}

void YuanhuCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *caohong = effect.from;
    Room *room = caohong->getRoom();
    room->moveCardTo(this, effect.to, Player::Equip);

    const Card *card = Sanguosha->getCard(subcards.first());

    LogMessage log;
    log.type = "$YuanhuEquip";
    log.from = effect.from;
    log.to << effect.to;
    log.card_str = card->getEffectIdString();
    room->sendLog(log);

    if (card->isKindOf("Weapon")) {
      QList<ServerPlayer *> targets;
      foreach (ServerPlayer *p, room->getAllPlayers()) {
          if (effect.to->distanceTo(p) == 1 && !p->isAllNude())
              targets << p;
      }
      if (!targets.isEmpty()) {
          ServerPlayer *to_dismantle = room->askForPlayerChosen(caohong, targets, "yuanhu");
          int card_id = room->askForCardChosen(caohong, to_dismantle, "hej", "yuanhu");
          room->throwCard(card_id);
      }
    } else if (card->isKindOf("Armor")) {
        effect.to->drawCards(1);
    } else if (card->isKindOf("Horse")) {
        RecoverStruct recover;
        recover.who = effect.from;
        room->recover(effect.to, recover);
    }
}

class YuanhuViewAsSkill: public OneCardViewAsSkill {
public:
    YuanhuViewAsSkill(): OneCardViewAsSkill("yuanhu") {
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@yuanhu";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isKindOf("EquipCard");
    }

    virtual const Card *viewAs(CardItem *originalcard) const{
        YuanhuCard *first = new YuanhuCard;
        first->addSubcard(originalcard->getFilteredCard());
        first->setSkillName(objectName());
        return first;
    }
};

class Yuanhu: public PhaseChangeSkill {
public:
    Yuanhu(): PhaseChangeSkill("yuanhu") {
        view_as_skill = new YuanhuViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        if(target->getPhase() == Player::Finish && !target->isNude())
            room->askForUseCard(target, "@@yuanhu", "@yuanhu-equip");
        return false;
    }
};

XuejiCard::XuejiCard(){
}

bool XuejiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (targets.length() >= Self->getLostHp())
        return false;

    if (to_select == Self)
        return false;

    if (Self->getWeapon() && Self->getWeapon()->getEffectiveId() == getEffectiveId())
        return Self->distanceTo(to_select) == 1;
    else if (Self->getOffensiveHorse() && Self->getOffensiveHorse()->getEffectiveId() == getEffectiveId())
        return Self->distanceTo(to_select) <= Self->getAttackRange();
    else
        return Self->distanceTo(to_select) <= Self->getAttackRange();
}

void XuejiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    DamageStruct damage;
    damage.from = source;

    foreach (ServerPlayer *p, targets) {
        damage.to = p;
        room->damage(damage);
    }
    foreach (ServerPlayer *p, targets) {
        if(p->isAlive())
            p->drawCards(1);
    }
}

class Xueji: public OneCardViewAsSkill {
public:
    Xueji(): OneCardViewAsSkill("xueji") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getLostHp() > 0 && !player->hasUsed("XuejiCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *originalcard) const{
        XuejiCard *first = new XuejiCard;
        first->addSubcard(originalcard->getFilteredCard());
        first->setSkillName(objectName());
        return first;
    }
};

class Huxiao: public TriggerSkill {
public:
    Huxiao(): TriggerSkill("huxiao") {
        events << SlashMissed << PhaseChange;
    }
    
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room* room = player->getRoom();
        if(event == SlashMissed) {
            if(player->getPhase() == Player::Play && player->getMark("wuji") == 0)
                room->setPlayerMark(player, objectName(), player->getMark(objectName()) + 1);
        }else if(event == PhaseChange) {
            if(player->getPhase() == Player::Play)
                if(player->getMark(objectName()) > 0)
                    room->setPlayerMark(player, objectName(), 0);
        }

        return false;
    }
};

class HuxiaoRemove: public TriggerSkill {
public:
    HuxiaoRemove(): TriggerSkill("#huxiao") {
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        if(data.toString() == "huxiao")
            player->getRoom()->setPlayerMark(player, "huxiao", 0);
        return false;
    }
};

class WujiCount: public TriggerSkill {
public:
    WujiCount(): TriggerSkill("#wuji-count") {
        events << DamageDone << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room* room = player->getRoom();
        if(event == DamageDone) {
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.from && damage.from->isAlive() && damage.from == room->getCurrent() && damage.from->getMark("wuji") == 0)
                room->setPlayerMark(damage.from, "wuji_damage", damage.from->getMark("wuji_damage") + damage.damage);
        }else if(event == PhaseChange) {
            if(player->getPhase() == Player::NotActive)
                if(player->getMark("wuji_damage") > 0)
                    room->setPlayerMark(player, "wuji_damage", 0);
        }

        return false;
    }
};

class Wuji: public PhaseChangeSkill {
public:
    Wuji(): PhaseChangeSkill("wuji") {
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Finish
                && target->getMark("wuji") == 0
                && target->getMark("wuji_damage") >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#WujiWake";
        log.from = player;
        log.arg = QString::number(player->getMark("wuji_damage"));
        log.arg2 = objectName();
        room->sendLog(log);

        room->playSkillEffect(objectName());
        //room->broadcastInvoke("animate", "lightbox:$wuji:3000");
        //room->getThread()->delay(4000);

        player->addMark("wuji");

        room->setPlayerProperty(player, "maxhp", player->getMaxHp() + 1);

        RecoverStruct recover;
        recover.who = player;
        room->recover(player, recover);

        room->detachSkillFromPlayer(player, "huxiao");

        return false;
    }
};

BifaCard::BifaCard(){
    will_throw = false;
    mute = true;
}

bool BifaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getPile("bifa").isEmpty() && to_select != Self;
}

void BifaCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    target->tag["BifaSource"] = QVariant::fromValue((PlayerStar)source);
    room->playSkillEffect("bifa", 1);
    foreach(int id, getSubcards())
        target->addToPile("bifa", id, false);
}

class BifaViewAsSkill: public OneCardViewAsSkill {
public:
    BifaViewAsSkill(): OneCardViewAsSkill("bifa") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@bifa";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new BifaCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Bifa: public TriggerSkill {
public:
    Bifa(): TriggerSkill("bifa") {
        events << PhaseChange;
        view_as_skill = new BifaViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        if(player->hasSkill(objectName())){
            if(player->getPhase() == Player::Finish && !player->isKongcheng())
                room->askForUseCard(player, "@@bifa", "@bifa-remove");
            return false;
        }
        if(player->getPhase() == Player::RoundStart && player->getPile("bifa").length() > 0) {
            QList<int> bifa_list = player->getPile("bifa");
            const Card *pen = Sanguosha->getCard(bifa_list.first());
            room->obtainCard(player, pen, false);
            ServerPlayer *chenlin = player->tag["BifaSource"].value<PlayerStar>();
            QString pattern;
            if(pen->isKindOf("BasicCard"))
                pattern = "BasicCard";
            else if(pen->isKindOf("TrickCard"))
                pattern = "TrickCard";
            else if(pen->isKindOf("EquipCard"))
                pattern = "EquipCard";
            QVariant data_for_ai = QVariant::fromValue(pattern);
            pattern.append("|.|.|hand");
            const Card *card = room->askForCard(player, pattern, "@bifa-give:" + pen->getType(), data_for_ai, NonTrigger);
            if(card && card != pen){
                room->playSkillEffect("bifa", 2);
                chenlin->obtainCard(card, false);
            }else{
                room->playSkillEffect("bifa", 3);
                room->throwCard(pen);
                room->loseHp(player);
            }
            player->tag.remove("BifaSource");
        }
        return false;
    }
};

SongciCard::SongciCard(){
}

bool SongciCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return to_select->getMark("@songci") == 0 && to_select->getHandcardNum() != to_select->getHp();
}

void SongciCard::onEffect(const CardEffectStruct &effect) const{
    int handcard_num = effect.to->getHandcardNum();
    int hp = effect.to->getHp();
	Room *room = effect.from->getRoom();
	if (handcard_num == hp)
		return;
    effect.to->gainMark("@songci");
    if (handcard_num > hp) {
        room->playSkillEffect("songci", 2);
        effect.to->getRoom()->askForDiscard(effect.to, "songci", 2, false, true);
	}
    else if (handcard_num < hp) {
        room->playSkillEffect("songci", 1);
        effect.to->drawCards(2, "songci");
	}
}

class SongciViewAsSkill: public ZeroCardViewAsSkill{
public:
    SongciViewAsSkill():ZeroCardViewAsSkill("songci"){
    }

    virtual const Card *viewAs() const{
        return new SongciCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if (player->getMark("@songci") == 0 && player->getHandcardNum() != player->getHp()) return true;
        foreach (const Player *sib, player->getSiblings())
            if (sib->getMark("@songci") == 0 && sib->getHandcardNum() != sib->getHp())
                return true;
        return false;
    }
};

class Songci: public TriggerSkill {
public:
    Songci(): TriggerSkill("songci") {
        events << Death;
        view_as_skill = new SongciViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        foreach (ServerPlayer *p, room->getAllPlayers())
            if (p->getMark("@songci") > 0)
                room->setPlayerMark(p, "@songci", 0);
		return false;
    }
};

SPCardPackage::SPCardPackage()
    :Package("sp_cards")
{
    (new SPMoonSpear)->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(SPCard)

SPPackage::SPPackage()
    :Package("sp")
{
    General *yangxiu = new General(this, "yangxiu", "wei", 3);
    yangxiu->addSkill(new Jilei);
    yangxiu->addSkill(new JileiClear);
    yangxiu->addSkill(new Danlao);
    related_skills.insertMulti("jilei", "#jilei-clear");

    General *sp_diaochan = new General(this, "sp_diaochan", "qun", 3, false, true);
    sp_diaochan->addSkill("lijian");
    sp_diaochan->addSkill("biyue");

    General *gongsunzan = new General(this, "gongsunzan", "qun");
    gongsunzan->addSkill(new Yicong);

    General *yuanshu = new General(this, "yuanshu", "qun");
    yuanshu->addSkill(new Yongsi);
    yuanshu->addSkill(new Weidi);
    yuanshu->addSkill(new SPConvertSkill("#yuanshut", "yuanshu", "tai_yuanshu"));

    General *sp_sunshangxiang = new General(this, "sp_sunshangxiang", "shu", 3, false, true);
    sp_sunshangxiang->addSkill("jieyin");
    sp_sunshangxiang->addSkill("xiaoji");

    General *sp_pangde = new General(this, "sp_pangde", "wei", 4, true, true);
    sp_pangde->addSkill("mengjin");
    sp_pangde->addSkill("mashu");

    General *sp_guanyu = new General(this, "sp_guanyu", "wei", 4, true, true);
    sp_guanyu->addSkill("wusheng");
    sp_guanyu->addSkill(new Danji);

    General *shenlvbu1 = new General(this, "shenlvbu1", "god", 8, true, true);
    shenlvbu1->addSkill("mashu");
    shenlvbu1->addSkill("wushuang");

    General *shenlvbu2 = new General(this, "shenlvbu2", "god", 4, true, true);
    shenlvbu2->addSkill("mashu");
    shenlvbu2->addSkill("wushuang");
    shenlvbu2->addSkill(new Xiuluo);
    shenlvbu2->addSkill(new ShenweiKeep);
    shenlvbu2->addSkill(new Shenwei);
    shenlvbu2->addSkill(new Skill("shenji"));
    related_skills.insertMulti("shenwei", "#shenwei-draw");

    General *sp_caiwenji = new General(this, "sp_caiwenji", "wei", 3, false, true);
    sp_caiwenji->addSkill("beige");
    sp_caiwenji->addSkill("duanchang");

    General *sp_machao = new General(this, "sp_machao", "qun", 4, true, true);
    sp_machao->addSkill("mashu");
    sp_machao->addSkill("tieji");

    General *sp_jiaxu = new General(this, "sp_jiaxu", "wei", 3, true, true);
    sp_jiaxu->addSkill("wansha");
    sp_jiaxu->addSkill("luanwu");
    sp_jiaxu->addSkill("weimu");
    sp_jiaxu->addSkill("#@chaos-1");

    General *caohong = new General(this, "caohong", "wei");
    caohong->addSkill(new Yuanhu);

    General *guanyinping = new General(this, "guanyinping", "shu", 3, false);
    guanyinping->addSkill(new Xueji);
    guanyinping->addSkill(new Huxiao);
    guanyinping->addSkill(new HuxiaoRemove);
    guanyinping->addSkill(new Wuji);
    guanyinping->addSkill(new WujiCount);
    related_skills.insertMulti("wuji", "#wuji-count");
    related_skills.insertMulti("huxiao", "#huxiao");

    General *chenlin = new General(this, "chenlin", "wei", 3);
    chenlin->addSkill(new Bifa);
    chenlin->addSkill(new Songci);
    
    addMetaObject<WeidiCard>();
    addMetaObject<YuanhuCard>();
    addMetaObject<XuejiCard>();
    addMetaObject<BifaCard>();
    addMetaObject<SongciCard>();

    General *tai_diaochan = new General(this, "tai_diaochan", "qun", 3, false, true);
    tai_diaochan->addSkill("lijian");
    tai_diaochan->addSkill("biyue");

    General *tai_yuanshu= new General(this, "tai_yuanshu", "qun", 4, true, true);
    tai_yuanshu->addSkill("yongsi");
    tai_yuanshu->addSkill("weidi");

    General *tai_daqiao= new General(this, "tai_daqiao", "wu", 3, false, true);
    tai_daqiao->addSkill("guose");
    tai_daqiao->addSkill("liuli");

    General *tai_zhaoyun= new General(this, "tai_zhaoyun", "shu", 4, true, true);
    tai_zhaoyun->addSkill("longdan");
}

ADD_PACKAGE(SP)
