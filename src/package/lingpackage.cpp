#include "lingpackage.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "engine.h"
#include "maneuvering.h"
#include "settings.h"

LuoyiCard::LuoyiCard() {
    target_fixed = true;
}

void LuoyiCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const{
    source->setFlags("neoluoyi");
}

class NeoLuoyi: public OneCardViewAsSkill {
public:
    NeoLuoyi(): OneCardViewAsSkill("neoluoyi") {
        filter_pattern = "EquipCard!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("LuoyiCard") && player->canDiscard(player, "he");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        LuoyiCard *card = new LuoyiCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class NeoLuoyiBuff: public TriggerSkill {
public:
    NeoLuoyiBuff(): TriggerSkill("#neoluoyi") {
        events << DamageCaused;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasFlag("neoluoyi") && target->isAlive();
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *xuchu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.chain || damage.transfer || !damage.by_user) return false;
        const Card *reason = damage.card;
        if (reason && (reason->isKindOf("Slash") || reason->isKindOf("Duel"))) {
            LogMessage log;
            log.type = "#LuoyiBuff";
            log.from = xuchu;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(++damage.damage);
            room->sendLog(log);

            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

NeoFanjianCard::NeoFanjianCard() {
    mute = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

void NeoFanjianCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhouyu = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = zhouyu->getRoom();

    room->broadcastSkillInvoke("fanjian");
    const Card *card = Sanguosha->getCard(getSubcards().first());
    int card_id = card->getEffectiveId();
    Card::Suit suit = room->askForSuit(target, "neofanjian");

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = target;
    log.arg = Card::Suit2String(suit);
    room->sendLog(log);

    room->getThread()->delay();
    target->obtainCard(this);
    room->showCard(target, card_id);

    if (card->getSuit() != suit)
        room->damage(DamageStruct("neofanjian", zhouyu, target));
}

class NeoFanjian: public OneCardViewAsSkill {
public:
    NeoFanjian(): OneCardViewAsSkill("neofanjian") {
        filter_pattern = ".|.|.|hand";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && !player->hasUsed("NeoFanjianCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *card = new NeoFanjianCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class Yishi: public TriggerSkill {
public:
    Yishi(): TriggerSkill("yishi") {
        events << DamageCaused;
    }

    virtual bool canInvoke(ServerPlayer *, const Card *c) const{
        return c->getSuit() == Card::Heart;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if (damage.card && damage.card->isKindOf("Slash") && canInvoke(player, damage.card)
            && damage.by_user && !damage.chain && !damage.transfer
            && player->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke("yishi", 1);
            LogMessage log;
            log.type = "#Yishi";
            log.from = player;
            log.arg = objectName();
            log.to << damage.to;
            room->sendLog(log);
            if (!damage.to->isAllNude()) {
                int card_id = room->askForCardChosen(player, damage.to, "hej", objectName());
                if(room->getCardPlace(card_id) == Player::PlaceDelayedTrick)
                    room->broadcastSkillInvoke("yishi", 2);
                else if(room->getCardPlace(card_id) == Player::PlaceEquip)
                    room->broadcastSkillInvoke("yishi", 3);
                else
                    room->broadcastSkillInvoke("yishi", 4);
                CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, player->objectName());
                room->obtainCard(player, Sanguosha->getCard(card_id), reason, room->getCardPlace(card_id) != Player::PlaceHand);
            }
            return true;
        }
        return false;
    }
};

class Zhulou: public PhaseChangeSkill {
public:
    Zhulou(): PhaseChangeSkill("zhulou") {
    }

    virtual QString getAskForCardPattern() const{
        return ".Weapon";
    }

    virtual bool onPhaseChange(ServerPlayer *gongsun) const{
        Room *room = gongsun->getRoom();
        if (gongsun->getPhase() == Player::Finish && gongsun->askForSkillInvoke(objectName())) {
            gongsun->drawCards(2);
            room->broadcastSkillInvoke("zhulou");
            if (!room->askForCard(gongsun, getAskForCardPattern(), "@zhulou-discard"))
                room->loseHp(gongsun);
        }
        return false;
    }
};

class Tannang: public DistanceSkill {
public:
    Tannang(): DistanceSkill("tannang") {
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if (from->hasSkill(objectName()))
            return -from->getLostHp();
        else
            return 0;
    }
};

#include "wind.h"
class NeoJushou: public Jushou {
public:
    NeoJushou(): Jushou() {
        setObjectName("neojushou");
    }

    virtual int getJushouDrawNum(ServerPlayer *caoren) const{
        return 2 + caoren->getLostHp();
    }
};

class NeoGanglie: public MasochismSkill {
public:
    NeoGanglie(): MasochismSkill("neoganglie") {
    }

    virtual void onDamaged(ServerPlayer *xiahou, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = xiahou->getRoom();
        QVariant data = QVariant::fromValue(damage);

        if (room->askForSkillInvoke(xiahou, "neoganglie", data)) {
            room->broadcastSkillInvoke("ganglie");

            JudgeStruct judge;
            judge.pattern = ".|heart";
            judge.good = false;
            judge.reason = objectName();
            judge.who = xiahou;

            room->judge(judge);
            if (!from || from->isDead()) return;
            if (judge.isGood()) {
                QStringList choicelist;
                choicelist << "damage";
                if (from->getHandcardNum() > 1)
                    choicelist << "throw";
                QString choice = room->askForChoice(xiahou, objectName(), choicelist.join("+"));
                if (choice == "damage")
                    room->damage(DamageStruct(objectName(), xiahou, from));
                else
                    room->askForDiscard(from, objectName(), 2, 2);
            }
        }
    }
};

LingPackage::LingPackage()
    : Package("ling")
{
    General *neo_xiahoudun = new General(this, "neo_xiahoudun", "wei");
    neo_xiahoudun->addSkill(new NeoGanglie);

    General *neo_xuchu = new General(this, "neo_xuchu", "wei");
    neo_xuchu->addSkill(new NeoLuoyi);
    neo_xuchu->addSkill(new NeoLuoyiBuff);
    related_skills.insertMulti("neoluoyi", "#neoluoyi");

    General *neo_caoren = new General(this, "neo_caoren", "wei");
    neo_caoren->addSkill(new NeoJushou);

    General *neo_guanyu = new General(this, "neo_guanyu", "shu");
    neo_guanyu->addSkill("wusheng");
    neo_guanyu->addSkill(new Yishi);

    General *neo_zhangfei = new General(this, "neo_zhangfei", "shu");
    neo_zhangfei->addSkill("paoxiao");
    neo_zhangfei->addSkill(new Tannang);

    General *neo_zhaoyun = new General(this, "neo_zhaoyun", "shu");
    neo_zhaoyun->addSkill("longdan");
    neo_zhaoyun->addSkill("yicong");

    General *neo_zhouyu = new General(this, "neo_zhouyu", "wu", 3);
    neo_zhouyu->addSkill("yingzi");
    neo_zhouyu->addSkill(new NeoFanjian);

    General *neo_gongsunzan = new General(this, "neo_gongsunzan", "qun");
    neo_gongsunzan->addSkill(new Zhulou);
    neo_gongsunzan->addSkill("yicong");

    addMetaObject<LuoyiCard>();
    addMetaObject<NeoFanjianCard>();
}

ADD_PACKAGE(Ling)


Neo2013XinzhanCard::Neo2013XinzhanCard(): XinzhanCard(){
    setObjectName("Neo2013XinzhanCard");
}


class Neo2013Xinzhan: public ZeroCardViewAsSkill{
public:
    Neo2013Xinzhan(): ZeroCardViewAsSkill("neo2013xinzhan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return (!player->hasUsed("Neo2013XinzhanCard")) && (player->getHandcardNum() > player->getLostHp());
    }

    virtual const Card *viewAs() const{
        return new Neo2013XinzhanCard;
    }

};


class Neo2013Huilei: public TriggerSkill {
public:
    Neo2013Huilei():TriggerSkill("neo2013huilei") {
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if (death.who != player)
            return false;
        ServerPlayer *killer = death.damage ? death.damage->from : NULL;
        if (killer) {
            LogMessage log;
            log.type = "#HuileiThrow";
            log.from = player;
            log.to << killer;
            log.arg = objectName();
            room->sendLog(log);
            room->notifySkillInvoked(player, objectName());

            QString killer_name = killer->getGeneralName();
            if (killer_name.contains("zhugeliang") || killer_name == "wolong")
                room->broadcastSkillInvoke(objectName(), 1);
            else
                room->broadcastSkillInvoke(objectName(), 2);

            killer->throwAllHandCardsAndEquips();
            room->setPlayerMark(killer, "@HuileiDecrease", 1);
            room->attachSkillToPlayer(killer, "#neo2013huilei-maxcards");
        }

        return false;
    }
};

class Neo2013HuileiDecrease: public MaxCardsSkill{
public:
    Neo2013HuileiDecrease(): MaxCardsSkill("#neo2013huilei-maxcards"){

    }

    virtual int getExtra(const Player *target) const{
        if (target->getMark("@HuileiDecrease") > 0)
            return -1;
        return 0;
    }
};

class Neo2013Yishi: public Yishi{
public:
    Neo2013Yishi(): Yishi(){
        setObjectName("neo2013yishi");
    }

    virtual bool canInvoke(ServerPlayer *player, const Card *) const{
        return !player->isAllNude();
    }
};

class Neo2013HaoyinVS: public ZeroCardViewAsSkill{
public:
    Neo2013HaoyinVS(): ZeroCardViewAsSkill("neo2013haoyin"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Analeptic::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern.contains("analeptic");
    }

    virtual const Card *viewAs() const{
        Analeptic *a = new Analeptic(Card::NoSuit, 0);
        a->setSkillName(objectName());
        return a;
    }

};

class Neo2013Haoyin: public TriggerSkill{
public:
    Neo2013Haoyin(): TriggerSkill("neo2013haoyin"){
        view_as_skill = new Neo2013HaoyinVS;
        events << PreCardUsed;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Analeptic") && use.from == player && use.card->getSkillName() == objectName())
            room->loseHp(player);

        return false;
    }

};

class Neo2013Zhulou: public Zhulou{
public:
    Neo2013Zhulou(): Zhulou(){
        setObjectName("neo2013zhulou");
    }

    virtual QString getAskForCardPattern() const{
        return "^BasicCard";
    }
};

Neo2013FanjianCard::Neo2013FanjianCard(): SkillCard(){
    will_throw = false;
    handling_method = Card::MethodNone;
}

void Neo2013FanjianCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *source = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = target->getRoom();

    const Card *card = Sanguosha->getCard(getSubcards().first());
    int card_id = card->getEffectiveId();
    Card::Suit suit = room->askForSuit(target, "neo2013fanjian");

    LogMessage l;
    l.type = "#ChooseSuit";
    l.from = target;
    l.arg = Card::Suit2String(suit);
    room->sendLog(l);

    room->getThread()->delay();
    room->showCard(source, card_id);

    if (card->getSuit() != suit)
        room->loseHp(target);

    if (target->isDead())
        return;

    QVariant data;
    data.setValue(source);

    QString choice = "getIt";
    if (target->canDiscard(source, "h"))
        choice = room->askForChoice(target, "neo2013fanjian", "getIt+discardOne", data);

    if (choice == "getIt")
        target->obtainCard(this);
    else 
        room->throwCard(room->askForCardChosen(target, source, "h", "neo2013fanjian", false, Card::MethodDiscard), source, target);

}
class Neo2013Fanjian: public OneCardViewAsSkill{
public:
    Neo2013Fanjian(): OneCardViewAsSkill("neo2013fanjian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return (!player->isKongcheng() && !player->hasUsed("Neo2013FanjianCard"));
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Neo2013FanjianCard *c = new Neo2013FanjianCard;
        c->addSubcard(originalCard);
        return c;
    }
};

class Neo2013Fankui: public MasochismSkill{
public:
    Neo2013Fankui(): MasochismSkill("neo2013fankui"){

    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{
        Room *room = target->getRoom();
        for (int i = 0; i < damage.damage; i++){
            QList<ServerPlayer *> players;
            foreach (ServerPlayer *p, room->getOtherPlayers(target)){
                if (!p->isNude())
                    players << p;
            }

            if (players.isEmpty())
                return;

            ServerPlayer *victim;
            if ((victim = room->askForPlayerChosen(target, players, objectName(), "@neo2013fankui", true, true)) != NULL){
                int card_id = room->askForCardChosen(target, victim, "he", objectName());
                room->obtainCard(target, Sanguosha->getCard(card_id), room->getCardPlace(card_id) != Player::PlaceHand);

                room->broadcastSkillInvoke(objectName());
            }
        }
    }
};
/*

class Neo2013Xiezun: public MaxCardsSkill{
public:
    Neo2013Xiezun(): MaxCardsSkill("neo2013xiezun"){

    }

    virtual int getFixed(const Player *target) const{

    }
};
*/ //temporily lay it aside
//todo：在player::getMaxCards里加个参数，令其计算手牌上限时不考虑某技能

class Neo2013RenWang: public TriggerSkill{
public:
    Neo2013RenWang():TriggerSkill("neo2013renwang"){
        events << EventPhaseStart << TargetConfirming << SlashEffected << CardEffected << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *selfplayer = room->findPlayerBySkillName(objectName());
        if (selfplayer == NULL)
            return false;

        switch (triggerEvent){
            case (EventPhaseStart):{
                if (player->getPhase() == Player::RoundStart){
                    QStringList emptylist;
                    selfplayer->tag["neorenwang"] = emptylist;
                }
            }
            case (TargetConfirming):{
                if (!TriggerSkill::triggerable(player))
                    return false;
                CardUseStruct use = data.value<CardUseStruct>();
                ServerPlayer *source = use.from;
                const Card *card = use.card;

                if (card != NULL && (card->isKindOf("Slash") || card->isNDTrick())){
                    if (!source->hasFlag("YiRenwangFirst"))
                        room->setPlayerFlag(source, "YiRenwangFirst");
                    else
                        if (room->askForSkillInvoke(player, objectName(), data)){
                            room->broadcastSkillInvoke(objectName());
                            if (!room->askForDiscard(source, objectName(), 1, 1, true, true, "@neo2013renwang-discard")){
                                QStringList neorenwanginvalid = player->tag["neorenwang"].toStringList();
                                neorenwanginvalid << card->toString();
                                player->tag["neorenwang"] = neorenwanginvalid;
                            }
                        }
                }
                break;
            }
            case (CardEffected):{
                CardEffectStruct effect = data.value<CardEffectStruct>();
                if (effect.card->isNDTrick() && effect.to == selfplayer){
                    QStringList neorenwanginvalid = selfplayer->tag["neorenwang"].toStringList();
                    if (neorenwanginvalid.contains(effect.card->toString())){
                        neorenwanginvalid.removeOne(effect.card->toString());
                        selfplayer->tag["neorenwang"] = neorenwanginvalid;
                        return true;
                    }
                }
                break;
            }
            case (SlashEffected):{
                SlashEffectStruct effect = data.value<SlashEffectStruct>();
                if (effect.to != selfplayer)
                    return false;

                QStringList neorenwanginvalid = selfplayer->tag["neorenwang"].toStringList();
                if (neorenwanginvalid.contains(effect.slash->toString())){
                    neorenwanginvalid.removeOne(effect.slash->toString());
                    selfplayer->tag["neorenwang"] = neorenwanginvalid;
                    return true;
                }
                break;
            }
            case (CardFinished):{
                const Card *c = data.value<CardUseStruct>().card;
                QStringList neorenwanginvalid = selfplayer->tag["neorenwang"].toStringList();
                if (neorenwanginvalid.contains(c->toString())){
                    neorenwanginvalid.removeOne(c->toString());
                    selfplayer->tag["neorenwang"] = neorenwanginvalid;
                }
            }
            default:
                Q_ASSERT(false);
        }
        return false;
    }
};

Neo2013YongyiCard::Neo2013YongyiCard(): SkillCard(){
    mute = true;
}

bool Neo2013YongyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    QList<int> arrows = Self->getPile("neoarrow");
    QList<const Card *> arrowcards;

    foreach(int id, arrows)
        arrowcards << Sanguosha->getCard(id);

    QList<const Player *> players = Self->getAliveSiblings();

    QList<const Player *> tars;

    foreach(const Card *c, arrowcards){
        Slash *slash = new Slash(c->getSuit(), c->getNumber());
        slash->addSubcard(c);
        slash->setSkillName("neo2013yongyi");
        QList<const Player *> oldplayers = players;
        foreach(const Player *p, oldplayers)
            if (slash->targetFilter(targets, p, Self)){
                players.removeOne(p);
                tars << p;
            }
        delete slash;
        slash = NULL;
        if (players.isEmpty())
            break;
    }

    return tars.contains(to_select);
}

const Card *Neo2013YongyiCard::validate(CardUseStruct &cardUse) const{
    QList<int> arrows = cardUse.from->getPile("neoarrow");
    QList<int> arrowsdisabled;
    QList<const Card *> arrowcards;

    foreach(int id, arrows)
        arrowcards << Sanguosha->getCard(id);

    foreach(const Card *c, arrowcards){
        Slash *slash = new Slash(c->getSuit(), c->getNumber());
        slash->addSubcard(c);
        slash->setSkillName("neo2013yongyi");
        foreach(ServerPlayer *to, cardUse.to)
            if (cardUse.from->isProhibited(to, slash)){
                arrows.removeOne(slash->getSubcards()[0]);
                arrowsdisabled << slash->getSubcards()[0];
                break;
            }
        delete slash;
        slash = NULL;
    }

    if (arrows.isEmpty())
        return NULL;

    Room *room = cardUse.from->getRoom();

    int or_aidelay = Config.AIDelay;
    Config.AIDelay = 0;

    room->fillAG(arrows + arrowsdisabled, cardUse.from, arrowsdisabled);

    int slashcard = room->askForAG(cardUse.from, arrows, false, "neo2013yongyi");

    room->clearAG(cardUse.from);
    Config.AIDelay = or_aidelay;

    const Card *c = Sanguosha->getCard(slashcard);
    Slash *realslash = new Slash(c->getSuit(), c->getNumber());
    realslash->addSubcard(c);
    realslash->setSkillName("neo2013yongyi");
    return realslash;
}

class Neo2013YongyiVS: public ZeroCardViewAsSkill{
public:
    Neo2013YongyiVS(): ZeroCardViewAsSkill("neo2013yongyi"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "slash" && Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE;
    }

    virtual const Card *viewAs() const{
        return new Neo2013YongyiCard;
    }
};

class Neo2013Yongyi: public TriggerSkill{
public:
    Neo2013Yongyi(): TriggerSkill("neo2013yongyi"){
        events << TargetConfirmed << Damage;
        view_as_skill = new Neo2013YongyiVS;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            if ((use.card->isKindOf("Slash") || use.card->isNDTrick()) && use.to.contains(player)){
                const Card *c = room->askForExchange(player, objectName(), 1, false, "@neo2013yongyiput", true);
                if (c != NULL)
                    player->addToPile("neoarrow", c, false);
            }
        }
        else {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card != NULL && damage.card->isKindOf("Slash") && damage.card->getSkillName() == objectName()
                    && !damage.chain && !damage.transfer && damage.by_user)
                if (damage.card->isRed()){
                    if (player->askForSkillInvoke(objectName(), data)){
                        room->broadcastSkillInvoke(objectName(), 3);
                        player->drawCards(1);
                    }
                }
                else if (damage.card->isBlack()){
                    if (!damage.to->isNude() && player->askForSkillInvoke(objectName(), data)){
                        int card_id = room->askForCardChosen(player, damage.to, "he", objectName(), false, Card::MethodDiscard);
                        room->throwCard(card_id, damage.to, player);
                        room->broadcastSkillInvoke(objectName(), 4);
                    }
                }
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *player, const Card *card) const{
        if (card->isKindOf("Slash"))
            return qrand() % 2 + 1;
        return -1;
    }
};


class Neo2013Duoyi: public TriggerSkill{
public:
    Neo2013Duoyi(): TriggerSkill("neo2013duoyi"){
        events << EventPhaseStart << CardUsed << CardResponded << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive() && !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *selfplayer = room->findPlayerBySkillName(objectName());
        if (selfplayer == NULL || selfplayer->isKongcheng())
            return false;
        static QStringList types;
        if (types.isEmpty())
            types << "BasicCard" << "EquipCard" << "TrickCard";

        if (triggerEvent == EventPhaseStart){
            if (player->getPhase() != Player::RoundStart)
                return false;

            if (room->askForDiscard(selfplayer, objectName(), 1, 1, true, true, "@neo2013duoyi")){
                QString choice = room->askForChoice(selfplayer, objectName(), types.join("+"), data);
                room->setPlayerMark(player, "YiDuoyiType", types.indexOf(choice) + 1);
            }
        }
        else if (triggerEvent == CardUsed || triggerEvent == CardResponded){
            if (player->getMark("YiDuoyiType") == 0)
                return false;

            std::string t = types[player->getMark("YiDuoyiType") - 1].toStdString();   //QString 2 char * is TOO complicated!

            const Card *c = NULL;

            if (triggerEvent == CardUsed)
                c = data.value<CardUseStruct>().card;
            else {
                CardResponseStruct resp = data.value<CardResponseStruct>();
                if (resp.m_isUse)
                    c = resp.m_card;
            }
            if (c == NULL)
                return false;

            if (c->isKindOf(t.c_str()))
                selfplayer->drawCards(1);
        }
        else {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive)
                room->setPlayerMark(player, "YiDuoyiType", 0);
        }
        return false;
    }
};

Neo2013PujiCard::Neo2013PujiCard(): SkillCard(){
}

bool Neo2013PujiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() == 0 && !to_select->isNude() && to_select != Self;
}

