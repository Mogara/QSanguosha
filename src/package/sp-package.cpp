#include "sp-package.h"
#include "general.h"
#include "skill.h"
#include "standard-skillcards.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"
#include "wisdompackage.h"

class SPMoonSpearSkill: public WeaponSkill{
public:
    SPMoonSpearSkill():WeaponSkill("SPMoonSpear"){
        events << CardFinished << CardResponsed;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;

        CardStar card = NULL;
        if(triggerEvent == CardFinished){
            CardUseStruct card_use = data.value<CardUseStruct>();
            card = card_use.card;

            if(card == player->tag["MoonSpearSlash"].value<CardStar>()){
                card = NULL;
            }
        }else if(triggerEvent == CardResponsed){
            card = data.value<ResponsedStruct>().m_card;
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
        if(!room->askForCard(target, "jink", "@moon-spear-jink", QVariant(), CardResponsed, player)){
            DamageStruct damage;
            damage.from = player;
            damage.to = target;
            room->damage(damage);
        }
        return false;
    }
};

SPMoonSpear::SPMoonSpear(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("SPMoonSpear");
    skill = new SPMoonSpearSkill;
}

class JileiClear: public PhaseChangeSkill{
public:
    JileiClear():PhaseChangeSkill("#jilei-clear"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
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
        events << DamageInflicted;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *yangxiu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from == NULL)
           return false;

