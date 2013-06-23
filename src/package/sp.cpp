#include "sp.h"
#include "general.h"
#include "client.h"
#include "standard-skillcards.h"
#include "carditem.h"
#include "engine.h"
#include "standard.h"
#include "maneuvering.h"
#include "wisdom.h"

class SPMoonSpearSkill: public WeaponSkill{
public:
    SPMoonSpearSkill():WeaponSkill("sp_moonspear"){
        events << CardFinished << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
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

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *yangxiu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from == NULL)
           return false;

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

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
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

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *yuanshu, QVariant &data) const{
        if(event == DrawNCards){
            int x = getKingdoms(yuanshu);
            data = data.toInt() + x;

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
                    room->showAllCards(yuanshu, true);
                    LogMessage log;
                    log.type = "#YongsiBad";
                    log.from = yuanshu;
                    log.arg = QString::number(total);
                    log.arg2 = objectName();
                    room->sendLog(log);
                    yuanshu->throwAllEquips();
                    DummyCard *dummy_card = new DummyCard;
                    foreach(const Card *card, handcards.toSet() - jilei_cards)
                        dummy_card->addSubcard(card);
                    room->throwCard(dummy_card, yuanshu);
                    delete dummy_card;
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
                room->throwCard(card, target);
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

class Shenji: public SlashSkill{
public:
    Shenji():SlashSkill("shenji"){
        frequency = NotFrequent;
    }

    virtual int getSlashExtraGoals(const Player *from, const Player *, const Card *) const{
        if(from->hasSkill(objectName()) && !from->getWeapon())
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
            room->playLightbox(guanyu, objectName(), "2500", 2500);

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
          room->throwCard(card_id, to_dismantle, caohong);
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

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == SlashMissed){
            if(player->getPhase() == Player::Play && player->getMark("wuji") == 0){
                room->playSkillEffect(objectName(), 1);
                room->setPlayerMark(player, objectName(), player->getMark(objectName()) + 1);
            }
        }else if(event == PhaseChange) {
            if(player->getMark(objectName()) > 0)
                room->setPlayerMark(player, objectName(), 0);
        }

        return false;
    }
};

class HuxiaoSlash: public SlashSkill{
public:
    HuxiaoSlash():SlashSkill("#huxiao_slash"){
    }

    virtual int getSlashResidue(const Player *likui) const{
        if(likui->hasSkill("huxiao")){
            int init = 1 - likui->getSlashCount();
            return init + likui->getMark("huxiao");
        }
        else
            return 0;
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

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
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

        room->playLightbox(player, "Wuji", "3100", 3100);

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
    return targets.isEmpty() && to_select->getPile("#pencil").isEmpty() && to_select != Self;
}

void BifaCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *target = effect.to;
    target->tag["BifaSource"] = QVariant::fromValue((PlayerStar)effect.from);
    effect.from->playSkillEffect("bifa", 1);
    foreach(int id, getSubcards())
        target->addToPile("#pencil", id, false);
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

class Bifa: public PhaseChangeSkill{
public:
    Bifa():PhaseChangeSkill("bifa"){
        view_as_skill = new BifaViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->hasSkill(objectName())){
            if(player->getPhase() == Player::Finish && !player->isKongcheng())
                room->askForUseCard(player, "@@bifa", "@bifa-remove");
        }
        if(player->getPhase() == Player::RoundStart && player->getPile("#pencil").length() > 0) {
            QList<int> bifa_list = player->getPile("#pencil");
            CardStar pen = Sanguosha->getCard(bifa_list.first());
            ServerPlayer *chenlin = player->tag["BifaSource"].value<PlayerStar>();
            if(!chenlin || chenlin->isDead()){
                room->playSkillEffect("bifa", 3);
                room->throwCard(pen);
                room->loseHp(player);
                return false;
            }
            QString pattern;
            if(pen->isKindOf("BasicCard"))
                pattern = "BasicCard";
            else if(pen->isKindOf("TrickCard"))
                pattern = "TrickCard";
            else if(pen->isKindOf("EquipCard"))
                pattern = "EquipCard";
            else
                pattern = "Shit";

            room->fillAG(bifa_list, player);
            //room->askForAG(player, bifa_list, true, objectName());

            QVariant data = QVariant::fromValue(pen);
            pattern.append("|.|.|hand");
            const Card *card = room->askForCard(player, pattern, "@bifa-give:" + pen->getType(), data, NonTrigger);
            if(card && card != pen){
                room->playSkillEffect("bifa", 2);
                chenlin->obtainCard(card);
                room->obtainCard(player, pen);
            }else{
                room->playSkillEffect("bifa", 3);
                room->throwCard(pen);
                room->loseHp(player);
            }
            player->invoke("clearAG");
            player->clearPile("#pencil");
            player->tag.remove("BifaSource");
        }
        return false;
    }
};

SongciCard::SongciCard(){
    mute = true;
}

bool SongciCard::targetFilter(const QList<const Player *> &, const Player *to_select, const Player *Self) const{
    return to_select->getMark("@sonnet") == 0 && to_select->getHandcardNum() != to_select->getHp();
}

void SongciCard::onEffect(const CardEffectStruct &effect) const{
    int handcard_num = effect.to->getHandcardNum();
    int hp = effect.to->getHp();
    Room *room = effect.from->getRoom();
    if(handcard_num == hp)
        return;
    effect.to->gainMark("@sonnet");
    if(handcard_num > hp){
        room->playSkillEffect("songci", 2);
        effect.to->getRoom()->askForDiscard(effect.to, "songci", 2, false, true);
    }
    else if(handcard_num < hp){
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
        if (player->getMark("@sonnet") == 0 && player->getHandcardNum() != player->getHp()) return true;
        foreach (const Player *sib, player->getSiblings())
            if (sib->getMark("@sonnet") == 0 && sib->getHandcardNum() != sib->getHp())
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

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        foreach (ServerPlayer *p, room->getAllPlayers())
            if (p->getMark("@sonnet") > 0)
                room->setPlayerMark(p, "@sonnet", 0);
		return false;
    }
};

class Tianming: public TriggerSkill{
public:
    Tianming(): TriggerSkill("tianming"){
        events << SlashEffected;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        //SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(player->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName(), 1);
            if(!player->isNude()){
                int total = 0;
                QSet<const Card *> jilei_cards;
                QList<const Card *> handcards = player->getHandcards();
                foreach(const Card *card, handcards){
                    if(player->isJilei(card))
                        jilei_cards << card;
                }
                total = handcards.size() - jilei_cards.size() + player->getEquips().length();

                if(total <= 2){
                    player->throwAllEquips();
                    player->throwAllHandCards();
                }
                else
                    room->askForDiscard(player, objectName(), 2, false, true);
            }

            player->drawCards(2);

            int max = -1000;
            foreach(ServerPlayer *p, room->getAllPlayers())
                if(p->getHp() > max)
                    max = p->getHp();
            if (player->getHp() == max)
                return false;

            QList<ServerPlayer *> maxs;
            foreach(ServerPlayer *p, room->getAllPlayers()) {
                if (p->getHp() == max)
                    maxs << p;
                if (maxs.size() > 1)
                    return false;
            }
            ServerPlayer *mosthp = maxs.first();
            if (room->askForSkillInvoke(mosthp, objectName())) {
                int index = 2;
                if (mosthp->getGeneral()->isFemale())
                    index = 3;
                room->playSkillEffect(objectName(), index);

                QSet<const Card *> jilei_cards;
                QList<const Card *> handcards = mosthp->getHandcards();
                foreach(const Card *card, handcards){
                    if(mosthp->isJilei(card))
                        jilei_cards << card;
                }
                int total = handcards.size() - jilei_cards.size() + mosthp->getEquips().length();

                if(total <= 2){
                    mosthp->throwAllEquips();
                    mosthp->throwAllHandCards();
                }
                else
                    room->askForDiscard(mosthp, objectName(), 2, false, true);
                mosthp->drawCards(2);
            }
        }

        return false;
    }
};

