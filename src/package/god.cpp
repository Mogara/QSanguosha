#include "god.h"
#include "client.h"
#include "engine.h"
#include "settings.h"
#include "maneuvering.h"
#include "general.h"

GongxinCard::GongxinCard(){
}

bool GongxinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self; 
}

void GongxinCard::onEffect(const CardEffectStruct &effect) const{
    if (!effect.to->isKongcheng())
        effect.from->getRoom()->doGongxin(effect.from, effect.to);
}

class Wuhun: public TriggerSkill{
public:
    Wuhun():TriggerSkill("wuhun"){
        events << Damaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from && damage.from != player){
            damage.from->gainMark("@nightmare", damage.damage);
            damage.from->getRoom()->broadcastSkillInvoke(objectName(), 1);
        }

        return false;
    }
};

class WuhunRevenge: public TriggerSkill{
public:
    WuhunRevenge():TriggerSkill("#wuhun"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasSkill("wuhun");
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *shenguanyu, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if (death.who != shenguanyu)
            return false;

        QList<ServerPlayer *> players = room->getOtherPlayers(shenguanyu);

        int max = 0;
        foreach(ServerPlayer *player, players){
            max = qMax(max, player->getMark("@nightmare"));
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
            foe = room->askForPlayerChosen(shenguanyu, foes, "wuhun");

        JudgeStruct judge;
        judge.pattern = QRegExp("(Peach|GodSalvation):(.*):(.*)");
        judge.good = true;
        judge.negative = true;
        judge.reason = "wuhun";
        judge.who = foe;
        judge.play_animation = true;

        room->judge(judge);

        if(judge.isBad()){
            LogMessage log;
            log.type = "#WuhunRevenge";
            log.from = shenguanyu;
            log.to << foe;
            log.arg = QString::number(max);
            log.arg2 = "wuhun";
            room->sendLog(log);

            room->killPlayer(foe);
            room->broadcastSkillInvoke("wuhun", 2);
        }else
            room->broadcastSkillInvoke("wuhun", 3);
        QList<ServerPlayer *> killers = room->getAllPlayers();
        foreach(ServerPlayer *player, killers){
            player->loseAllMarks("@nightmare");
        }
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

    virtual bool onPhaseChange(ServerPlayer *shenlvmeng) const{
        if(shenlvmeng->getPhase() != Player::Draw)
            return false;

        Room *room = shenlvmeng->getRoom();
        if(!shenlvmeng->askForSkillInvoke(objectName()))
            return false;

        room->broadcastSkillInvoke(objectName());

        QList<int> card_ids = room->getNCards(5);
        qSort(card_ids.begin(), card_ids.end(), CompareBySuit);
        room->fillAG(card_ids);

        while(!card_ids.isEmpty()){
            int card_id = room->askForAG(shenlvmeng, card_ids, false, "shelie");
            card_ids.removeOne(card_id);
            // throw the rest cards that matches the same suit
            const Card *card = Sanguosha->getCard(card_id);
            Card::Suit suit = card->getSuit();

            room->takeAG(shenlvmeng, card_id);

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

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("GongxinCard");
    }
};

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
    mute = true;
    m_skillName = "yeyan";
}

bool GreatYeyanCard::targetFilter(const QList<const Player *> &targets,
                                  const Player *to_select, const Player *Self) const
{
    Q_ASSERT(false);
    return false;
}

bool GreatYeyanCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    // check legitimacy
    // first, we need 4 suits
    if (subcards.length() != 4) return false;
    QList<Card::Suit> allsuits;
    foreach (int cardId, subcards)
    {
        // @todo: get this from room / roomscene rather than Sanguosha
        const Card* card = Sanguosha->getCard(cardId);
        if (allsuits.contains(card->getSuit())) return false;
        allsuits.append(card->getSuit());
    }
    
    //We can only assign 2 damage to one player
    //If we select only one target only once, we assign 3 damage to the target
    if(targets.toSet().size() == 1)
        return true;
    else if(targets.toSet().size() == 2)
        return targets.size() == 3;
    return false;
}

bool GreatYeyanCard::targetFilter(const QList<const Player *> &targets,
                                  const Player *to_select, const Player *Self,
                                  int& maxVotes) const
{
    int i = 0;
    foreach(const Player* player, targets)
        if(player == to_select) i++;
    maxVotes = qMax(3 - targets.size(),0) + i;
    return maxVotes > 0;
}

void GreatYeyanCard::use(Room *room, ServerPlayer *shenzhouyu, QList<ServerPlayer *> &targets) const{
    int criticaltarget = 0;
    int totalvictim = 0;
    QMap<ServerPlayer*,int> map;

    foreach(ServerPlayer* sp, targets)
        map[sp]++;

    if (targets.size() == 1)
        map[targets.first()] += 2;

    foreach(ServerPlayer* sp,map.keys()){
        if(map[sp] > 1)
            criticaltarget++;
        totalvictim++;
    }
    if (criticaltarget > 0){
        room->loseHp(shenzhouyu, 3);
        shenzhouyu->loseMark("@flame");
        if(totalvictim > 1){
            room->broadcastInvoke("animate", "lightbox:$YeyanAnimate2");
            room->broadcastSkillInvoke("yeyan", 2);
        }
        else{
            room->broadcastInvoke("animate", "lightbox:$YeyanAnimate3");
            room->broadcastSkillInvoke("yeyan", 3);
        }

        QList<ServerPlayer *>targets = map.keys();
        if(targets.size() > 1)
            qSort(targets.begin(), targets.end(), ServerPlayer::CompareByActionOrder);
        foreach(ServerPlayer* sp, targets)
            damage(shenzhouyu, sp, map[sp]);
    }
}

SmallYeyanCard::SmallYeyanCard(){
    mute = true;
    m_skillName = "yeyan";
}

bool SmallYeyanCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    return !targets.isEmpty();
}