void Neo2013PujiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets[0];
    const Card *card = Sanguosha->getCard(room->askForCardChosen(source, target, "he", objectName(), false, Card::MethodDiscard));

    QList<ServerPlayer *> beneficiary;
    if (Sanguosha->getCard(getSubcards()[0])->isBlack())
        beneficiary << source;

    if (card->isBlack())
        beneficiary << target;

    room->throwCard(card, target, source);

    if (beneficiary.length() != 0)
        foreach(ServerPlayer *p, beneficiary){
            QStringList choicelist;
            choicelist << "draw";
            if (p->isWounded())
                choicelist << "recover";

            QString choice = room->askForChoice(p, objectName(), choicelist.join("+"));

            if (choice == "draw")
                p->drawCards(1);
            else {
                RecoverStruct r;
                r.who = p;
                room->recover(p, r);
            }
        }
}

class Neo2013Puji: public OneCardViewAsSkill{
public:
    Neo2013Puji(): OneCardViewAsSkill("neo2013puji"){

    }

    virtual bool viewFilter(const Card *to_select) const{
        return !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Neo2013PujiCard *c = new Neo2013PujiCard;
        c->addSubcard(originalCard);
        return c;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && !player->hasUsed("Neo2013PujiCard");
    }
};

class Neo2013Suishi: public TriggerSkill{
public:
    Neo2013Suishi(): TriggerSkill("neo2013suishi"){
        events << Damage << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *selfplayer = room->findPlayerBySkillName(objectName());
        if (selfplayer == NULL || selfplayer->isDead())
            return false;
        if (triggerEvent == Damage){
            if (selfplayer->hasFlag("YiSuishiUsed"))
                return false;
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.from->hasSkill(objectName()))
                if (room->askForSkillInvoke(player, objectName(), data)){
                    room->broadcastSkillInvoke(objectName(), 1);
                    room->notifySkillInvoked(selfplayer, objectName());
                    LogMessage l;
                    l.type = "#InvokeOthersSkill";
                    l.from = player;
                    l.to << selfplayer;
                    l.arg = objectName();
                    room->sendLog(l);
                    selfplayer->drawCards(1);
                    room->setPlayerFlag(selfplayer, "YiSuishiUsed");
                }
        }
        else if (TriggerSkill::triggerable(player)){
            if (player->isNude())
                return false;
            ServerPlayer *target = NULL;
            DeathStruct death = data.value<DeathStruct>();
            if (death.who->hasSkill(objectName()))
                return false;
            if (death.damage && death.damage->from)
                target = death.damage->from;

            if (target != NULL && room->askForSkillInvoke(target, objectName(), data)){
                room->broadcastSkillInvoke(objectName(), 2);
                if (target != player){
                    LogMessage l;
                    l.type = "#InvokeOthersSkill";
                    l.from = target;
                    l.to << player;
                    l.arg = objectName();
                    room->sendLog(l);
                }
                room->askForDiscard(player, objectName(), 1, 1, false, true, "@neo2013suishi");
            }
        }
        return false;
    }
};