MizhaoCard::MizhaoCard(){
    will_throw = false;
    mute = true;
    once = true;
}

bool MizhaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void MizhaoCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(effect.card, false);
    Room *room = effect.from->getRoom();

    if(effect.to->getGeneralName().contains("liubei"))
        room->playSkillEffect("mizhao", 2);

    QList<ServerPlayer *> targets;
    foreach(ServerPlayer *p, room->getOtherPlayers(effect.to))
        if(!p->isKongcheng())
            targets << p;

    if(!effect.to->isKongcheng() && !targets.isEmpty()) {
        ServerPlayer *target = room->askForPlayerChosen(effect.from, targets, "mizhao");
        effect.to->pindian(target, "mizhao", NULL);
    }
}

class MizhaoViewAsSkill: public ViewAsSkill{
public:
    MizhaoViewAsSkill():ViewAsSkill("mizhao"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && !player->hasUsed("MizhaoCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if (cards.length() < Self->getHandcardNum())
            return NULL;

        MizhaoCard *card = new MizhaoCard;
        card->addSubcards(cards);
        return card;
    }
};

class Mizhao: public TriggerSkill{
public:
    Mizhao(): TriggerSkill("mizhao") {
        events << Pindian;
        view_as_skill = new MizhaoViewAsSkill;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        if (pindian->reason != objectName())
            return false;
        //if (pindian->from_card->getNumber() == pindian->to_card->getNumber())
        //    return false;

        ServerPlayer *winner = pindian->isSuccess() ? pindian->from : pindian->to;
        ServerPlayer *loser = pindian->isSuccess() ? pindian->to : pindian->from;
        if(winner->canSlash(loser, false)) {
            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName(objectName());
            CardUseStruct card_use;
            card_use.from = winner;
            card_use.to << loser;
            card_use.card = slash;
            room->useCard(card_use, false);
        }

        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const {
        return -2;
    }
};

class Jieyuan: public TriggerSkill{
public:
    Jieyuan(): TriggerSkill("jieyuan"){
        events << Predamaged << Predamage;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(event == Predamage){
            if(damage.to && damage.to->isAlive()
                && damage.to->getHp() >= player->getHp() && damage.to != player && !player->isKongcheng()){
                if(room->askForCard(player, ".black", "@JieyuanIncrease", data, CardDiscarded)){
                    room->playSkillEffect(objectName(), 1);

                    LogMessage log;
                    log.type = "#JieyuanIncrease";
                    log.from = player;
                    log.arg = QString::number(damage.damage);
                    log.arg2 = QString::number(++damage.damage);
                    room->sendLog(log);

                    data = QVariant::fromValue(damage);
                }
            }
        }else if(event == Predamaged){
            if(damage.from && damage.from->isAlive()
               && damage.from->getHp() >= player->getHp() && damage.from != player && !player->isKongcheng())
                if(damage.damage > 0 && room->askForCard(player, ".red", "@JieyuanDecrease", data, CardDiscarded)){
                    room->playSkillEffect(objectName(), 2);

                    LogMessage log;
                    log.type = "#JieyuanDecrease";
                    log.from = player;
                    log.arg = QString::number(damage.damage);
                    log.arg2 = QString::number(--damage.damage);
                    room->sendLog(log);

                    data = QVariant::fromValue(damage);
                }
        }

        return false;
    }
};

class Fenxin: public TriggerSkill{
public:
    Fenxin(): TriggerSkill("fenxin"){
        events << AskForPeachesDone;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if (!room->getMode().endsWith("p") && !room->getMode().endsWith("pd") && !room->getMode().endsWith("pz"))
            return false;
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.damage == NULL)
            return false;
        ServerPlayer *killer = dying.damage->from;
        if (killer == NULL || killer->isLord() || player->isLord() || player->getHp() > 0)
            return false;
        if (!killer->hasSkill(objectName()) || killer->getMark("@burnheart") == 0)
            return false;
        room->setPlayerFlag(player, "FenxinTarget");
        if (room->askForSkillInvoke(killer, objectName(), QVariant::fromValue(player))) {
            room->playLightbox(killer, objectName(), "", 1500);
            killer->loseMark("@burnheart");
            QString role1 = killer->getRole();
            killer->setRole(player->getRole());
            room->setPlayerProperty(killer, "role", player->getRole());
            player->setRole(role1);
            room->setPlayerProperty(player, "role", role1);
        }
        room->setPlayerFlag(player, "-FenxinTarget");
        return false;
    }
};

class Moukui: public TriggerSkill{
public:
    Moukui(): TriggerSkill("moukui"){
        events << SlashEffect << SlashMissed << CardFinished;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == SlashEffect){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            PlayerStar p = effect.to;
            if(player->askForSkillInvoke(objectName(), QVariant::fromValue(p))) {
                QString choice;
                if(p->isNude())
                    choice = "draw";
                else
                    choice = room->askForChoice(player, objectName(), "draw+discard");
                if(choice == "draw") {
                    room->playSkillEffect(objectName(), 1);
                    player->drawCards(1);
                }else {
                    room->playSkillEffect(objectName(), 2);
                    int disc = room->askForCardChosen(player, p, "he", objectName());
                    room->throwCard(disc, p, player);
                }
                room->setPlayerMark(p, objectName() + effect.slash->getEffectIdString(),
                                    p->getMark(objectName() + effect.slash->getEffectIdString()) + 1);
            }
        }else if(event == SlashMissed) {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(effect.to->getMark(objectName() + effect.slash->getEffectIdString()) <= 0)
                return false;
            if(!effect.from->isAlive() || !effect.to->isAlive() || effect.from->isNude())
                return false;
            int disc = room->askForCardChosen(effect.to, effect.from, "he", objectName());
            room->playSkillEffect(objectName(), 3);
            room->throwCard(disc, effect.from, effect.to);
            room->setPlayerMark(effect.to, objectName() + effect.slash->getEffectIdString(),
                                effect.to->getMark(objectName() + effect.slash->getEffectIdString()) - 1);
        }else if(event == CardFinished) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.card->isKindOf("Slash"))
                return false;
            foreach (ServerPlayer *p, room->getAllPlayers())
                room->setPlayerMark(p, objectName() + use.card->getEffectIdString(), 0);
        }

        return false;
    }
};

