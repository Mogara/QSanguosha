#include "lingpackage.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "engine.h"
#include "maneuvering.h"

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

    QString choice = room->askForChoice(target, "neo2013fanjian", "getIt+discardOne", data);
    //ToAsk: 这里应该是不能弃牌的话就直接拿这张牌，不能先选弃牌，然后发现不能弃，放弃弃牌吧？

    if (choice == "getIt")
        target->obtainCard(this);
    else {
        if (!target->canDiscard(source, "h"))
            return ;
        room->throwCard(room->askForCardChosen(target, source, "h", "neo2013fanjian", false, Card::MethodDiscard), source, target);
    }
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
        events << TargetConfirming << SlashEffected << CardEffected << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *selfplayer = room->findPlayerBySkillName(objectName());
        if (selfplayer == NULL)
            return false;

        switch (triggerEvent){
            case (TargetConfirming):{
                if (!TriggerSkill::triggerable(player)){
                    room->setPlayerFlag(selfplayer, "-YiRenWangInvalid");
                    return false;
                }
                CardUseStruct use = data.value<CardUseStruct>();
                ServerPlayer *source = use.from;
                const Card *card = use.card;

                if (card->isKindOf("Slash") || card->isNDTrick()){
                    if (!source->hasFlag("YiRenwangFirst"))
                        room->setPlayerFlag(source, "YiRenwangFirst");
                    else
                        if (room->askForSkillInvoke(player, objectName(), data)){
                            QStringList choicelist;
                            choicelist << "invalid";
                            if (!source->isNude())
                                choicelist << "discard";
                            QString choice = room->askForChoice(source, objectName(), choicelist.join("+"), data);
                            if (choice == "discard"){
                                room->askForDiscard(source, objectName(), 1, 1, false, true);
                                room->setPlayerFlag(player, "-YiRenWangInvalid");
                            }
                            else
                                room->setPlayerFlag(player, "YiRenWangInvalid");
                        }
                }
                break;
            }
            case (CardEffected):{
                return data.value<CardEffectStruct>().card->isNDTrick() && selfplayer->hasFlag("YiRenWangInvalid");
                break;
            }
            case (SlashEffected):{
                return selfplayer->hasFlag("YiRenWangInvalid");
                break;
            }
            case (CardFinished):{
                if (selfplayer->hasFlag("YiRenWangInvalid"))
                    room->setPlayerFlag(player, "-YiRenWangInvalid");
                break;
            }
            default:
                Q_ASSERT(false);
        }
        return false;
    }
};

class Neo2013Qianhuan: public TriggerSkill{
public:
    Neo2013Qianhuan(): TriggerSkill("neo2013qianhuan"){
        events << Damaged << TargetConfirming << CardsMoveOneTime;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *selfplayer = room->findPlayerBySkillName(objectName());
        if (selfplayer == NULL)
            return false;
        switch (triggerEvent){
            case (Damaged):{
                if (room->askForSkillInvoke(player, objectName(), data)){
                    if (player != selfplayer){
                        room->notifySkillInvoked(selfplayer, objectName());
                        LogMessage l;
                        l.type = "#InvokeOthersSkill";
                        l.from = player;
                        l.to << selfplayer;
                        l.arg = objectName();
                        room->sendLog(l);
                    }
                    int id = room->drawCard();
                    QList<int> fantasy = selfplayer->getPile("fantasy");
                    selfplayer->addToPile("fantasy", id);
                    const Card *card = Sanguosha->getCard(id);
                    foreach(int id, fantasy){
                        if (card->getSuit() == Sanguosha->getCard(id)->getSuit()){
                            room->throwCard(card, NULL, player);
                            break;
                        }
                    }
                }
                break;
            }
            case (TargetConfirming):{
                CardUseStruct use = data.value<CardUseStruct>();
                if (use.to.length() == 1 && (use.card->isKindOf("BasicCard") || use.card->isKindOf("TrickCard"))
                        && !selfplayer->getPile("fantasy").isEmpty()){
                    bool invalid = false;
                    if (room->askForSkillInvoke(selfplayer, objectName(), data)){
                        if (selfplayer != player && room->askForSkillInvoke(player, objectName(), data)){
                            room->notifySkillInvoked(selfplayer, objectName());
                            LogMessage l;
                            l.type = "#InvokeOthersSkill";
                            l.from = player;
                            l.to << selfplayer;
                            l.arg = objectName();
                            room->sendLog(l);
                            invalid = true;
                        }
                        else
                            invalid = true;
                    }
                    if (invalid){
                        QList<int> fantasy = selfplayer->getPile("fantasy");
                        room->fillAG(fantasy, selfplayer);
                        int id = room->askForAG(selfplayer, fantasy, true, objectName());
                        if (id != -1){
                            room->throwCard(id, NULL, selfplayer);
                            use.to.removeOne(use.to.first());
                            data = QVariant::fromValue(use);
                            if (use.card->isKindOf("DelayedTrick"))
                                room->throwCard(use.card, NULL);
                            LogMessage l;
                            l.type = "#QiaoshuiRemove";
                            l.from = player;
                            l.to << player;
                            l.arg = use.card->objectName();
                            l.arg2 = objectName();
                            room->sendLog(l);
                        }
                        room->clearAG(selfplayer);
                    }
                }
                break;
            }
            case (CardsMoveOneTime):{
                //It seems this part of skill is not necessary anymore.
                break;
            }
            default:
                Q_ASSERT(false);
        }
        return false;
    }
};
//
//
//
//每当你成为其他角色使用【杀】或非延时类锦囊牌的唯一目标后，
//你可以将一张手牌背面朝上置于你的武将牌上，称为“箭”；
//你可以将一张“箭”当【杀】使用（无距离限制），
//当此【杀】对目标角色造成伤害后：
//锁定技，若此【杀】为红色，你可以摸一张牌；
//锁定技，若此【杀】为黑色，你可以弃置该角色的一张牌。
//
//锁定技怎么能“可以”…………这又不是袁术的视为拥有…………
//
//

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
        if (selfplayer == NULL)
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

            std::string t = types[player->getMark("YiDuoyiType") - 1].toStdString();   //QString转char *好麻烦！

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
            if (change.to == Player::PhaseNone)
                room->setPlayerMark(player, "YiDuoyiType", 0);
        }
        return false;
    }
};

Neo2013PujiCard::Neo2013PujiCard(): SkillCard(){
    //handling_method = Card::MethodDiscard;
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
    Neo2013Shushen(): TriggerSkill("neo2013_shushen"){
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

class Neo2013FengyinVS: public ViewAsSkill{ //这算是写LUA留下的后遗症么……
public:
    Neo2013FengyinVS(): ViewAsSkill("neo2013fengyin"){

    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.length() == 0 && (to_select->isKindOf("Slash") || to_select->isKindOf("EquipCard"));
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 1)
            return NULL;

        Neo2013FengyinCard *card = new Neo2013FengyinCard;
        card->addSubcard(cards[0]);
        return card;
    }
    
    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@neo2013fengyin";
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

    General *neo2013_yuji = new General(this, "neo2013_yuji", "qun", 3);
    neo2013_yuji->addSkill(new Neo2013Qianhuan);

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

    addMetaObject<Neo2013XinzhanCard>();
    addMetaObject<Neo2013FanjianCard>();
    addMetaObject<Neo2013FengyinCard>();

    skills << new Neo2013HuileiDecrease;
}

ADD_PACKAGE(Ling2013)