class Neo2013Shushen: public TriggerSkill{
public:
    Neo2013Shushen(): TriggerSkill("neo2013shushen"){
        events << HpRecover;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        RecoverStruct recover_struct = data.value<RecoverStruct>();
        int recover = recover_struct.recover;
        for (int i = 1; i <= recover; i++){
            ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName(), "@neo2013shushen", true, true);
            if (target != NULL){
                room->broadcastSkillInvoke(objectName());
                room->drawCards(target, 1);
            }
            else
                break;
        }
        return false;
    }
};

class Neo2013Shenzhi: public PhaseChangeSkill{
public:
    Neo2013Shenzhi(): PhaseChangeSkill("neo2013shenzhi"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if (player->getPhase() != Player::Start || player->isKongcheng())
            return false;

        foreach (const Card *card, player->getHandcards()){
            if (player->isJilei(card))
                return false;
        }

        Room *room = player->getRoom();
        if (room->askForSkillInvoke(player, objectName())){
            room->broadcastSkillInvoke(objectName());
            player->throwAllHandCards();
            RecoverStruct recover;
            recover.who = player;
            recover.recover = 1;
            room->recover(player, recover);
        }
        return false;
    }
};

class Neo2013Longyin: public TriggerSkill{
public:
    Neo2013Longyin(): TriggerSkill("neo2013longyin"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->getPhase() == Player::Play;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash")){
            ServerPlayer *selfplayer = room->findPlayerBySkillName(objectName());
            if (selfplayer != NULL && selfplayer->canDiscard(selfplayer, "he")){
                const Card *c = NULL;
                if (use.card->isRed())
                    c = room->askForCard(selfplayer, ".|red", "@neo2013longyin", data, objectName());
                else if (use.card->isBlack())
                    c = room->askForCard(selfplayer, ".|black", "@neo2013longyin", data, objectName());
                if (c != NULL){
                    if (use.m_addHistory)
                        room->addPlayerHistory(player, use.card->getClassName(), -1);
                    selfplayer->drawCards(1);
                }
            }
        }
        return false;
    }

};