class Baobian: public TriggerSkill{
public:
    Baobian(): TriggerSkill("baobian"){
        frequency = Compulsory;
        events << HpChanged;
    }

    void loseskill(ServerPlayer *target, const QString &skill) const{
        if(target->getGeneral()->hasSkill(skill))
            return;
        if(target->getGeneral2() && target->getGeneral2()->hasSkill(skill))
            return;
        target->getRoom()->detachSkillFromPlayer(target, skill);
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *target, QVariant &data) const{
        if(target->isDead())
            return false;
        int hp = target->getHp();
        switch(hp){
        case 0:
        case 1:{
            room->playSkillEffect(objectName(), 3);
            room->acquireSkill(target, "shensu");
        }
        case 2:{
            if(hp == 2){
                room->playSkillEffect(objectName(), 2);
                loseskill(target, "shensu");
            }
            room->acquireSkill(target, "paoxiao");
        }
        case 3:{
            if(hp == 3){
                room->playSkillEffect(objectName(), 1);
                loseskill(target, "shensu");
                loseskill(target, "paoxiao");
            }
            room->acquireSkill(target, "tiaoxin");
            break;
        }
        default:
            QStringList skills;
            skills << "shensu" << "paoxiao" << "tiaoxin";
            foreach(QString skill, skills)
                loseskill(target, skill);
            break;
        }
        return false;
    }
};

