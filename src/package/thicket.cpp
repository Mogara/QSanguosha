#include "thicket.h"
#include "general.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "general.h"

class Xingshang: public TriggerSkill{
public:
    Xingshang():TriggerSkill("xingshang"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->isNude())
            return false;
        QList<ServerPlayer *> caopis = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *caopi, caopis){
            if(caopi->loseTriggerSkills())
                continue;
            if(caopi->isAlive() && room->askForSkillInvoke(caopi, objectName(), data)){
                if(player->isCaoCao()){
                    room->playSkillEffect(objectName(), 3);
                }else if(player->getGeneral()->isMale())
                    room->playSkillEffect(objectName(), 1);
                else
                    room->playSkillEffect(objectName(), 2);

                caopi->obtainCard(player->getWeapon());
                caopi->obtainCard(player->getArmor());
                caopi->obtainCard(player->getDefensiveHorse());
                caopi->obtainCard(player->getOffensiveHorse());

                DummyCard *all_cards = player->wholeHandCards();
                if(all_cards){
                    CardMoveReason reason(CardMoveReason::S_REASON_RECYCLE, caopi->objectName());
                    room->obtainCard(caopi, all_cards, reason, false);
                    delete all_cards;
                }
                break;
            }
        }
        return false;
    }
};

FangzhuCard::FangzhuCard(){
    mute = true;
}

void FangzhuCard::onEffect(const CardEffectStruct &effect) const{
    int x = effect.from->getLostHp();

    effect.to->drawCards(x);

    Room *room = effect.to->getRoom();

    int index;
    if(effect.to->faceUp()){
        QString to_exile = effect.to->getGeneralName();
        bool is_brother = to_exile == "caozhi" || to_exile == "caochong";
        index = is_brother ? 3 : 1;
    }else
        index = 2;
    room->playSkillEffect("fangzhu", index);

    effect.to->turnOver();
}

class FangzhuViewAsSkill: public ZeroCardViewAsSkill{
public:
    FangzhuViewAsSkill():ZeroCardViewAsSkill("fangzhu"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@fangzhu";
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
        events << FinishJudge;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        QList<ServerPlayer *> players = target->getRoom()->getAlivePlayers();
        ServerPlayer *lord = NULL;
        foreach(ServerPlayer *player, players){
            if(player->isLord())
                lord = player;
            if(!player->hasLordSkill(objectName()) || player->loseTriggerSkills())
                players.removeOne(player);
        }
        return target->getKingdom() == "wei" && !lord->loseTriggerSkills() && !players.empty();
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        CardStar card = judge->card;

        if(card->isBlack()){
            if (player == NULL) return false;
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            foreach(ServerPlayer *p, players){
                QVariant who = QVariant::fromValue(p);
                if(p->hasLordSkill("songwei") && !p->loseTriggerSkills() && player->askForSkillInvoke("songwei", who)){
                    if(player->getGeneral()->isMale())
                        room->playSkillEffect(objectName(), 1);
                    else
                        room->playSkillEffect(objectName(), 2);
                    p->drawCards(1);
                }
            }
        }

        return false;
    }
};

class Duanliang: public OneCardViewAsSkill{
public:
    Duanliang():OneCardViewAsSkill("duanliang"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->isBlack() && !card->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();

        SupplyShortage *shortage = new SupplyShortage(card->getSuit(), card->getNumber());
        shortage->setSkillName(objectName());
        shortage->addSubcard(card);

        return shortage;
    }
};

class SavageAssaultAvoid: public TriggerSkill{
public:
    SavageAssaultAvoid(const QString &avoid_skill)
        :TriggerSkill("#sa_avoid_" + avoid_skill), avoid_skill(avoid_skill)
    {
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("SavageAssault")){
            LogMessage log;
            log.type = "#SkillNullify";
            log.from = player;
            log.arg = avoid_skill;
            log.arg2 = "savage_assault";
            player->getRoom()->sendLog(log);

            return true;
        }else
            return false;
    }

private:
    QString avoid_skill;
};

class Huoshou: public TriggerSkill{
public:
    Huoshou():TriggerSkill("huoshou"){
        events << TargetConfirmed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("SavageAssault")){
            room->playSkillEffect(objectName());
            room->setTag("Huoshou", true);
         }