Neo2013FengyinCard::Neo2013FengyinCard(): SkillCard(){
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool Neo2013FengyinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() == 0 && to_select->hasFlag("Neo2013FengyinTarget");
}

void Neo2013FengyinCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets[0];
    target->obtainCard(this, true);
    target->skip(Player::Play);
    target->skip(Player::Discard);
    source->drawCards(1);
}

class Neo2013FengyinVS: public OneCardViewAsSkill{
public:
    Neo2013FengyinVS(): OneCardViewAsSkill("neo2013fengyin"){
        response_pattern = "@@neo2013fengyin";
        filter_pattern = "Slash,EquipCard";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Neo2013FengyinCard *card = new Neo2013FengyinCard;
        card->addSubcard(originalCard);
        return card;
    }
};


class Neo2013Fengyin: public TriggerSkill{
public:
    Neo2013Fengyin(): TriggerSkill("neo2013fengyin"){
        events << EventPhaseChanging;
        view_as_skill = new Neo2013FengyinVS;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *splayer = room->findPlayerBySkillName(objectName());
        if (splayer == NULL || splayer == player)
            return false;

        if (player->getHp() >= splayer->getHp())
            room->askForUseCard(splayer, "@@neo2013fengyin", "@neo2013fengyin", -1, Card::MethodNone);

        return false;
    }
};