class Xingwu: public TriggerSkill {
public:
    Xingwu(): TriggerSkill("xingwu") {
        events << CardUsed << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        if(event == CardUsed) {
            CardStar card = data.value<CardUseStruct>().card;
            if(card){
                int n = player->getMark(objectName());
                if(card->isBlack())
                    n |= 1;
                else if (card->isRed())
                    n |= 2;
                player->setMark(objectName(), n);
            }
        }else if(event == PhaseChange) {
            if(player->getPhase() == Player::Discard) {
                int n = player->getMark(objectName());
                bool red_avail = ((n & 2) == 0), black_avail = ((n & 1) == 0);
                if(player->isKongcheng() || (!red_avail && !black_avail))
                    return false;
                QString pattern = ".|.|.|hand";
                if (red_avail != black_avail)
                    pattern = QString("%1|%2").arg(pattern).arg(red_avail ? "red" : "black");
                const Card *card = room->askForCard(player, pattern, "@xingwu", QVariant(), NonTrigger);
                if(card){
                    LogMessage log;
                    log.type = "#InvokeSkill";
                    log.from = player;
                    log.arg = objectName();
                    room->sendLog(log);

                    player->addToPile("xizimai", card);

                    if(player->getPile("xizimai").length() >= 3){
                        player->clearPile("xizimai");
                        QList<ServerPlayer *> males;
                        foreach(ServerPlayer *p, room->getAlivePlayers()) {
                            if(p->getGenderString() == "male")
                                males << p;
                        }
                        if (males.isEmpty()) return false;

                        ServerPlayer *target = room->askForPlayerChosen(player, males, objectName());
                        DamageStruct damage;
                        damage.from = player;
                        damage.to = target;
                        damage.damage = 2;
                        room->damage(damage);

                        if (!player->isAlive()) return false;
                        QList<const Card *> equips = target->getEquips();
                        if (!equips.isEmpty()) {
                            DummyCard *dummy = new DummyCard;
                            foreach (const Card *equip, equips)
                                dummy->addSubcard(equip);
                            room->throwCard(dummy, target, player);
                            delete dummy;
                        }
                    }

                    if(player->hasSkill("luoyan")){
                        if(player->getPile("xizimai").isEmpty()){
                            if(player->getAcquiredSkills().contains("tianxiang"))
                                room->detachSkillFromPlayer(player, "tianxiang");
                            if(player->getAcquiredSkills().contains("liuli"))
                                room->detachSkillFromPlayer(player, "liuli");
                        }
                        else{
                            room->acquireSkill(player, "tianxiang");
                            room->acquireSkill(player, "liuli");
                        }
                    }
                }
            }else if(player->getPhase() == Player::RoundStart)
                player->setMark(objectName(), 0);
        }
        return false;
    }
};