        return false;
    }
};

class Lieren: public TriggerSkill{
public:
    Lieren():TriggerSkill("lieren"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *zhurong, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = damage.to;
        if(damage.card && damage.card->inherits("Slash") && !zhurong->isKongcheng()
            && !target->isKongcheng() && target != zhurong && !damage.chain){
            Room *room = zhurong->getRoom();
            if(room->askForSkillInvoke(zhurong, objectName(), data)){
                room->playSkillEffect(objectName(), 1);

                bool success = zhurong->pindian(target, "lieren", NULL);
                if(success)
                    room->playSkillEffect(objectName(), 2);
                else{
                    room->playSkillEffect(objectName(), 3);
                    return false;
                }

                if(!target->isNude()){
                    int card_id = room->askForCardChosen(zhurong, target, "he", objectName());
                    CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, zhurong->objectName());
                    room->obtainCard(zhurong, Sanguosha->getCard(card_id), reason, room->getCardPlace(card_id) != Player::Hand);
                }
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

                room->playSkillEffect(objectName(), 1);
                bool has_heart = false;

                for(i = 0; i < x; i++){
                    int card_id = room->drawCard();
                    /* revive this after TopDrawPile works
                    room->moveCardTo(Sanguosha->getCard(card_id), NULL, NULL, Player::TopDrawPile,
                        CardMoveReason(CardMoveReason::S_REASON_TURNOVER, menghuo->objectName(), QString(), "zaiqi", QString()), true);  */
                    room->moveCardTo(Sanguosha->getCard(card_id), NULL, menghuo, Player::Special,
                        CardMoveReason(CardMoveReason::S_REASON_TURNOVER, menghuo->objectName(), QString(), "zaiqi", QString()), true);
                    room->getThread()->delay();

                    const Card *card = Sanguosha->getCard(card_id);
                    if(card->getSuit() == Card::Heart){
                        RecoverStruct recover;
                        recover.card = card;
                        recover.who = menghuo;
                        room->recover(menghuo, recover);
                        CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, menghuo->objectName(), "zaiqi", QString());
                        room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
                        has_heart = true;
                    }else{
                        CardMoveReason reason(CardMoveReason::S_REASON_GOTBACK, menghuo->objectName());
                        room->obtainCard(menghuo, Sanguosha->getCard(card_id), reason);
                    }
                }

                if(has_heart)
                    room->playSkillEffect(objectName(), 2);
                else
                    room->playSkillEffect(objectName(), 3);

                return true;
            }
        }

        return false;
    }
};

class Juxiang: public TriggerSkill{
public:
    Juxiang():TriggerSkill("juxiang"){
        events << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *zhurong = room->findPlayerBySkillName(objectName());
        if(zhurong && zhurong->loseTriggerSkills())
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("SavageAssault") &&
                ((!use.card->isVirtualCard()) ||
                  (use.card->getSubcards().length() == 1 &&
                  Sanguosha->getCard(use.card->getSubcards().first())->inherits("SavageAssault")))){
            if (player == NULL) return false;
            if(room->getCardPlace(use.card->getEffectiveId()) == Player::DiscardPile){
                // finding zhurong;
                QList<ServerPlayer *> players = room->getAllPlayers();
                foreach(ServerPlayer *p, players){
                    if(p->hasSkill(objectName())){
                        p->obtainCard(use.card);
                        room->playSkillEffect(objectName());
                        break;
                    }
                }
            }
        }

        return false;
    }
};


YinghunCard::YinghunCard(){
    mute = true;
}