bool SmallYeyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.length() < 3;
}

void SmallYeyanCard::use(Room *room, ServerPlayer *shenzhouyu, QList<ServerPlayer *> &targets) const{
    room->broadcastInvoke("animate", "lightbox:$YeyanAnimate1");
    room->broadcastSkillInvoke("yeyan", 1);
    shenzhouyu->loseMark("@flame");
    Card::use(room, shenzhouyu, targets);
}

void SmallYeyanCard::onEffect(const CardEffectStruct &effect) const{
    damage(effect.from, effect.to, 1);
}

class Yeyan: public ViewAsSkill{
public:
    Yeyan(): ViewAsSkill("yeyan"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@flame") >= 1;
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if(selected.length() >= 4)
            return false;

        if (to_select->isEquipped())
            return false;

        if (Self->isJilei(to_select))
            return false;

        foreach (const Card *item, selected){
            if (to_select->getSuit() == item->getSuit())
                return false;
        }

        return true;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length()  == 0) 
            return new SmallYeyanCard;
        if (cards.length() != 4)
            return NULL;

        GreatYeyanCard *card = new GreatYeyanCard;
        card->addSubcards(cards);

        return card;
    }
};

class Qinyin: public TriggerSkill{
public:
    Qinyin():TriggerSkill("qinyin"){
        events << CardsMoveOneTime << EventPhaseStart;
        default_choice = "down";
    }