class Aocai: public TriggerSkill{
public:
    Aocai():TriggerSkill("aocai"){
        events << CardAsked << AskForPeaches;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(room->getCurrent() == player)
            return false;
        if(event == CardAsked){
            const CardPattern *pattern = Sanguosha->getPattern(data.toString());
            if(player->askForSkillInvoke(objectName())){
                int peek = room->getDrawPile().at(0);
                room->showCard(player, peek);
                const Card *card = Sanguosha->getCard(peek);
                if(pattern->match(player, card) && card->isKindOf("BasicCard")){
                    room->showCard(player, room->getDrawPile().at(1));
                    room->provide(card);
                    return false;
                }
                else{
                    peek = room->getDrawPile().at(1);
                    room->showCard(player, peek);
                    card = Sanguosha->getCard(peek);
                    if(pattern->match(player, card) && card->isKindOf("BasicCard")){
                        room->provide(card);
                        return false;
                    }
                }
            }
        }
        else{
            DyingStruct dying = data.value<DyingStruct>();
            if(player->askForSkillInvoke(objectName(), QVariant::fromValue((PlayerStar)dying.who))){
                int peek = room->getDrawPile().at(0);
                room->showCard(player, peek);
                const Card *card = Sanguosha->getCard(peek);
                if((card->isKindOf("Analeptic") && player == dying.who) || card->isKindOf("Peach")){
                    room->showCard(player, room->getDrawPile().at(1));

                    CardUseStruct use;
                    use.card = card;
                    use.from = player;
                    use.to << dying.who;
                    room->useCard(use);
                    return false;
                }
                else{
                    peek = room->getDrawPile().at(1);
                    room->showCard(player, peek);
                    card = Sanguosha->getCard(peek);
                    if((card->isKindOf("Analeptic") && player == dying.who) || card->isKindOf("Peach")){
                        CardUseStruct use;
                        use.card = card;
                        use.from = player;
                        use.to << dying.who;
                        room->useCard(use);
                        return false;
                    }
                }
            }
        }
        return false;
    }
};