class Neo2013Cangni: public ProhibitSkill{
public:
    Neo2013Cangni(): ProhibitSkill("neo2013cangni"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others) const{
        return from != to && card->isKindOf("TrickCard") && to->hasSkill(objectName()) && from->inMyAttackRange(to);
    }
};

class Neo2013Duoshi: public OneCardViewAsSkill{
public:
    Neo2013Duoshi(): OneCardViewAsSkill("neo2013duoshi"){
        filter_pattern = ".|red|.|hand!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->usedTimes("NeoDuoshiAE") <= 4;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        AwaitExhausted *ae = new AwaitExhausted(originalCard->getSuit(), originalCard->getNumber());
        ae->addSubcard(originalCard);
        ae->setSkillName(objectName());
        return ae;
    }
};

class Neo2013Danji: public PhaseChangeSkill{
public:
    Neo2013Danji(): PhaseChangeSkill("neo2013danji"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
            && target->getPhase() == Player::RoundStart
            && target->getMark(objectName()) == 0
            && target->getHandcardNum() > target->getHp();
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        room->notifySkillInvoked(player, objectName());
        LogMessage l;
        l.type = "#NeoDanjiWake";
        l.from = player;
        l.arg = QString::number(player->getHandcardNum());
        l.arg2 = QString::number(player->getHp());
        room->sendLog(l);
        room->broadcastSkillInvoke(objectName());
        room->doLightbox("$DanjiAnimate", 5000);
        if (room->changeMaxHpForAwakenSkill(player)){
            room->setPlayerProperty(player, "kingdom", "shu");
            room->setPlayerMark(player, objectName(), 1);
            room->acquireSkill(player, "mashu");
            room->acquireSkill(player, "zhongyi");
            room->acquireSkill(player, "neo2013huwei");
        }

        return false;
    }
};

class Neo2013Huwei: public PhaseChangeSkill{
public:
    Neo2013Huwei(): PhaseChangeSkill("neo2013huwei"){
        frequency = Limited;
        limit_mark = "@yihuwei";
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if (player->getPhase() != Player::RoundStart)
            return false;
        if (player->getMark("@yihuwei") == 0)
            return false;

        NeoDrowning *dr = new NeoDrowning(Card::NoSuit, 0);
        dr->setSkillName(objectName());

        if (!dr->isAvailable(player)){
            delete dr;
            return false;
        }
        if (player->askForSkillInvoke(objectName())){
            room->doLightbox("$HuweiAnimate", 4000);
            room->setPlayerMark(player, "@yihuwei", 0);
            room->useCard(CardUseStruct(dr, player, room->getOtherPlayers(player)), false);
        }
        return false;

    }
};


class Neo2013Huoshui: public TriggerSkill{
public:
    Neo2013Huoshui(): TriggerSkill("neo2013huoshui") {
        events << EventPhaseStart << Death
            << EventLoseSkill << EventAcquireSkill
            << CardsMoveOneTime;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority() const{
        return 5;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseStart) {
            if (!TriggerSkill::triggerable(player) 
                || (player->getPhase() != Player::RoundStart || player->getPhase() != Player::NotActive)) return false;
        } else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player || !player->hasSkill(objectName())) return false;
        } else if (triggerEvent == EventLoseSkill) {
            if (data.toString() != objectName() || player->getPhase() == Player::NotActive) return false;
        } else if (triggerEvent == EventAcquireSkill) {
            if (data.toString() != objectName() || !player->hasSkill(objectName()) || player->getPhase() == Player::NotActive)
                return false;
        } else if (triggerEvent == CardsMoveOneTime) {  //this can fix filter skill?
            if (!room->getCurrent() || !room->getCurrent()->hasSkill(objectName())) return false;
        }

        if (player->getPhase() == Player::RoundStart || triggerEvent == EventAcquireSkill)
            room->broadcastSkillInvoke(objectName(), 1);
        else if (player->getPhase() == Player::NotActive || triggerEvent == EventLoseSkill)
            room->broadcastSkillInvoke(objectName(), 2);

        foreach (ServerPlayer *p, room->getAllPlayers())
            room->filterCards(p, p->getCards("he"), true);
        Json::Value args;
        args[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);

        return false;
    }
};