        if(room->askForSkillInvoke(yangxiu, objectName(), data)){
            QString choice = room->askForChoice(yangxiu, objectName(), "basic+equip+trick");
            room->broadcastSkillInvoke(objectName());

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
        events << TargetConfirmed << CardEffected;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(triggerEvent == TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.to.length() <= 1 || !use.to.contains(player) ||
               !use.card->isKindOf("TrickCard") ||
               !room->askForSkillInvoke(player, objectName(), data))
                    return false;

            player->tag["Danlao"] = use.card->getEffectiveId();
            room->broadcastSkillInvoke(objectName());

            LogMessage log;
            log.type = "#DanlaoAvoid";
            log.from = player;
            log.arg = use.card->objectName();
            log.arg2 = objectName();

            room->sendLog(log);
            player->drawCards(1);
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
        events << DrawNCards << EventPhaseStart;
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

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *yuanshu, QVariant &data) const{
        if(triggerEvent == DrawNCards){
            int x = getKingdoms(yuanshu);
            data = data.toInt() + x;

            Room *room = yuanshu->getRoom();
            LogMessage log;
            log.type = "#YongsiGood";
            log.from = yuanshu;
            log.arg = QString::number(x);
            log.arg2 = objectName();
            room->sendLog(log);

            room->broadcastSkillInvoke("yongsi", x);

        }else if(triggerEvent == EventPhaseStart && yuanshu->getPhase() == Player::Discard){
            int x = getKingdoms(yuanshu);
            int total = 0;
            QSet<const Card *> jilei_cards;
            QList<const Card *> handcards = yuanshu->getHandcards();
            foreach(const Card *card, handcards){
                if(yuanshu->isJilei(card))
                    jilei_cards << card;
            }
            total = handcards.size() - jilei_cards.size() + yuanshu->getEquips().length();

            if(x >= total){
                if (yuanshu->hasFlag("jilei")){
                    LogMessage log;
                    log.type = "#YongsiBad";
                    log.from = yuanshu;
                    log.arg = QString::number(total);
                    log.arg2 = objectName();
                    room->sendLog(log);
                    DummyCard *dummy_card = new DummyCard;
                    foreach(const Card *card, handcards.toSet() - jilei_cards){
                        dummy_card->addSubcard(card);
                    }
                    QList<const Card *> equips = yuanshu->getEquips();
                    foreach(const Card *equip, equips)
                        dummy_card->addSubcard(equip);
                    if (dummy_card->subcardsLength() > 0)
                        room->throwCard(dummy_card, yuanshu);
                    room->showAllCards(yuanshu);
                    delete dummy_card;
                }
                else
                {
                    LogMessage log;
                    log.type = "#YongsiWorst";
                    log.from = yuanshu;
                    log.arg = QString::number(total);
                    log.arg2 = objectName();
                    room->sendLog(log);

                    yuanshu->throwAllHandCardsAndEquips();
                }
            }
            else
            {
                LogMessage log;
                log.type = "#YongsiBad";
                log.from = yuanshu;
                log.arg = QString::number(x);
                log.arg2 = objectName();
                room->sendLog(log);
                room->askForDiscard(yuanshu, "yongsi", x, x, false, true);
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
    if(yuanshu->hasLordSkill("jijiang") && room->getLord()->hasLordSkill("jijiang") && Slash::IsAvailable(yuanshu))
        choices << "jijiang";

    if(yuanshu->hasLordSkill("weidai") && room->getLord()->hasLordSkill("weidai") && Analeptic::IsAvailable(yuanshu))
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

class WeidiViewAsSkill:public ZeroCardViewAsSkill{
public:
    WeidiViewAsSkill():ZeroCardViewAsSkill("weidi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return (player->hasLordSkill("jijiang") && Slash::IsAvailable(player))
                ||(player->hasLordSkill("weidai") && Analeptic::IsAvailable(player));
    }

    virtual const Card *viewAs() const{
        return new WeidiCard;
    }
};

class Weidi:public GameStartSkill{
public:
    Weidi():GameStartSkill("weidi"){
        frequency = Compulsory;
        view_as_skill = new WeidiViewAsSkill;
    }

    virtual void onGameStart(ServerPlayer *) const{
        // do nothing
        return;
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

class YicongEffect: public TriggerSkill{
public:
    YicongEffect():TriggerSkill("#yicong_effect"){
        events << PostHpReduced << HpRecover;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        int hp = player->getHp();
        int index = 0;
        if (triggerEvent == HpRecover){
            RecoverStruct recover = data.value<RecoverStruct>();
            if (hp <= 2 && hp + recover.recover > 2)
                index = 1;
        }
        else if (triggerEvent == PostHpReduced){
            int reduce = 0;
            if (data.canConvert<DamageStruct>()) {
                DamageStruct damage = data.value<DamageStruct>();
                reduce = damage.damage;
            } else
                reduce = data.toInt();
            if (hp <= 2 && hp + reduce > 2)
                index = 2;
        }


        if (index){
            room->broadcastSkillInvoke("yicong", index);
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
    const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
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
    room->broadcastSkillInvoke("yuanhu", index);
    SkillCard::onUse(room, card_use);
}

void YuanhuCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *caohong = effect.from;
    Room *room = caohong->getRoom();
    room->moveCardTo(this, caohong, effect.to, Player::PlaceEquip,
                     CardMoveReason(CardMoveReason::S_REASON_USE, caohong->objectName(), "yuanhu", QString()));

    const Card *card = Sanguosha->getCard(subcards.first());

    LogMessage log;
    log.type = "$ZhijianEquip";
    log.from = effect.to;
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
          room->throwCard(Sanguosha->getCard(card_id), to_dismantle, caohong);
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

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isKindOf("EquipCard");
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        YuanhuCard *first = new YuanhuCard;
        first->addSubcard(originalcard->getId());
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
        if (target->getPhase() == Player::Finish && !target->isNude()) {
            room->askForUseCard(target, "@@yuanhu", "@yuanhu-equip");
        }
        return false;
    }
};

class Xiuluo: public PhaseChangeSkill{
public:
    Xiuluo():PhaseChangeSkill("xiuluo"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && PhaseChangeSkill::triggerable(target)
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
                room->throwCard(card, NULL);
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
        return target != NULL && PhaseChangeSkill::triggerable(target)
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
            room->broadcastSkillInvoke(objectName());
            room->broadcastInvoke("animate", "lightbox:$danji:5000");
            room->getThread()->delay(5000);

            guanyu->setMark("danji", 1);
			guanyu->gainMark("@waked");
            room->loseMaxHp(guanyu);
            room->acquireSkill(guanyu, "mashu");
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
    gongsunzan->addSkill(new YicongEffect);
    related_skills.insertMulti("yicong", "#yicong_effect");

    General *yuanshu = new General(this, "yuanshu", "qun");
    yuanshu->addSkill(new Yongsi);
    yuanshu->addSkill(new Weidi);

    General *sp_sunshangxiang = new General(this, "sp_sunshangxiang", "shu", 3, false, true);
    sp_sunshangxiang->addSkill("jieyin");
    sp_sunshangxiang->addSkill("xiaoji");

    General *sp_pangde = new General(this, "sp_pangde", "wei", 4, true, true);
    sp_pangde->addSkill("mengjin");
    sp_pangde->addSkill("mashu");

    General *sp_guanyu = new General(this, "sp_guanyu", "wei", 4);
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

    addMetaObject<WeidiCard>();
}

ADD_PACKAGE(SP)