DuwuCard::DuwuCard(){
    once = true;
}

bool DuwuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(getSubcards().length() != to_select->getHp())
        return false;
    return Self->inMyAttackRange(to_select) && Self != to_select;
}

void DuwuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    DamageStruct damage;
    damage.from = effect.from;
    damage.to = effect.to;
    room->setPlayerFlag(damage.from, "duwumark");
    room->damage(damage);
    room->setPlayerFlag(damage.from, "-duwumark");
}

class DuwuViewAsSkill: public ViewAsSkill{
public:
    DuwuViewAsSkill():ViewAsSkill("duwu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasFlag("duwu");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        DuwuCard *card = new DuwuCard;
        card->addSubcards(cards);
        return card;
    }
};

class Duwu: public TriggerSkill{
public:
    Duwu():TriggerSkill("duwu"){
        events << AskForPeachesDone;
        view_as_skill = new DuwuViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        if(dying.damage == NULL)
            return false;
        ServerPlayer *killer = dying.damage->from;
        if(killer->hasSkill(objectName()) && killer->getPhase() == Player::Play &&
           killer->hasFlag("duwumark")){
            room->loseHp(killer);
            room->setPlayerFlag(killer, "-duwumark");
            room->setPlayerFlag(killer, "duwu");
        }
        return false;
    }
};

MouzhuCard::MouzhuCard(){
}

bool MouzhuCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    if(ServerInfo.GameMode == "02_1v1")
        return true;
    else
        return !targets.isEmpty();
}

void MouzhuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    PlayerStar target = targets.value(0, room->getOtherPlayers(source).first());
    if(!target->isKongcheng()){
        const Card *card = room->askForCard(target, ".", "@mouzhu:" + source->objectName(), QVariant::fromValue(target), NonTrigger);
        if(card)
            source->obtainCard(card, false);
        else
            return;
    }
    if(source->getHandcardNum() > target->getHandcardNum()){
        QString choice = room->askForChoice(target, skill_name, "duel+slash", QVariant::fromValue(target));
        CardUseStruct use;
        use.from = target;
        use.to << source;
        if(choice == "slash"){
            Slash *slash = new Slash(Card::NoSuit, 0);
            //slash->setSkillName(skill_name);
            use.card = slash;
            room->useCard(use);
        }
        else{
            Duel *duel = new Duel(Card::NoSuit, 0);
            //duel->setSkillName(skill_name);
            use.card = duel;
            room->useCard(use);
        }
    }
}

class Mouzhu:public ZeroCardViewAsSkill{
public:
    Mouzhu():ZeroCardViewAsSkill("mouzhu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("MouzhuCard");
    }

    virtual const Card *viewAs() const{
        return new MouzhuCard;
    }
};