class Neo2013Qingcheng: public TriggerSkill{
public:
    Neo2013Qingcheng(): TriggerSkill("neo2013qingcheng"){
        events << EventPhaseStart << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual int getPriority() const{
        return 6;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseStart && player->getPhase() == Player::RoundStart){
            ServerPlayer *zou = room->findPlayerBySkillName(objectName());
            if (zou == NULL || zou->isNude())
                return false;

            if (room->askForDiscard(zou, "neo2013qingcheng", 1, 1, true, true, "@neo2013qingcheng-discard")){
                QStringList skill_list;
                foreach (const Skill *skill, player->getVisibleSkillList()){
                    if (!skill_list.contains(skill->objectName()) && !skill->inherits("SPConvertSkill") && !skill->isAttachedLordSkill()) {
                        skill_list << skill->objectName();
                    }
                }
                QString skill_qc;
                if (!skill_list.isEmpty()) {
                    QVariant data_for_ai = QVariant::fromValue(player);
                    skill_qc = room->askForChoice(zou, "neo2013qingcheng", skill_list.join("+"), data_for_ai);
                }

                if (!skill_qc.isEmpty()) {
                    LogMessage log;
                    log.type = "$QingchengNullify";
                    log.from = zou;
                    log.to << player;
                    log.arg = skill_qc;
                    room->sendLog(log);

                    QStringList Qingchenglist = player->tag["neo2013qingcheng"].toStringList();
                    Qingchenglist << skill_qc;
                    player->tag["neo2013qingcheng"] = QVariant::fromValue(Qingchenglist);
                    room->addPlayerMark(player, "Qingcheng" + skill_qc);
                    room->broadcastSkillInvoke(objectName(), 1);
                }
            }
        }
        else if (triggerEvent == EventPhaseChanging && data.value<PhaseChangeStruct>().to == Player::NotActive){
            QStringList Qingchenglist = player->tag["neo2013qingcheng"].toStringList();
            if (Qingchenglist.isEmpty()) return false;
            foreach (QString skill_name, Qingchenglist) {
                room->setPlayerMark(player, "Qingcheng" + skill_name, 0);
                if (player->hasSkill(skill_name)) {
                    LogMessage log;
                    log.type = "$QingchengReset";
                    log.from = player;
                    log.arg = skill_name;
                    room->sendLog(log);
                }
            }
            player->tag.remove("neo2013qingcheng");
            room->broadcastSkillInvoke(objectName(), 2);
        }
        else
            return false;

        foreach (ServerPlayer *p, room->getAllPlayers())
            room->filterCards(p, p->getCards("he"), true);

        Json::Value args;
        args[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);

        return false;
    }
};


Ling2013Package::Ling2013Package(): Package("Ling2013"){
    General *neo2013_masu = new General(this, "neo2013_masu", "shu", 3);
    neo2013_masu->addSkill(new Neo2013Xinzhan);
    neo2013_masu->addSkill(new Neo2013Huilei);

    General *neo2013_guanyu = new General(this, "neo2013_guanyu", "shu");
    neo2013_guanyu->addSkill(new Neo2013Yishi);
    neo2013_guanyu->addSkill("wusheng");

    General *neo2013_zhangfei = new General(this, "neo2013_zhangfei", "shu");
    neo2013_zhangfei->addSkill(new Neo2013Haoyin);
    neo2013_zhangfei->addSkill("paoxiao");
    neo2013_zhangfei->addSkill("tannang");

    General *neo2013_gongsun = new General(this, "neo2013_gongsunzan", "qun");
    neo2013_gongsun->addSkill(new Neo2013Zhulou);
    neo2013_gongsun->addSkill("yicong");

    General *neo2013_zhouyu = new General(this, "neo2013_zhouyu", "wu", 3);
    neo2013_zhouyu->addSkill(new Neo2013Fanjian);
    neo2013_zhouyu->addSkill("yingzi");

    General *neo2013_sima = new General(this, "neo2013_simayi", "wei", 3);
    neo2013_sima->addSkill(new Neo2013Fankui);
    neo2013_sima->addSkill("guicai");

    General *neo2013_liubei = new General(this, "neo2013_liubei$", "shu");
    neo2013_liubei->addSkill(new Neo2013RenWang);
    neo2013_liubei->addSkill("jijiang");
    neo2013_liubei->addSkill("rende");

    General *neo2013_huangzhong = new General(this, "neo2013_huangzhong", "shu", 4);
    neo2013_huangzhong->addSkill(new Neo2013Yongyi);
    neo2013_huangzhong->addSkill(new SlashNoDistanceLimitSkill("neo2013yongyi"));
    neo2013_huangzhong->addSkill("liegong");
    related_skills.insertMulti("neo2013yongyi", "#neo2013yongyi-slash-ndl");

    General *neo2013_yangxiu = new General(this, "neo2013_yangxiu", "wei", 3);
    neo2013_yangxiu->addSkill(new Neo2013Duoyi);
    neo2013_yangxiu->addSkill("jilei");
    neo2013_yangxiu->addSkill("danlao");

    General *neo2013_huatuo = new General(this, "neo2013_huatuo", "qun", 3);
    neo2013_huatuo->addSkill(new Neo2013Puji);
    neo2013_huatuo->addSkill("jijiu");

    General *neo2013_xuchu = new General(this, "neo2013_xuchu", "wei", 4);
    neo2013_xuchu->addSkill("neoluoyi");
    neo2013_xuchu->addSkill("xiechan");

    General *neo2013_zhaoyun = new General(this, "neo2013_zhaoyun", "shu", 4);
    neo2013_zhaoyun->addSkill("longdan");
    neo2013_zhaoyun->addSkill("yicong");
    neo2013_zhaoyun->addSkill("jiuzhu");

    General *neo2013_tianfeng = new General(this, "neo2013_tianfeng", "qun", 3);
    neo2013_tianfeng->addSkill(new Neo2013Suishi);
    neo2013_tianfeng->addSkill("sijian");

    General *neo2013_gan = new General(this, "neo2013_ganfuren", "shu", 3, false);
    neo2013_gan->addSkill(new Neo2013Shushen);
    neo2013_gan->addSkill(new Neo2013Shenzhi);

    General *neo2013_guanping = new General(this, "neo2013_guanping", "shu", 4);
    neo2013_guanping->addSkill(new Neo2013Longyin);

    General *neo2013_fuwan = new General(this, "neo2013_fuwan", "qun", 3);
    neo2013_fuwan->addSkill(new Neo2013Fengyin);
    neo2013_fuwan->addSkill("chizhong");

    General *neo2013_fuhh = new General(this, "neo2013_fuhuanghou", "qun", 3);
    neo2013_fuhh->addSkill(new Neo2013Cangni);
    neo2013_fuhh->addSkill("mixin");

    General *neo2013_luxun = new General(this, "neo2013_luxun", "wu", 3);
    neo2013_luxun->addSkill(new Neo2013Duoshi);
    neo2013_luxun->addSkill("qianxun");

    General *neo2013_spguanyu = new General(this, "neo2013_sp_guanyu", "wei", 4);
    neo2013_spguanyu->addSkill(new Neo2013Danji);
    neo2013_spguanyu->addSkill("wusheng");
    neo2013_spguanyu->addRelateSkill("neo2013huwei");

    General *neo2013_zoushi = new General(this, "neo2013_zoushi", "qun", 3);
    neo2013_zoushi->addSkill(new Neo2013Huoshui);
    neo2013_zoushi->addSkill(new Neo2013Qingcheng);

    addMetaObject<Neo2013XinzhanCard>();
    addMetaObject<Neo2013FanjianCard>();
    addMetaObject<Neo2013FengyinCard>();
    addMetaObject<Neo2013YongyiCard>();

    skills << new Neo2013HuileiDecrease << new Neo2013Huwei;
}