    void perform(ServerPlayer *shenzhouyu) const{
        Room *room = shenzhouyu->getRoom();
        QString result = room->askForChoice(shenzhouyu, objectName(), "up+down");
        QList<ServerPlayer *> all_players = room->getAllPlayers();
        if(result == "up"){
            room->broadcastSkillInvoke(objectName(), 2);
            foreach(ServerPlayer *player, all_players){
                RecoverStruct recover;
                recover.who = shenzhouyu;
                room->recover(player, recover);
            }
        }else if(result == "down"){
            foreach(ServerPlayer *player, all_players){
                room->loseHp(player);
            }

            int index = 1;
            if(room->findPlayer("caocao+shencaocao+weiwudi"))
                index = 3;

            room->broadcastSkillInvoke(objectName(), index);
        }
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *shenzhouyu, QVariant &data) const{
        if(shenzhouyu->getPhase() != Player::Discard)
            return false;

        if(triggerEvent == CardsMoveOneTime){
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            if (move->from == shenzhouyu && move->to_place == Player::DiscardPile
                && (move->reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD) {
                shenzhouyu->setMark("qinyin", shenzhouyu->getMark("qinyin") + move->card_ids.size());
                if(!shenzhouyu->hasFlag("qinyin_used") && shenzhouyu->getMark("qinyin") >= 2){
                    if(shenzhouyu->askForSkillInvoke(objectName())){
                        room->setPlayerFlag(shenzhouyu, "qinyin_used");
                        perform(shenzhouyu);
                    }
                }
            }
        }else if(triggerEvent == EventPhaseStart){
            shenzhouyu->setMark("qinyin", 0);
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
        bool can_invoke = false;
        QList<ServerPlayer *> players = room->getAllPlayers();
        players.removeOne(shencc);
        for(i=0; i<x; i++){
            foreach(ServerPlayer *player, players){
                if(!player->isAllNude()){
                    can_invoke = true;
                    break;
                }
            }
            if(can_invoke && shencc->askForSkillInvoke(objectName())){
                shencc->setFlags("GuixinUsing");
                room->broadcastSkillInvoke(objectName());

                QList<ServerPlayer *> players = room->getOtherPlayers(shencc);
                if(players.length() >= 4) {
                    room->broadcastInvoke("animate", "lightbox:$GuixinAnimate");
                    room->getThread()->delay(1500);
                }

                foreach (ServerPlayer *player, players) {
                    if (player->isAlive() && !player->isAllNude()) {
                        CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, shencc->objectName());
                        int card_id = room->askForCardChosen(shencc, player, "hej", objectName());

                        room->obtainCard(shencc, Sanguosha->getCard(card_id), reason, room->getCardPlace(card_id) != Player::PlaceHand);
                    }
                }
                can_invoke = false;
                shencc->turnOver();
                shencc->setFlags("-GuixinUsing");
            }else
                break;
        }
    }
};

class Feiying: public DistanceSkill{
public:
    Feiying():DistanceSkill("feiying"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(to->hasSkill(objectName()))
            return +1;
        else
            return 0;
    }
};

class Kuangbao: public TriggerSkill{
public:
    Kuangbao():TriggerSkill("kuangbao"){
        events << Damage << Damaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        LogMessage log;
        log.type = triggerEvent == Damage ? "#KuangbaoDamage" : "#KuangbaoDamaged";
        log.from = player;
        log.arg = QString::number(damage.damage);
        log.arg2 = objectName();
        room->sendLog(log);

        player->gainMark("@wrath", damage.damage);
        room->broadcastSkillInvoke(objectName());

        return false;
    }
};

class Wumou:public TriggerSkill{
public:
    Wumou():TriggerSkill("wumou"){
        frequency = Compulsory;
        events << CardUsed << CardResponded;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardStar card = NULL;
        if(triggerEvent == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(triggerEvent == CardResponded)
            card = data.value<ResponsedStruct>().m_card;

        if(card->isNDTrick()){
            room->broadcastSkillInvoke(objectName());

            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);

            int num = player->getMark("@wrath");
            if(num >= 1 && room->askForChoice(player, objectName(), "discard+losehp") == "discard"){
                player->loseMark("@wrath");
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

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@wrath") >= 6 && !player->hasUsed("ShenfenCard");
    }

    virtual const Card *viewAs() const{
        return new ShenfenCard;
    }
};


ShenfenCard::ShenfenCard(){
    target_fixed = true;
    mute = true;
}

void ShenfenCard::use(Room *room, ServerPlayer *shenlvbu, QList<ServerPlayer *> &) const{
    room->setPlayerFlag(shenlvbu, "ShenfenUsing");
    int index = qrand() % 2 + 1;
    room->broadcastSkillInvoke("shenfen", index);
    room->broadcastInvoke("animate", QString("lightbox:$ShenfenAnimate%1").arg(index));
    room->getThread()->delay(4500);
    shenlvbu->loseMark("@wrath", 6);

    QList<ServerPlayer *> players = room->getOtherPlayers(shenlvbu);
    foreach(ServerPlayer *player, players){
        DamageStruct damage;
        damage.card = this;
        damage.from = shenlvbu;
        damage.to = player;

        room->damage(damage);
        room->getThread()->delay();
    }

    foreach (ServerPlayer *player, players) {
        QList<const Card *> equips = player->getEquips();
        player->throwAllEquips();
        if (!equips.isEmpty())
            room->getThread()->delay();
    }

    foreach (ServerPlayer *player, players) {
        DummyCard *card = player->wholeHandCards();
        room->askForDiscard(player, "shenfen", 4, 4);
        if (card != NULL)
            room->getThread()->delay();
    }

    shenlvbu->turnOver();
    room->setPlayerFlag(shenlvbu, "-ShenfenUsing");
}

WuqianCard::WuqianCard(){
}

bool WuqianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self ; 
}

void WuqianCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    effect.from->loseMark("@wrath", 2);
    room->acquireSkill(effect.from, "wushuang", false);
    room->setPlayerFlag(effect.to,"wuqian");
}

class WuqianViewAsSkill: public ZeroCardViewAsSkill{
public:
    WuqianViewAsSkill():ZeroCardViewAsSkill("wuqian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@wrath") >= 2;
    }