class Yanhuo: public TriggerSkill{
public:
    Yanhuo():TriggerSkill("yanhuo"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *target = ServerInfo.GameMode == "02_1v1" ?
                               room->getOtherPlayers(player).first() :
                               damage ? damage->from : NULL;
        if(target && !player->isNude() && player->askForSkillInvoke(objectName(), data)){
            for(int i = player->getCardCount(true); i > 0; i --){
                if(target->isNude())
                    break;
                int card_id = room->askForCardChosen(player, target, "he", objectName());
                room->throwCard(card_id, target, player);
            }
        }
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
    //yuanshu->addSkill(new SPConvertSkill("#yuanshut", "yuanshu", "tai_yuanshu"));
    addMetaObject<WeidiCard>();

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
    shenlvbu1->addRelateSkill("Tshenlvbu");
    skills << new Skill("Tshenlvbu");

    General *shenlvbu2 = new General(this, "shenlvbu2", "god", 4, true, true);
    shenlvbu2->addSkill("mashu");
    shenlvbu2->addSkill("wushuang");
    shenlvbu2->addSkill(new Xiuluo);
    shenlvbu2->addSkill(new ShenweiKeep);
    shenlvbu2->addSkill(new Shenwei);
    shenlvbu2->addSkill(new Shenji);
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
    addMetaObject<YuanhuCard>();

    General *guanyinping = new General(this, "guanyinping", "shu", 3, false);
    guanyinping->addSkill(new Xueji);
    guanyinping->addSkill(new Huxiao);
    guanyinping->addSkill(new Wuji);
    guanyinping->addSkill(new WujiCount);
    related_skills.insertMulti("wuji", "#wuji-count");
    skills << new HuxiaoSlash;
    addMetaObject<XuejiCard>();

    General *sp_zhenji = new General(this, "sp_zhenji", "wei", 3, false, true);
    sp_zhenji->addSkill("qingguo");
    sp_zhenji->addSkill("luoshen");

    General *chenlin = new General(this, "chenlin", "wei", 3);
    chenlin->addSkill(new Bifa);
    chenlin->addSkill(new Songci);
    addMetaObject<BifaCard>();
    addMetaObject<SongciCard>();

    General *hanxiandi = new General(this, "hanxiandi", "qun", 3);
    hanxiandi->addSkill(new Tianming);
    hanxiandi->addSkill(new Mizhao);
    addMetaObject<MizhaoCard>();

    General *lingju = new General(this, "lingju", "qun", 3, false);
    lingju->addSkill(new Jieyuan);
    lingju->addSkill(new Fenxin);
    lingju->addSkill(new MarkAssignSkill("@burnheart", 1));

    General *fuwan = new General(this, "fuwan", "qun");
    fuwan->addSkill(new Moukui);

    General *xiahouba = new General(this, "xiahouba", "shu");
    xiahouba->addSkill(new Baobian);

    General *erqiao = new General(this, "erqiao", "wu", 3, false);
    erqiao->addSkill(new Xingwu);
    erqiao->addSkill(new Skill("luoyan", Skill::Compulsory));

    General *zhugeke = new General(this, "zhugeke", "wu", 3);
    zhugeke->addSkill(new Aocai);
    zhugeke->addSkill(new Duwu);
    addMetaObject<DuwuCard>();

    General *hejin = new General(this, "hejin", "qun");
    hejin->addSkill(new Mouzhu);
    hejin->addSkill(new Yanhuo);
    addMetaObject<MouzhuCard>();
/*
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

    General *tai_ganning= new General(this, "tai_ganning", "wu", 4, true, true);
    tai_ganning->addSkill("qixi");

    General *tai_lvbu= new General(this, "tai_lvbu", "qun", 4, true, true);
    tai_lvbu->addSkill("wushuang");

    General *tai_machao= new General(this, "tai_machao", "shu", 4, true, true);
    tai_machao->addSkill("tieji");
    tai_machao->addSkill("mashu");

    General *tai_zhenji= new General(this, "tai_zhenji", "wei", 3, false, true);
    tai_zhenji->addSkill("qingguo");
    tai_zhenji->addSkill("luoshen");
*/
}

ADD_PACKAGE(SP)