void YinghunCard::onEffect(const CardEffectStruct &effect) const{
    int x = effect.from->getLostHp();
    Room *room = effect.from->getRoom();

    bool good = false;
    if(x == 1){
        room->playSkillEffect("yinghun", 1);

        effect.to->drawCards(1);
        room->askForDiscard(effect.to, "yinghun", 1, 1, false, true);
        good = true;
    }else{
        QString choice = room->askForChoice(effect.from, "yinghun", "d1tx+dxt1");
        if(choice == "d1tx"){
            room->playSkillEffect("yinghun", 2);

            effect.to->drawCards(1);
            x = qMin(x, effect.to->getCardCount(true));
            room->askForDiscard(effect.to, "yinghun", x, x, false, true);
            good = false;
        }else{
            room->playSkillEffect("yinghun", 1);

            effect.to->drawCards(x);
            room->askForDiscard(effect.to, "yinghun", 1, 1, false, true);
            good = true;
        }
    }

    if(good)
        room->setEmotion(effect.to, "good");
    else
        room->setEmotion(effect.to, "bad");
}

class YinghunViewAsSkill: public ZeroCardViewAsSkill{
public:
    YinghunViewAsSkill():ZeroCardViewAsSkill("yinghun"){
    }

    virtual const Card *viewAs() const{
        return new YinghunCard;
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@yinghun";
    }
};

class Yinghun: public PhaseChangeSkill{
public:
    Yinghun():PhaseChangeSkill("yinghun"){
        default_choice = "d1tx";

        view_as_skill = new YinghunViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->isWounded();
    }

    virtual bool onPhaseChange(ServerPlayer *sunjian) const{
        Room *room = sunjian->getRoom();
        room->askForUseCard(sunjian, "@@yinghun", "@yinghun");

        return false;
    }
};

HaoshiCard::HaoshiCard(){
    will_throw = false;
    mute = true;
}

bool HaoshiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    return to_select->getHandcardNum() == Self->getMark("haoshi");
}

void HaoshiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *beggar = targets.first();

    room->moveCardTo(this, beggar, Player::Hand, false);
    room->setEmotion(beggar, "draw-card");
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
        if(cards.length() != Self->getHandcardNum() / 2)
            return NULL;

        HaoshiCard *card = new HaoshiCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@haoshi!";
    }
};

class HaoshiGive: public PhaseChangeSkill{
public:
    HaoshiGive():PhaseChangeSkill("#haoshi-give"){

    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool onPhaseChange(ServerPlayer *lusu) const{
        if(lusu->getPhase() == Player::Draw && lusu->hasFlag("haoshi")){
            lusu->setFlags("-haoshi");

            Room *room = lusu->getRoom();
            if(lusu->getHandcardNum() <= 5)
                return false;            

            QList<ServerPlayer *> other_players = room->getOtherPlayers(lusu);
            int least = 1000;
            foreach(ServerPlayer *player, other_players)
                least = qMin(player->getHandcardNum(), least);
            room->setPlayerMark(lusu, "haoshi", least);
            bool used = room->askForUseCard(lusu, "@@haoshi!", "@haoshi");

            if(!used){
                // force lusu to give his half cards
                ServerPlayer *beggar = NULL;
                foreach(ServerPlayer *player, other_players){
                    if(player->getHandcardNum() == least){
                        beggar = player;
                        break;
                    }
                }

                int n = lusu->getHandcardNum()/2;
                QList<int> to_give = lusu->handCards().mid(0, n);
                HaoshiCard *haoshi_card = new HaoshiCard;
                foreach(int card_id, to_give)
                    haoshi_card->addSubcard(card_id);
                QList<ServerPlayer *> targets;
                targets << beggar;
                haoshi_card->use(room, lusu, targets);
                delete haoshi_card;
            }
        }

        return false;
    }
};

class Haoshi: public DrawCardsSkill{
public:
    Haoshi():DrawCardsSkill("#haoshi"){

    }

    virtual int getDrawNum(ServerPlayer *lusu, int n) const{
        Room *room = lusu->getRoom();
        if(room->askForSkillInvoke(lusu, "haoshi")){
            room->playSkillEffect("haoshi");
            lusu->setFlags("haoshi");
            return n + 2;
        }else
            return n;
    }
};

DimengCard::DimengCard(){
    once = true;
}

bool DimengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;

    if(targets.isEmpty())
        return true;

    if(targets.length() == 1){
        int max_diff = Self->getCardCount(true);
        return max_diff >= qAbs(to_select->getHandcardNum() - targets.first()->getHandcardNum());
    }

    return false;
}