    virtual const Card *viewAs() const{
        return new WuqianCard;
    }
};

class Wuqian: public TriggerSkill {
public:
    Wuqian(): TriggerSkill("wuqian") {
        events << EventPhaseChanging << Death;
        view_as_skill = new WuqianViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasSkill("wuqian");
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
        }
        if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player)
                return false;
        }

        foreach (ServerPlayer *p , room->getAllPlayers()) {
            if (p->hasFlag("wuqian")) {
                room->setPlayerFlag(p, "-wuqian");
            }
        }
        room->detachSkillFromPlayer(player, "wushuang");

        return false;
    }
};


class Wushen: public FilterSkill{
public:
    Wushen():FilterSkill("wushen"){

    }

    virtual bool viewFilter(const Card* to_select) const{
        Room *room = Sanguosha->currentRoom();
        Player::Place place = room->getCardPlace(to_select->getEffectiveId());
        return to_select->getSuit() == Card::Heart && place == Player::PlaceHand;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Slash *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
        slash->setSkillName(objectName());
        WrappedCard *card = Sanguosha->getWrappedCard(originalCard->getId());
        card->takeOver(slash);
        return card;
    }
};

class WushenTargetMod: public TargetModSkill {
public:
    WushenTargetMod(): TargetModSkill("#wushen-target") {
    }

    virtual int getDistanceLimit(const Player *from, const Card *card) const{
        if (from->hasSkill("wushen") && card->getSuit() == Card::Heart)
            return 1000;
        else
            return 0;
    }
};

class Qixing: public TriggerSkill{
public:
    Qixing():TriggerSkill("qixing"){
        frequency = Frequent;
        events << EventPhaseEnd << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    static void Exchange(ServerPlayer *shenzhuge){
        QList<int> stars = shenzhuge->getPile("stars");
        if (stars.isEmpty()) return;

        shenzhuge->getRoom()->broadcastSkillInvoke("qixing");
        shenzhuge->exchangeFreelyFromPrivatePile("qixing", "stars");
    }

    static void DiscardStar(ServerPlayer *shenzhuge, int n, QString skillName){
        Room *room = shenzhuge->getRoom();
        QList<int> stars = shenzhuge->getPile("stars");


        for(int i = 0; i < n; i++){
            room->fillAG(stars, shenzhuge);
            int card_id = room->askForAG(shenzhuge, stars, false, "qixing-discard");
            shenzhuge->invoke("clearAG");
            stars.removeOne(card_id);
            CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), skillName, QString());
            room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
        }

    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *, ServerPlayer *shenzhuge, QVariant &data) const{
        if(triggerEvent == EventPhaseEnd && TriggerSkill::triggerable(shenzhuge)
                && shenzhuge->getPile("stars").length() > 0 && shenzhuge->getPhase() == Player::Draw)
            Exchange(shenzhuge);
        else if(triggerEvent == EventLoseSkill && data.toString() == objectName()){
            shenzhuge->removePileByName("stars");
        }
        return false;
    }
};

class QixingStart: public GameStartSkill{
public:
    QixingStart():GameStartSkill("#qixing"){

    }

    virtual void onGameStart(ServerPlayer *shenzhuge) const{
        Room *room = shenzhuge->getRoom();

        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = shenzhuge;
        log.arg = "qixing";
        room->sendLog(log);

        room->setTag("FirstRound", true); //For Manjuan
        shenzhuge->drawCards(7);
        room->setTag("FirstRound", false);

        room->broadcastSkillInvoke("qixing");
        const Card *exchange_card = room->askForExchange(shenzhuge, "qixing", 7);
        shenzhuge->addToPile("stars", exchange_card->getSubcards(), false);
        delete exchange_card;
    }
};

KuangfengCard::KuangfengCard() {
    handling_method = Card::MethodNone;
}

bool KuangfengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void KuangfengCard::onEffect(const CardEffectStruct &effect) const{
    Qixing::DiscardStar(effect.from, 1, "kuangfeng");
    effect.from->tag["Qixing_user"] = true;
    effect.to->gainMark("@gale");
}

class KuangfengViewAsSkill: public ZeroCardViewAsSkill{
public:
    KuangfengViewAsSkill():ZeroCardViewAsSkill("kuangfeng"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@kuangfeng";
    }

    virtual const Card *viewAs() const{
        return new KuangfengCard;
    }
};