ADD_PACKAGE(Ling2013)


AwaitExhausted::AwaitExhausted(Card::Suit suit, int number): TrickCard(suit, number){
    setObjectName("await_exhausted");
}


QString AwaitExhausted::getSubtype() const{
    return "await_exhausted";
}

bool AwaitExhausted::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    return to_select != Self;
}

bool AwaitExhausted::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return true;
}

void AwaitExhausted::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct new_use = card_use;
    if (!card_use.to.contains(card_use.from))
        new_use.to << card_use.from;

    if (getSkillName() == "neo2013duoshi")
        room->addPlayerHistory(card_use.from, "NeoDuoshiAE", 1);

    TrickCard::onUse(room, new_use);
}

void AwaitExhausted::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(2);
    effect.to->getRoom()->askForDiscard(effect.to, objectName(), 2, 2, false, true);
}


BefriendAttacking::BefriendAttacking(Card::Suit suit, int number): SingleTargetTrick(suit, number){
    setObjectName("befriend_attacking");
}

bool BefriendAttacking::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num)
        return false;

    QList<const Player *> siblings = Self->getAliveSiblings();
    int distance = 0;

    foreach(const Player *p, siblings){
        int dist = Self->distanceTo(p);
        if (dist > distance)
            distance = dist;
    }

    return Self->distanceTo(to_select) == distance;
}

bool BefriendAttacking::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() > 0;
}

void BefriendAttacking::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(1);
    effect.from->drawCards(3);
}

KnownBoth::KnownBoth(Card::Suit suit, int number): SingleTargetTrick(suit, number){
    setObjectName("known_both");
    can_recast = true;
}

bool KnownBoth::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (Self->isCardLimited(this, Card::MethodUse))
        return false;

    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num || to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

bool KnownBoth::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if (Self->isCardLimited(this, Card::MethodUse))
        return targets.length() == 0;

    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (getSkillName().contains("guhuo") || getSkillName() == "qice")  // Dirty hack here!!!
        return targets.length() > 0 && targets.length() <= total_num;
    else
        return targets.length() <= total_num;
}

void KnownBoth::onUse(Room *room, const CardUseStruct &card_use) const{
    if (card_use.to.isEmpty()){
        CardMoveReason reason(CardMoveReason::S_REASON_RECAST, card_use.from->objectName());
        reason.m_skillName = this->getSkillName();
        room->moveCardTo(this, card_use.from, NULL, Player::DiscardPile, reason);
        card_use.from->broadcastSkillInvoke("@recast");

        LogMessage log;
        log.type = "#Card_Recast";
        log.from = card_use.from;
        log.card_str = card_use.card->toString();
        room->sendLog(log);

        card_use.from->drawCards(1);
    }
    else
        SingleTargetTrick::onUse(room, card_use);
}

void KnownBoth::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->showAllCards(effect.to, effect.from);
}

NeoDrowning::NeoDrowning(Card::Suit suit, int number): AOE(suit, number){
    setObjectName("neo_drowning");
}

void NeoDrowning::onEffect(const CardEffectStruct &effect) const{
    QVariant data = QVariant::fromValue(effect);
    Room *room = effect.to->getRoom();
    QString choice = "";
    if (!effect.to->getEquips().isEmpty() && (choice = room->askForChoice(effect.to, objectName(), "throw+damage", data)) == "throw")
        effect.to->throwAllEquips();
    else{
        ServerPlayer *source = NULL;
        if (effect.from->isAlive())
            source = effect.from;
        if (choice == "")
            room->getThread()->delay();
        room->damage(DamageStruct(this, source, effect.to));
    }
}

SixSwords::SixSwords(Card::Suit suit, int number): Weapon(suit, number, 2){
    setObjectName("SixSwords");
}

SixSwordsSkillCard::SixSwordsSkillCard(): SkillCard(){

}

bool SixSwordsSkillCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return to_select != Self;
}

void SixSwordsSkillCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->gainMark("@SixSwordsBuff");
}

class SixSwordsSkillVS: public ZeroCardViewAsSkill{
public:
    SixSwordsSkillVS(): ZeroCardViewAsSkill("SixSwords"){
        response_pattern = "@@SixSwords";
    }

    virtual const Card *viewAs() const{
        return new SixSwordsSkillCard;
    }
};