bool DimengCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void DimengCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *a = targets.at(0);
    ServerPlayer *b = targets.at(1);

    int n1 = a->getHandcardNum();
    int n2 = b->getHandcardNum();

    // make sure a is front of b
    if(room->getFront(a, b) != a){
        qSwap(a, b);
        qSwap(n1, n2);
    }

    int diff = qAbs(n1 - n2);
    if(diff != 0){
        room->askForDiscard(source, "dimeng", diff, diff, false, true);
    }

    QList<CardsMoveStruct> exchangeMove;
    CardsMoveStruct move1;
    move1.card_ids = a->handCards();
    move1.to = b;
    move1.to_place = Player::Hand;
    CardsMoveStruct move2;
    move2.card_ids = b->handCards();
    move2.to = a;
    move2.to_place = Player::Hand;
    exchangeMove.push_back(move1);
    exchangeMove.push_back(move2);
    
    LogMessage log;
    log.type = "#Dimeng";
    log.from = a;
    log.to << b;
    log.arg = QString::number(n1);
    log.arg2 = QString::number(n2);
    room->sendLog(log);

    room->moveCards(exchangeMove, false);
    room->getThread()->delay();
}

class Dimeng: public ZeroCardViewAsSkill{
public:
    Dimeng():ZeroCardViewAsSkill("dimeng"){

    }

    virtual const Card *viewAs() const{
        return new DimengCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("DimengCard");
    }
};

class Luanwu: public ZeroCardViewAsSkill{
public:
    Luanwu():ZeroCardViewAsSkill("luanwu"){
        frequency = Limited;
    }

    virtual const Card *viewAs() const{
        return new LuanwuCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@chaos") >= 1;
    }
};

LuanwuCard::LuanwuCard(){
    target_fixed = true;
}

void LuanwuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->loseMark("@chaos");
    //room->broadcastInvoke("animate", "lightbox:$luanwu");

    QList<ServerPlayer *> players = room->getOtherPlayers(source);
    foreach(ServerPlayer *player, players){
        if(player->isAlive())
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
    if(!luanwu_targets.isEmpty()){
        slash = room->askForCard(effect.to, "slash", "@luanwu-slash", QVariant(), CardUsed);
        ServerPlayer *to_slash;
        int slash_targets = 1;
        if(luanwu_targets.length() == 1)
            to_slash = luanwu_targets.first();
        else
            to_slash = room->askForPlayerChosen(effect.to, luanwu_targets, "luanwu");
        if(slash){
            if(effect.to->hasWeapon("halberd") && effect.to->isLastHandCard(slash)){
                slash_targets = 3;
            }
            if(effect.to->hasSkill("shenji") && effect.to->getWeapon() == NULL)
                slash_targets = 3;
            bool distance_limit = true;
            int rangefix = 0;
            if(slash->isVirtualCard() && slash->getSubcards().length() > 0){
                foreach(int card_id, slash->getSubcards()){
                    if(Sanguosha->getCard(card_id)->inherits("Weapon")){
                        const Weapon *hisweapon = effect.to->getWeapon();
                        if(hisweapon->getRange() > 1){
                            rangefix = qMax((hisweapon->getRange()), rangefix);
                        }
                    }
                    if(Sanguosha->getCard(card_id)->inherits("OffensiveHorse")){
                        rangefix = qMax(rangefix, 1);
                    }
                }
            }
            if(effect.to->hasSkill("lihuo") && slash->inherits("FireSlash"))
                slash_targets ++;

            if(slash->inherits("WushenSlash")){
                distance_limit = false;
            }

            if(!effect.to->canSlash(to_slash, distance_limit, rangefix)){
                slash = NULL;
            }
        }
        if(slash){
            CardUseStruct use;
            use.card = slash;
            use.from = effect.to;
            use.to << to_slash;
            if(slash_targets > 1){
                luanwu_targets = room->getOtherPlayers(effect.to);
                luanwu_targets.removeOne(to_slash);
                foreach(ServerPlayer *p, luanwu_targets){
                    if(effect.to->distanceTo(p) > effect.to->getAttackRange())
                        luanwu_targets.removeOne(p);
                }

                while(slash_targets > 1 && luanwu_targets.length() > 0
                        && room->askForChoice(effect.to, "luanwu", "yes+no") == "yes"){
                    ServerPlayer *tmptarget = room->askForPlayerChosen(effect.to,luanwu_targets,"halberd");
                    use.to << tmptarget;
                    luanwu_targets.removeOne(tmptarget);
                    slash_targets--;
                }
            }
            CardMoveReason reason(CardMoveReason::S_REASON_LETUSE, effect.to->objectName());
            room->moveCardTo(slash, effect.to, NULL, Player::DiscardPile, reason);
            room->useCard(use);
        }
        else
           room->loseHp(effect.to);
    }else
        room->loseHp(effect.to);
}