class Kuangfeng: public TriggerSkill{
public:
    Kuangfeng(): TriggerSkill("kuangfeng") {
        events << DamageForseen;
        view_as_skill = new KuangfengViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->getMark("@gale") > 0;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Fire){
            LogMessage log;
            log.type = "#GalePower";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(++damage.damage);
            room->sendLog(log);

            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class QixingAsk: public PhaseChangeSkill{
public:
    QixingAsk():PhaseChangeSkill("#qixing-ask"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        if(target->getPhase() == Player::Finish){
            if(target->getPile("stars").length() > 0 && target->hasSkill("kuangfeng"))
                room->askForUseCard(target, "@@kuangfeng", "@kuangfeng-card", -1, Card::MethodNone);

            if(target->getPile("stars").length() > 0 && target->hasSkill("dawu"))
                room->askForUseCard(target, "@@dawu", "@dawu-card", -1, Card::MethodNone);
        }
        return false;
    }
};

class QixingClear: public TriggerSkill{
public:
    QixingClear():TriggerSkill("#qixing-clear"){
        events << Death << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseStart || triggerEvent == Death) {
            if (triggerEvent == Death) {
                DeathStruct death = data.value<DeathStruct>();
                if (death.who != player)
                    return false;
            }
            if (!player->tag.value("Qixing_user", false).toBool())
                return false;
            bool invoke = false;
            if ((triggerEvent == EventPhaseStart && player->getPhase() == Player::RoundStart) || triggerEvent == Death)
                invoke = true;
            if (!invoke)
                return false;
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *player, players){
                player->loseAllMarks("@gale");
                player->loseAllMarks("@fog");
            }
            player->tag.remove("Qixing_user");
        }

        return false;
    }
};

DawuCard::DawuCard() {
    handling_method = Card::MethodNone;
}

bool DawuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() < Self->getPile("stars").length();
}

void DawuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    int n = targets.length();
    Qixing::DiscardStar(source, n, "dawu");
    source->tag["Qixing_user"] = true;

    foreach(ServerPlayer *target, targets){
        target->gainMark("@fog");
    }
}

class DawuViewAsSkill: public ZeroCardViewAsSkill{
public:
    DawuViewAsSkill():ZeroCardViewAsSkill("dawu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@dawu";
    }

    virtual const Card *viewAs() const{
        return new DawuCard;
    }
};

class Dawu: public TriggerSkill{
public:
    Dawu(): TriggerSkill("dawu") {
        events << DamageForseen;
        view_as_skill = new DawuViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->getMark("@fog") > 0;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.nature != DamageStruct::Thunder) {
            LogMessage log;
            log.type = "#FogProtect";
            log.from = player;
            log.arg = QString::number(damage.damage);
            if(damage.nature == DamageStruct::Normal)
                log.arg2 = "normal_nature";
            else if(damage.nature == DamageStruct::Fire)
                log.arg2 = "fire_nature";
            room->sendLog(log);

            return true;
        }else
            return false;
    }
};

class Renjie: public TriggerSkill{
public:
    Renjie():TriggerSkill("renjie"){
        events << Damaged << CardsMoveOneTime;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == CardsMoveOneTime) {
            if (player->getPhase() == Player::Discard) {
                CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
                if (move->from == player && move->to_place == Player::DiscardPile
                    && (move->reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD) {
                    int n = move->card_ids.length();
                    if (n > 0) {
                        room->broadcastSkillInvoke(objectName());
                        player->gainMark("@bear", n);
                    }
                }
            }
        }else if(triggerEvent == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            room->broadcastSkillInvoke(objectName());
            player->gainMark("@bear", damage.damage);
        }

        return false;
    }
};

class Baiyin: public PhaseChangeSkill{
public:
    Baiyin():PhaseChangeSkill("baiyin"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("baiyin") == 0
                && target->getMark("@bear") >= 4;
    }

    virtual bool onPhaseChange(ServerPlayer *shensimayi) const{
        Room *room = shensimayi->getRoom();

        LogMessage log;
        log.type = "#BaiyinWake";
        log.from = shensimayi;
        log.arg = QString::number(shensimayi->getMark("@bear"));
        room->sendLog(log);
        
        room->broadcastSkillInvoke(objectName());
        room->broadcastInvoke("animate", "lightbox:$BaiyinAnimate");
        room->getThread()->delay(2000);

        room->setPlayerMark(shensimayi, "baiyin", 1);
        shensimayi->gainMark("@waked");
        room->loseMaxHp(shensimayi);
        room->acquireSkill(shensimayi, "jilve");

        return false;
    }
};

JilveCard::JilveCard(){
    target_fixed = true;
    mute = true;
}

void JilveCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *shensimayi = card_use.from;

    QStringList choices;
    choices << "cancel";
    if(!shensimayi->hasUsed("ZhihengCard") && !shensimayi->hasSkill("zhiheng"))
        choices << "zhiheng";

    if(!shensimayi->tag.value("JilveWansha", false).toBool() && !shensimayi->hasSkill("wansha"))
        choices << "wansha";

    if(choices.isEmpty())
        return;

    QString choice = room->askForChoice(shensimayi, "jilve", choices.join("+"));

    if(choice == "wansha"){
        shensimayi->loseMark("@bear");
        room->acquireSkill(shensimayi, "wansha");
        room->broadcastSkillInvoke("jilve", 3);
        shensimayi->tag["JilveWansha"] = true;
    }else if (choice == "zhiheng"){
        if (room->askForUseCard(shensimayi, "@zhiheng", "@jilve-zhiheng"))
        {
            shensimayi->loseMark("@bear");
            room->broadcastSkillInvoke("jilve", 4);
        }
    }
}

// wansha & zhiheng
class JilveViewAsSkill: public ZeroCardViewAsSkill{
public:
    JilveViewAsSkill(): ZeroCardViewAsSkill("jilve") {

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        int times = 0;
        if(player->hasUsed("ZhihengCard"))
            times++;
        else if(player->tag.value("JilveWansha", false).toBool())
            times++;
        return times < 2 && player->getMark("@bear") > 0;
    }

    virtual const Card *viewAs() const{
        return new JilveCard;
    }
};

class Jilve: public TriggerSkill{
public:
    Jilve():TriggerSkill("jilve"){
        events << CardUsed << CardResponded // jizhi
                << AskForRetrial // guicai
                << Damaged; // fangzhu

        view_as_skill = new JilveViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && TriggerSkill::triggerable(target)
                && target->getMark("@bear") > 0;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        player->setMark("JilveEvent",(int)triggerEvent);
        if(triggerEvent == CardUsed || triggerEvent == CardResponded){
            CardStar card = NULL;
            if(triggerEvent == CardUsed)
                card = data.value<CardUseStruct>().card;
            else
                card = data.value<ResponsedStruct>().m_card;

            if(card->isNDTrick() && !player->hasSkill("jizhi") && player->askForSkillInvoke("jilve", data)){
                player->loseMark("@bear");
                room->broadcastSkillInvoke("jilve", 5);
                player->drawCards(1);
            }
        }else if(triggerEvent == AskForRetrial){
            if(!player->isKongcheng() && !player->hasSkill("guicai") && player->askForSkillInvoke("jilve", data)){
                JudgeStar judge = data.value<JudgeStar>();

                QStringList prompt_list;
                prompt_list << "@jilve-guicai" << judge->who->objectName()
                        << objectName() << judge->reason << judge->card->getEffectIdString();
                QString prompt = prompt_list.join(":");
                const Card *card = room->askForCard(player, "@guicai", prompt, data, Card::MethodResponse, judge->who, true);
                if (card != NULL){
                    room->broadcastSkillInvoke("jilve", 1);
                    player->loseMark("@bear");
                    room->retrial(card, player, judge, objectName());
                }

                return false;
            }
        }else if(triggerEvent == Damaged){
            if(!player->hasSkill("fangzhu") && player->askForSkillInvoke("jilve", data)){
                if (room->askForUseCard(player, "@@fangzhu", "@jilve-fangzhu"))
                {
                    player->loseMark("@bear");
                    room->broadcastSkillInvoke("jilve",2);
                }
            }
        }
        player->setMark("JilveEvent",0);
        return false;
    }

    virtual Location getLocation() const{
        return Right;
    }
};

class JilveClear: public TriggerSkill {
public:
    JilveClear(): TriggerSkill("#jilve-clear") {
        events << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && TriggerSkill::triggerable(target)
               && target->tag.value("JilveWansha").toBool();
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *target, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive)
            return false;
        room->detachSkillFromPlayer(target, "wansha");
        target->tag.remove("JilveWansha");
        return false;
    }
};

class LianpoCount: public TriggerSkill {
public:
    LianpoCount(): TriggerSkill("#lianpo-count") {
        events << Death << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player)
                return false;
            ServerPlayer *killer = death.damage ? death.damage->from : NULL;