class SixSwordsSkill: public WeaponSkill{
public:
    SixSwordsSkill(): WeaponSkill("SixSwords"){
        events << EventPhaseStart << BeforeCardsMove;
        view_as_skill = new SixSwordsSkillVS;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        QList<ServerPlayer *> players = room->getOtherPlayers(player);
        if (triggerEvent == BeforeCardsMove){
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from != NULL && move.from == player && move.from_places.contains(Player::PlaceEquip))
                foreach(int id, move.card_ids){
                    const Card *card = Sanguosha->getCard(id);
                    if (card->getClassName() == "SixSwords"){
                        foreach(ServerPlayer *p, players)
                            if (p->getMark("@SixSwordsBuff") > 0)
                                p->loseMark("@SixSwordsBuff");
                        break;
                    }
                }
        }
        else if (player->getPhase() == Player::NotActive){
            foreach(ServerPlayer *p, players)
                if (p->getMark("@SixSwordsBuff") > 0)
                    p->loseMark("@SixSwordsBuff");
            room->askForUseCard(player, "@@SixSwords", "@six_swords");
        }
        return false;
    }
};

Triblade::Triblade(Card::Suit suit, int number): Weapon(suit, number, 3){
    setObjectName("Triblade");
}

TribladeSkillCard::TribladeSkillCard(): SkillCard(){

}

bool TribladeSkillCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() == 0 && to_select->hasFlag("TribladeFilter");
}

void TribladeSkillCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    room->damage(DamageStruct("Triblade", source, targets[0]));
}

class TribladeSkillVS: public OneCardViewAsSkill{
public:
    TribladeSkillVS(): OneCardViewAsSkill("Triblade"){
        response_pattern = "@@Triblade";
        filter_pattern = ".|.|.|hand!";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        TribladeSkillCard *c = new TribladeSkillCard;
        c->addSubcard(originalCard);
        return c;
    }
};

class TribladeSkill: public WeaponSkill{
public:
    TribladeSkill(): WeaponSkill("Triblade"){
        events << Damage;
        view_as_skill = new TribladeSkillVS;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.to && damage.to->isAlive() && damage.card && damage.card->isKindOf("Slash")
                && damage.by_user && !damage.chain && !damage.transfer){
            QList<ServerPlayer *> players;
            foreach(ServerPlayer *p, room->getOtherPlayers(damage.to))
                if (damage.to->distanceTo(p) == 1){
                    players << p;
                    room->setPlayerFlag(p, "TribladeFilter");
                }
            if (players.isEmpty())
                return false;
            room->askForUseCard(player, "@@Triblade", "@triblade");
        }

        foreach(ServerPlayer *p, room->getAllPlayers())
            if (p->hasFlag("TribladeFilter"))
                room->setPlayerFlag(p, "TribladeFilter");

        return false;
    }
};

DragonPhoenix::DragonPhoenix(Card::Suit suit, int number): Weapon(suit, number, 2){
    setObjectName("DragonPhoenix");
}

class DragonPhoenixSkill: public WeaponSkill{
public:
    DragonPhoenixSkill(): WeaponSkill("DragonPhoenix"){
        events << TargetConfirmed << Death;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.from != player)
                return false;

            if (use.card->isKindOf("Slash"))
                foreach(ServerPlayer *to, use.to){
                    if (!to->canDiscard(to, "he"))
                        return false;
                    else if (use.from->askForSkillInvoke(objectName(), data)){
                        QString prompt = "dragon-phoenix-card:" + use.from->objectName();
                        room->askForDiscard(to, objectName(), 1, 1, false, true, prompt);
                    }
                }
        }
        else {
            DeathStruct death = data.value<DeathStruct>();
            if (death.damage != NULL && death.damage->card != NULL && death.damage->card->isKindOf("Slash")
                    && death.damage->from == player && player->askForSkillInvoke(objectName(), data)){
                QString general1 = death.who->getGeneralName(), general2 = death.who->getGeneral2Name();
                QString general3 = player->getGeneralName(), general4 = player->getGeneral2Name();
                int maxhp1 = death.who->getMaxHp(), maxhp2 = player->getMaxHp();
                QList<const Skill *> skills1 = death.who->getVisibleSkillList(), skills2 = player->getVisibleSkillList();

                QStringList detachlist;
                foreach(const Skill *skill, skills1)
                    if (player->hasSkill(skill->objectName()))
                        detachlist.append(QString("-") + skill->objectName());

                if (!detachlist.isEmpty())
                    room->handleAcquireDetachSkills(player, detachlist);

                detachlist.clear();

                foreach(const Skill *skill, skills2)
                    if (death.who->hasSkill(skill->objectName()))
                        detachlist.append(QString("-") + skill->objectName());

                if (!detachlist.isEmpty())
                    room->handleAcquireDetachSkills(player, detachlist);

                room->changeHero(player, general1, false, false, false, true);
                if (general2.length() > 0)
                    room->changeHero(player, general2, false, false, true, true);

                room->changeHero(death.who, general3, false, false, false, true);
                if (general4.length() > 0)
                    room->changeHero(death.who, general4, false, false, true, true);

                if (player->getMaxHp() != maxhp1)
                    room->setPlayerProperty(player, "maxhp", (player->isLord()) ? maxhp1 + 1 : maxhp1);

                if (death.who->getMaxHp() != maxhp2)
                    room->setPlayerProperty(death.who, "maxhp", (death.who->isLord()) ? maxhp2 + 1: maxhp2);

            }
        }
        return false;
    }
};


LingCardsPackage::LingCardsPackage(): Package("LingCards", Package::CardPack){

    QList<Card *> cards;

    cards << new AwaitExhausted(Card::Diamond, 4);
    cards << new AwaitExhausted(Card::Heart, 11);
    cards << new BefriendAttacking(Card::Heart, 9);
    cards << new KnownBoth(Card::Club, 3);
    cards << new KnownBoth(Card::Club, 4);
    cards << new NeoDrowning(Card::Club, 7);
    cards << new SixSwords(Card::Diamond, 6); //攻击范围写在Player::getAttackRange(bool)中
    cards << new Triblade(Card::Diamond, 12);
    cards << new DragonPhoenix(Card::Spade, 2);

    foreach(Card *c, cards)
        c->setParent(this);


    skills << new SixSwordsSkill << new TribladeSkill << new DragonPhoenixSkill;

    addMetaObject<SixSwordsSkillCard>();
    addMetaObject<TribladeSkillCard>();
}

ADD_PACKAGE(LingCards)