class Wansha: public TriggerSkill{
public:
    Wansha():TriggerSkill("wansha"){
        events << Dying;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return (target != NULL);
    }

    virtual bool trigger(TriggerEvent , Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *jiaxu = room->findPlayerBySkillName(objectName());
        if(jiaxu && !jiaxu->loseTriggerSkills())
            room->playSkillEffect(objectName());
        return false;
    }
};

class Weimu: public ProhibitSkill{
public:
    Weimu():ProhibitSkill("weimu"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        if(to->loseProhibitSkills())
            return false;
        return card->inherits("TrickCard") && card->isBlack();
    }
};

class Jiuchi: public OneCardViewAsSkill{
public:
    Jiuchi():OneCardViewAsSkill("jiuchi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Analeptic::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern.contains("analeptic");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Analeptic *analeptic = new Analeptic(card->getSuit(), card->getNumber());
        analeptic->setSkillName(objectName());
        analeptic->addSubcard(card->getId());

        return analeptic;
    }
};

class Roulin: public TriggerSkill{
public:
    Roulin():TriggerSkill("roulin"){
        events << SlashProceed;

        frequency = Compulsory;
    }

    const Card *askForDoubleJink(ServerPlayer *player, const QString &reason) const{
        Room *room = player->getRoom();

        const Card *first_jink = NULL, *second_jink = NULL;
        first_jink = room->askForCard(player, "jink", QString("@%1-jink-1").arg(reason), QVariant(), CardUsed);
        if(first_jink)
            second_jink = room->askForCard(player, "jink", QString("@%1-jink-2").arg(reason), QVariant(), CardUsed);

        Card *jink = NULL;
        if(first_jink && second_jink){
            jink = new DummyCard;
            jink->addSubcard(first_jink);
            jink->addSubcard(second_jink);
        }

        return jink;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName()) || target->getGeneral()->isFemale();
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *, QVariant &data) const{
        ServerPlayer *dongzhuo = room->findPlayerBySkillName(objectName());
        if(dongzhuo && dongzhuo->loseTriggerSkills())
            return false;
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.from->hasSkill(objectName()) && effect.to->getGeneral()->isFemale()){
            // dongzhuo slash female
            ServerPlayer *dongzhuo = effect.from;
            ServerPlayer *female = effect.to;
            Room *room = dongzhuo->getRoom();

            room->playSkillEffect(objectName(), 1);

            room->slashResult(effect, askForDoubleJink(female, "roulin1"));
            return true;

        }else if(effect.from->getGeneral()->isFemale() && effect.to->hasSkill(objectName())){
            // female slash dongzhuo

            ServerPlayer *female = effect.from;
            ServerPlayer *dongzhuo = effect.to;
            Room *room = female->getRoom();

            int index = effect.drank ? 3 : 2;
            room->playSkillEffect(objectName(), index);
            room->slashResult(effect, askForDoubleJink(dongzhuo, "roulin2"));

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

    virtual QString getDefaultChoice(ServerPlayer *player) const{
        if(player->getMaxHp() >= player->getHp() + 2)
            return "maxhp";
        else
            return "hp";
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
            QString result = room->askForChoice(dongzhuo, "benghuai", "hp+maxhp");

            int index = dongzhuo->getGeneral()->isFemale() ? 2: 1;
            room->playSkillEffect(objectName(), index);
            room->setEmotion(dongzhuo, "bad");

            LogMessage log;
            log.from = dongzhuo;
            log.arg = objectName();
            log.type = "#TriggerSkill";
            room->sendLog(log);
            if(result == "hp")
                room->loseHp(dongzhuo);
            else
                room->loseMaxHp(dongzhuo);
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
        QList<ServerPlayer *> players = target->getRoom()->getAlivePlayers();
        ServerPlayer *lord = NULL;
        foreach(ServerPlayer *player, players){
            if(player->isLord())
                lord = player;
            if(!player->hasLordSkill(objectName()) || player->loseTriggerSkills())
                players.removeOne(player);
        }
        return target->getKingdom() == "qun" && !lord->loseTriggerSkills() && !players.empty();
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
        if (player == NULL) return false;
        QList<ServerPlayer *> dongzhuos;
        QList<ServerPlayer *> players = room->getOtherPlayers(player);
        foreach(ServerPlayer *p, players){
            if(p->hasLordSkill(objectName()) && !p->loseTriggerSkills()){
                dongzhuos << p;
            }
        }
        if(dongzhuos.empty())
            return false;
        foreach(ServerPlayer *dongzhuo, dongzhuos){
            QVariant who = QVariant::fromValue(dongzhuo);
            if(player->askForSkillInvoke(objectName(), who)){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade):(.*)");
                judge.good = true;
                judge.reason = "baonue";
                judge.who = player;

                room->judge(judge);

                if(judge.isGood()){
                    room->playSkillEffect(objectName());

                    RecoverStruct recover;
                    recover.who = player;
                    room->recover(dongzhuo, recover);
                }
            }
        }

        return false;
    }
};