            if (killer && room->getCurrent()->isAlive()) {
                killer->addMark("lianpo");

                if (killer->hasSkill("lianpo")) {
                    LogMessage log;
                    log.type = "#LianpoRecord";
                    log.from = killer;
                    log.to << player;

                    log.arg = room->getCurrent()->getGeneralName();
                    room->sendLog(log);
                }
            }
        } else if (player->getPhase() == Player::NotActive)
                foreach(ServerPlayer *p, room->getAlivePlayers())
                    room->setPlayerMark(p, "lianpo", 0);

        return false;
    }
};

class Lianpo: public TriggerSkill {
public:
    Lianpo(): TriggerSkill("lianpo") {
        events << EventPhaseChanging;
    }

    virtual int getPriority() const{
        return 1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive)
            return false;
        ServerPlayer *shensimayi = room->findPlayerBySkillName("lianpo");
        if(shensimayi == NULL || shensimayi->getMark("lianpo") <= 0)
            return false;

        int n = shensimayi->getMark("lianpo");
        shensimayi->setMark("lianpo", 0);

        if (!shensimayi->askForSkillInvoke("lianpo"))
            return false;

        LogMessage log;
        log.type = "#LianpoCanInvoke";
        log.from = shensimayi;
        log.arg = QString::number(n);
        log.arg2 = objectName();
        room->sendLog(log);
        room->broadcastSkillInvoke(objectName());

        PlayerStar p = shensimayi;
        room->setTag("LianpoInvoke", QVariant::fromValue(p));

        return false;
    }
};

class LianpoDo: public PhaseChangeSkill{
public:
    LianpoDo():PhaseChangeSkill("#lianpo-do"){
    }

    virtual int getPriority() const{
        return 1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->getPhase() == Player::NotActive;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if (!room->getTag("LianpoInvoke").isNull()) {
            PlayerStar target = room->getTag("LianpoInvoke").value<PlayerStar>();
            room->removeTag("LianpoInvoke");
            if (target->isAlive()) {
                room->broadcastSkillInvoke("lianpo");
                target->gainAnExtraTurn();
            }
        }
        return false;
    }
};

class Juejing: public DrawCardsSkill{
public:
    Juejing():DrawCardsSkill("#juejing-draw"){
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        if(player->isWounded())
            player->getRoom()->broadcastSkillInvoke("juejing");
        return n + player->getLostHp();
    }
};

class JuejingKeep: public MaxCardsSkill{
public:
    JuejingKeep():MaxCardsSkill("juejing"){
    }

    virtual int getExtra(const Player *target) const{
        if(target->hasSkill(objectName()))
            return 2;
        else
            return 0;
    }
};

class Longhun: public ViewAsSkill{
public:
    Longhun():ViewAsSkill("longhun"){

    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "slash"
                || pattern == "jink"
                || pattern.contains("peach")
                || pattern == "nullification";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->isWounded() || Slash::IsAvailable(player);
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *card) const{
        int n = qMax(1, Self->getHp());

        if(selected.length() >= n)
            return false;

        if(n > 1 && !selected.isEmpty()){
            Card::Suit suit = selected.first()->getSuit();
            return card->getSuit() == suit;
        }

        switch(Sanguosha->currentRoomState()->getCurrentCardUseReason()){
        case CardUseStruct::CARD_USE_REASON_PLAY:{
                if(Self->isWounded() && card->getSuit() == Card::Heart)
                    return true;
                else if(Slash::IsAvailable(Self) && card->getSuit() == Card::Diamond) {
                    if (Self->getWeapon() && card->getEffectiveId() == Self->getWeapon()->getId()
                        && card->objectName() == "crossbow")
                        return Self->canSlashWithoutCrossbow();
                    else
                        return true;
                } else
                    return false;
            }

        case CardUseStruct::CARD_USE_REASON_RESPONSE: 
            {
                QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
                if(pattern == "jink")
                    return card->getSuit() == Card::Club;
                else if(pattern == "nullification")
                    return card->getSuit() == Card::Spade;
                else if(pattern == "peach" || pattern == "peach+analeptic")
                    return card->getSuit() == Card::Heart;
                else if(pattern == "slash")
                    return card->getSuit() == Card::Diamond;
            }

        default:
            break;
        }

        return false;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        int n = qMax(1, Self->getHp());

        if(cards.length() != n)
            return NULL;

        const Card *card = cards.first();
        Card *new_card = NULL;

        switch (card->getSuit()) {
        case Card::Spade: {
                new_card = new Nullification(Card::SuitToBeDecided, 0);
                break;
            }
        case Card::Heart: {
                new_card = new Peach(Card::SuitToBeDecided, 0);
                break;
            }
        case Card::Club: {
                new_card = new Jink(Card::SuitToBeDecided, 0);
                break;
            }
        case Card::Diamond: {
                new_card = new FireSlash(Card::SuitToBeDecided, 0);
                break;
        }
        default:
            break;
        }

        if(new_card){
            new_card->setSkillName(objectName());
            new_card->addSubcards(cards);
        }

        return new_card;
    }