ThicketPackage::ThicketPackage()
    :Package("thicket")
{
    General *xuhuang, *caopi, *menghuo, *zhurong, *sunjian, *lusu, *dongzhuo, *jiaxu;

    xuhuang = new General(this, "xuhuang", "wei");
    xuhuang->addSkill(new Duanliang);

    caopi = new General(this, "caopi$", "wei", 3);
    caopi->addSkill(new Xingshang);
    caopi->addSkill(new Fangzhu);
    caopi->addSkill(new Songwei);

    menghuo = new General(this, "menghuo", "shu");
    menghuo->addSkill(new SavageAssaultAvoid("huoshou"));
    menghuo->addSkill(new Huoshou);
    menghuo->addSkill(new Zaiqi);
    related_skills.insertMulti("huoshou", "#sa_avoid_huoshou");

    zhurong = new General(this, "zhurong", "shu", 4, false);
    zhurong->addSkill(new SavageAssaultAvoid("juxiang"));
    zhurong->addSkill(new Juxiang);
    zhurong->addSkill(new Lieren);
    related_skills.insertMulti("juxiang", "#sa_avoid_juxiang");

    sunjian = new General(this, "sunjian", "wu");
    sunjian->addSkill(new Yinghun);

    lusu = new General(this, "lusu", "wu", 3);
    lusu->addSkill(new Haoshi);
    lusu->addSkill(new HaoshiViewAsSkill);
    lusu->addSkill(new HaoshiGive);
    lusu->addSkill(new Dimeng);
    related_skills.insertMulti("haoshi", "#haoshi");
    related_skills.insertMulti("haoshi", "#haoshi-give");

    dongzhuo = new General(this, "dongzhuo$", "qun", 8);
    dongzhuo->addSkill(new Jiuchi);
    dongzhuo->addSkill(new Roulin);
    dongzhuo->addSkill(new Benghuai);
    dongzhuo->addSkill(new Baonue);

    jiaxu = new General(this, "jiaxu", "qun", 3);
    jiaxu->addSkill(new Wansha);
    jiaxu->addSkill(new MarkAssignSkill("@chaos", 1));
    jiaxu->addSkill(new Luanwu);
    jiaxu->addSkill(new Weimu);
    jiaxu->addSkill(new SPConvertSkill("guiwei", "jiaxu", "sp_jiaxu"));
    related_skills.insertMulti("luanwu", "#@chaos-1");

    addMetaObject<DimengCard>();
    addMetaObject<LuanwuCard>();
    addMetaObject<YinghunCard>();
    addMetaObject<FangzhuCard>();
    addMetaObject<HaoshiCard>();
}

ADD_PACKAGE(Thicket)