    virtual int getEffectIndex(const ServerPlayer *player, const Card *card) const{
        return static_cast<int>(player->getRoom()->getCard(card->getSubcards().first())->getSuit()) + 1;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        int n = qMax(1, player->getHp());
        int count = 0;
        foreach(const Card *card, player->getHandcards() + player->getEquips()){
            if(card->objectName() == "nullification")
                return true;

            if(card->getSuit() == Card::Spade)
                count ++;
        }

        return count >= n;
    }
};


GodPackage::GodPackage()
    :Package("god")
{
    General *shenguanyu = new General(this, "shenguanyu", "god", 5);
    shenguanyu->addSkill(new Wushen);
    shenguanyu->addSkill(new WushenTargetMod);
    shenguanyu->addSkill(new Wuhun);
    shenguanyu->addSkill(new WuhunRevenge);
    related_skills.insertMulti("wushen", "#wushen-target");
    related_skills.insertMulti("wuhun", "#wuhun");

    General *shenlvmeng = new General(this, "shenlvmeng", "god", 3);
    shenlvmeng->addSkill(new Shelie);
    shenlvmeng->addSkill(new Gongxin);

    General *shenzhouyu = new General(this, "shenzhouyu", "god");
    shenzhouyu->addSkill(new Qinyin);
    shenzhouyu->addSkill(new MarkAssignSkill("@flame", 1));
    shenzhouyu->addSkill(new Yeyan);

    General *shenzhugeliang = new General(this, "shenzhugeliang", "god", 3);
    shenzhugeliang->addSkill(new Qixing);
    shenzhugeliang->addSkill(new QixingStart);
    shenzhugeliang->addSkill(new QixingAsk);
    shenzhugeliang->addSkill(new QixingClear);
    shenzhugeliang->addSkill(new FakeMoveSkill("qixing"));
    shenzhugeliang->addSkill(new Kuangfeng);
    shenzhugeliang->addSkill(new Dawu);
    related_skills.insertMulti("qixing", "#qixing");
    related_skills.insertMulti("qixing", "#qixing-ask");
    related_skills.insertMulti("qixing", "#qixing-clear");
    related_skills.insertMulti("qixing", "#qixing-fake-move");

    General *shencaocao = new General(this, "shencaocao", "god", 3);
    shencaocao->addSkill(new Guixin);
    shencaocao->addSkill(new Feiying);

    General *shenlvbu = new General(this, "shenlvbu", "god", 5);
    shenlvbu->addSkill(new Kuangbao);
    shenlvbu->addSkill(new MarkAssignSkill("@wrath", 2));
    shenlvbu->addSkill(new Wumou);
    shenlvbu->addSkill(new Wuqian);
    shenlvbu->addSkill(new Shenfen);

    related_skills.insertMulti("kuangbao", "#@wrath-2");

    General *shenzhaoyun = new General(this, "shenzhaoyun", "god", 2);
    shenzhaoyun->addSkill(new JuejingKeep);
    shenzhaoyun->addSkill(new Juejing);
    shenzhaoyun->addSkill(new Longhun);
    related_skills.insertMulti("juejing", "#juejing-draw");

    General *shensimayi = new General(this, "shensimayi", "god", 4);
    shensimayi->addSkill(new Renjie);
    shensimayi->addSkill(new Baiyin);
    shensimayi->addSkill(new Lianpo);
    shensimayi->addSkill(new LianpoCount);
    shensimayi->addSkill(new LianpoDo);
    shensimayi->addRelateSkill("jilve");

    related_skills.insertMulti("jilve", "#jilve-clear");
    related_skills.insertMulti("lianpo", "#lianpo-count");
    related_skills.insertMulti("lianpo", "#lianpo-do");

    addMetaObject<GongxinCard>();
    addMetaObject<YeyanCard>();
    addMetaObject<ShenfenCard>();
    addMetaObject<GreatYeyanCard>();
    addMetaObject<SmallYeyanCard>();
    addMetaObject<KuangfengCard>();
    addMetaObject<DawuCard>();
    addMetaObject<WuqianCard>();
    addMetaObject<JilveCard>();

    skills << new Jilve << new JilveClear;
}

ADD_PACKAGE(God)
