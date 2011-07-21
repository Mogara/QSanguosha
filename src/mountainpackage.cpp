#include "mountainpackage.h"

#include "general.h"
#include "skill.h"
#include "engine.h"
#include "standard.h"
#include "carditem.h"

QiaobianCard::QiaobianCard(){

}

bool QiaobianCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(Self->getPhase() == Player::Draw)
        return targets.length() <= 2 && !targets.isEmpty();
    else if(Self->getPhase() == Player::Play)
        return targets.length() == 1;
    else
        return targets.isEmpty();
}

bool QiaobianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(Self->getPhase() == Player::Draw)
        return targets.length() < 2 && to_select != Self && !to_select->isKongcheng();
    else if(Self->getPhase() == Player::Play){
        return targets.isEmpty() &&
                (!to_select->getJudgingArea().isEmpty() || !to_select->getEquips().isEmpty());
    }else
        return false;
}

void QiaobianCard::use(Room *room, ServerPlayer *zhanghe, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    if(zhanghe->getPhase() == Player::Draw){
        foreach(ServerPlayer *target, targets){
            int card_id = room->askForCardChosen(zhanghe, target, "h", "qiaobian");
            room->moveCardTo(Sanguosha->getCard(card_id), zhanghe, Player::Hand, false);
        }
    }else if(zhanghe->getPhase() == Player::Play){
        ServerPlayer *from = targets.first();
        int card_id = room->askForCardChosen(zhanghe, from , "ej", "qiaobian");
        const Card *card = Sanguosha->getCard(card_id);
        Player::Place place = room->getCardPlace(card_id);

        int equip_index = -1;
        const DelayedTrick *trick = NULL;
        if(place == Player::Equip){
            const EquipCard *equip = qobject_cast<const EquipCard *>(card);
            equip_index = static_cast<int>(equip->location());
        }else{
            trick = DelayedTrick::CastFrom(card);
        }

        QList<ServerPlayer *> tos;
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            if(equip_index != -1){
                if(p->getEquip(equip_index) == NULL)
                    tos << p;
            }else{
                if(!zhanghe->isProhibited(p, trick) && !p->containsTrick(trick->objectName()))
                    tos << p;
            }
        }

        if(trick && trick->isVirtualCard())
            delete trick;

        ServerPlayer *to = room->askForPlayerChosen(zhanghe, tos, "qiaobian");
        if(to)
            room->moveCardTo(card, to, place);
    }
}

class QiaobianViewAsSkill: public OneCardViewAsSkill{
public:
    QiaobianViewAsSkill():OneCardViewAsSkill(""){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QiaobianCard *card = new QiaobianCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@qiaobian";
    }
};

class Qiaobian: public PhaseChangeSkill{
public:
    Qiaobian():PhaseChangeSkill("qiaobian"){
        view_as_skill = new QiaobianViewAsSkill;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && !target->isKongcheng();
    }

    virtual bool onPhaseChange(ServerPlayer *zhanghe) const{
        Room *room = zhanghe->getRoom();

        switch(zhanghe->getPhase()){
        case Player::Start:
        case Player::Finish:
        case Player::NotActive: return false;

        case Player::Judge: return room->askForUseCard(zhanghe, "@qiaobian", "@qiaobian-judge");
        case Player::Draw: return room->askForUseCard(zhanghe, "@qiaobian", "@qiaobian-draw");
        case Player::Play: return room->askForUseCard(zhanghe, "@qiaobian", "@qiaobian-play");
        case Player::Discard: return room->askForUseCard(zhanghe, "@qiaobian", "@qiaobian-discard");
        }

        return false;
    }
};

class Beige: public TriggerSkill{
public:
    Beige():TriggerSkill("beige"){
        events << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card == NULL || !damage.card->inherits("Slash") || damage.to->isDead())
            return false;

        Room *room = player->getRoom();
        ServerPlayer *caiwenji = room->findPlayerBySkillName(objectName());
        if(caiwenji && !caiwenji->isNude() && caiwenji->askForSkillInvoke(objectName(), data)){
            room->askForDiscard(caiwenji, "beige", 1, false, true);

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(.*):(.*)");
            judge.good = true;
            judge.who = player;
            judge.reason = objectName();

            room->judge(judge);

            switch(judge.card->getSuit()){
            case Card::Heart:{
                    RecoverStruct recover;
                    recover.who = caiwenji;
                    room->recover(player, recover);

                    break;
                }

            case Card::Diamond:{
                    player->drawCards(2);

                    break;
                }

            case Card::Club:{
                    if(damage.from && damage.from->isAlive()){
                        int to_discard = qMin(2, damage.from->getCardCount(true));
                        if(to_discard != 0)
                            room->askForDiscard(damage.from, "beige", to_discard, false, true);
                    }

                    break;
                }

            case Card::Spade:{
                    if(damage.from && damage.from->isAlive())
                        damage.from->turnOver();

                    break;
                }

            default:
                break;
            }
        }

        return false;
    }
};

class Duanchang: public TriggerSkill{
public:
    Duanchang():TriggerSkill("duanchang"){
        events << Death;

        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();

        if(damage && damage->from){
            Room *room = player->getRoom();

            QList<const Skill *> skills = damage->from->getVisibleSkillList();
            foreach(const Skill *skill, skills){
                if(skill->inherits("WeaponSkill") || skill->inherits("ArmorSkill"))
                    continue;

                room->detachSkillFromPlayer(damage->from, skill->objectName());
            }

            int max_hp = damage->from->getMaxHP();
            int hp = damage->from->getHp();
            QString kingdom = damage->from->getKingdom();

            QString to_transfigure = damage->from->getGeneral()->isMale() ? "sujiang" : "sujiangf";
            room->transfigure(damage->from, to_transfigure, false, false);
            if(damage->from->getGeneral2())
                room->setPlayerProperty(damage->from, "general2", to_transfigure);

            if(max_hp != damage->from->getMaxHP())
                room->setPlayerProperty(damage->from, "maxhp", max_hp);

            if(hp != damage->from->getHp())
                room->setPlayerProperty(damage->from, "hp", hp);

            room->setPlayerProperty(damage->from, "kingdom", kingdom);
        }

        return false;
    }
};

class Tuntian: public DistanceSkill{
public:
    Tuntian():DistanceSkill("tuntian"){

    }

    virtual int getCorrect(const Player *from, const Player *) const{
        if(from->hasSkill(objectName()))
            return -from->getMark("@field");
        else
            return 0;
    }
};

class TuntianGet: public MasochismSkill{
public:
    TuntianGet():MasochismSkill("#tuntian"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *dengai, const DamageStruct &damage) const{
        if(dengai->askForSkillInvoke("tuntian")){
            dengai->gainMark("@field", damage.damage);

            foreach(int card_id, dengai->getRoom()->getNCards(damage.damage))
                dengai->addToPile("field", card_id, false);
        }
    }
};

class JixiWake: public TriggerSkill{
public:
    JixiWake():TriggerSkill("#jixi-wake"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *dengai = room->findPlayerBySkillName(objectName());
        if(dengai && dengai->getMark("jixi") == 0){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature == DamageStruct::Thunder){
                room->setPlayerMark(dengai, "jixi", 1);
                room->drawCards(dengai, 2);
                room->loseMaxHp(dengai);
            }
        }

        return false;
    }
};

class Jixi: public ZeroCardViewAsSkill{
public:
    Jixi():ZeroCardViewAsSkill("jixi"){
        snatch = new Snatch(Card::NoSuit, 0);
        snatch->setSkillName("jixi");

        frequency = Wake;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->getMark("jixi") == 0)
            return player->getPile("field").length() >= 3;
        else
            return player->getPile("field").length() > 0;
    }

    virtual const Card *viewAs() const{
        return snatch;
    }

private:
    Snatch *snatch;
};

class JixiThrow: public TriggerSkill{
public:
    JixiThrow():TriggerSkill("#jixi-throw"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->objectName() == "snatch" && use.card->getSkillName() == "jixi"){
            if(player->getMark("jixi") == 0){
                foreach(int card_id, player->getPile("field").mid(0, 3)){
                    player->getRoom()->throwCard(card_id);
                }
            }else
                player->getRoom()->throwCard(player->getPile("field").first());
        }

        return false;
    }
};

class Jiang: public TriggerSkill{
public:
    Jiang():TriggerSkill("jiang"){
        events << CardUsed << CardEffected;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *sunce, QVariant &data) const{
        const Card *card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardEffected){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            card = effect.card;
        }

        if(card == NULL)
            return false;

        if(card->inherits("Duel") || (card->inherits("Slash") && card->isRed())){
            if(sunce->askForSkillInvoke(objectName(), data))
                sunce->drawCards(1);
        }

        return false;
    }
};

class Hunzi: public PhaseChangeSkill{
public:
    Hunzi():PhaseChangeSkill("hunzi"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("hunzi") == 0
                && target->getPhase() == Player::Start
                && target->getHp() == 1;
    }

    virtual bool onPhaseChange(ServerPlayer *sunce) const{
        Room *room = sunce->getRoom();

        LogMessage log;
        log.type = "#HunziWake";
        log.from = sunce;
        room->sendLog(log);

        room->loseMaxHp(sunce);

        const Skill *yinghun_skill = Sanguosha->getSkill("yinghun");
        const PhaseChangeSkill *yinghun = qobject_cast<const PhaseChangeSkill *>(yinghun_skill);
        int refcount = room->getThread()->getRefCount(yinghun);

        room->acquireSkill(sunce, "yinghun");
        room->acquireSkill(sunce, "yingzi");

        if(refcount == 0){
            yinghun->onPhaseChange(sunce);
        }

        room->setPlayerMark(sunce, "hunzi", 1);

        return false;
    }
};

ZhibaCard::ZhibaCard(){
    will_throw = false;
}

bool ZhibaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("sunce_zhiba") && to_select != Self && !to_select->isKongcheng();
}

void ZhibaCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *sunce = targets.first();
    if(sunce->getMark("hunzi") > 0 &&
       room->askForChoice(sunce, "zhiba_pindian", "accept+reject") == "reject")
    {
        return;
    }

    source->pindian(sunce, "zhiba", this);
}

class ZhibaPindian: public OneCardViewAsSkill{
public:
    ZhibaPindian():OneCardViewAsSkill("zhiba_pindian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ZhibaCard") && player->getKingdom() == "wu" && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ZhibaCard *card = new ZhibaCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class SunceZhiba: public TriggerSkill{
public:
    SunceZhiba():TriggerSkill("sunce_zhiba$"){
        events << GameStart << Pindian;

        frequency = Wake;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == GameStart){
            if(!player->hasSkill(objectName()))
                return false;

            Room *room = player->getRoom();
            foreach(ServerPlayer *p, room->getAlivePlayers()){
                if(!p->hasSkill("zhiba_pindian"))
                    room->attachSkillToPlayer(p, "zhiba_pindian");
            }
        }else if(event == Pindian){
            PindianStar pindian = data.value<PindianStar>();
            if(pindian->reason == "zhiba" &&
               pindian->to->hasSkill(objectName()) &&
               pindian->from_card->getNumber() <= pindian->to_card->getNumber())
            {
                pindian->to->obtainCard(pindian->from_card);
                pindian->to->obtainCard(pindian->to_card);
            }
        }

        return false;
    }
};

TiaoxinCard::TiaoxinCard(){
    once = true;
}

bool TiaoxinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->canSlash(Self);
}

void TiaoxinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    const Card *slash = room->askForCard(effect.to, "slash", "@tiaoxin-slash");

    if(slash){
        CardUseStruct use;
        use.card = slash;
        use.to << effect.from;
        use.from = effect.to;
        room->useCard(use);
    }else if(!effect.to->isNude()){
        room->throwCard(room->askForCardChosen(effect.from, effect.to, "he", "tiaoxin"));
    }
}

class Tiaoxin: public ZeroCardViewAsSkill{
public:
    Tiaoxin():ZeroCardViewAsSkill("tiaoxin"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("TiaoxinCard");
    }

    virtual const Card *viewAs() const{
        return new TiaoxinCard;
    }
};

class Zhiji: public PhaseChangeSkill{
public:
    Zhiji():PhaseChangeSkill("zhiji"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("zhiji") == 0
                && target->getPhase() == Player::Start
                && target->isKongcheng();
    }

    virtual bool onPhaseChange(ServerPlayer *jiangwei) const{
        Room *room = jiangwei->getRoom();

        LogMessage log;
        log.type = "#ZhijiWake";
        log.from = jiangwei;
        room->sendLog(log);

        if(room->askForChoice(jiangwei, objectName(), "recover+draw") == "recover"){
            RecoverStruct recover;
            recover.who = jiangwei;
            room->recover(jiangwei, recover);
        }else
            room->drawCards(jiangwei, 2);

        room->setPlayerMark(jiangwei, "zhiji", 1);
        room->acquireSkill(jiangwei, "guanxing");

        room->loseMaxHp(jiangwei);

        const TriggerSkill *guanxing = Sanguosha->getTriggerSkill("guanxing");
        if(room->getThread()->getRefCount(guanxing) == 0){
            QVariant void_data;
            guanxing->trigger(PhaseChange, jiangwei, void_data);
        }

        return false;
    }
};

ZhijianCard::ZhijianCard(){
    will_throw = false;
}

bool ZhijianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty() || to_select == Self)
        return false;

    const Card *card = Sanguosha->getCard(subcards.first());
    const EquipCard *equip = qobject_cast<const EquipCard *>(card);
    int equip_index = static_cast<int>(equip->location());
    return to_select->getEquip(equip_index) == NULL;
}

void ZhijianCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *erzhang = effect.from;
    erzhang->getRoom()->moveCardTo(this, effect.to, Player::Equip);
    erzhang->drawCards(1);
}

class Zhijian: public OneCardViewAsSkill{
public:
    Zhijian():OneCardViewAsSkill("zhijian"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getTypeId() == Card::Equip;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ZhijianCard *zhijian_card = new ZhijianCard();
        zhijian_card->addSubcard(card_item->getFilteredCard());
        return zhijian_card;
    }
};

class Guzheng: public TriggerSkill{
public:
    Guzheng():TriggerSkill("guzheng"){
        events << CardDiscarded;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("guzheng");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *erzhang = room->findPlayerBySkillName(objectName());

        if(erzhang == NULL)
            return false;

        if(player->getPhase() == Player::Discard){
            QVariantList guzheng = erzhang->tag["Guzheng"].toList();

            CardStar card = data.value<CardStar>();
            foreach(int card_id, card->getSubcards()){
                guzheng << card_id;
            }

            erzhang->tag["Guzheng"] = guzheng;
        }

        return false;
    }
};

class GuzhengGet: public PhaseChangeSkill{
public:
    GuzhengGet():PhaseChangeSkill("#guzheng-get"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("guzheng");
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->isDead())
            return false;

        Room *room = player->getRoom();
        ServerPlayer *erzhang = room->findPlayerBySkillName(objectName());
        if(erzhang == NULL)
            return false;

        QVariantList guzheng_cards = erzhang->tag["Guzheng"].toList();
        erzhang->tag.remove("Guzheng");

        QList<int> cards;
        foreach(QVariant card_data, guzheng_cards){
            int card_id = card_data.toInt();
            if(room->getCardPlace(card_id) == Player::DiscardedPile)
                cards << card_id;
        }

        if(cards.isEmpty())
            return false;

        if(erzhang->askForSkillInvoke("guzheng", cards.length())){
            room->fillAG(cards, erzhang);

            int to_back = room->askForAG(erzhang, cards, false, objectName());
            player->obtainCard(Sanguosha->getCard(to_back));

            cards.removeOne(to_back);

            erzhang->invoke("clearAG");

            foreach(int card_id, cards)
                erzhang->obtainCard(Sanguosha->getCard(card_id));
        }

        return false;
    }
};

class BasicPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return ! player->hasEquip(card) && card->getTypeId() == Card::Basic;
    }
};

class Xiangle: public TriggerSkill{
public:
    Xiangle():TriggerSkill("xiangle"){
        events << CardEffected;

        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.card->inherits("Slash")){
            Room *room = player->getRoom();

            LogMessage log;
            log.type = "#Xiangle";
            log.from = effect.from;
            log.to << effect.to;
            room->sendLog(log);

            return !room->askForCard(effect.from, ".basic", "@xiangle-discard");
        }

        return false;
    }
};

class Fangquan: public PhaseChangeSkill{
public:
    Fangquan():PhaseChangeSkill("fangquan"){

    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *liushan) const{
        switch(liushan->getPhase()){
        case Player::Play: {
                bool invoked = liushan->askForSkillInvoke(objectName());
                if(invoked)
                    liushan->setFlags("fangquan");
                return invoked;
            }

        case Player::Finish: {
                if(liushan->hasFlag("fangquan")){
                    Room *room = liushan->getRoom();

                    if(liushan->isKongcheng())
                        return false;

                    ServerPlayer *player = room->askForPlayerChosen(liushan, room->getOtherPlayers(liushan), objectName());

                    LogMessage log;
                    log.type = "#Fangquan";
                    log.from = liushan;
                    log.to << player;
                    room->sendLog(log);

                    room->setCurrent(player);
                    room->getThread()->trigger(TurnStart, player);
                    room->setCurrent(liushan);
                }

                break;
            }

        default:
            break;
        }

        return false;
    }
};

class Ruoyu: public PhaseChangeSkill{
public:
    Ruoyu():PhaseChangeSkill("ruoyu$"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("ruoyu") == 0
                && target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *liushan) const{
        Room *room = liushan->getRoom();

        bool can_invoke = true;
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(liushan->getHp() > p->getHp()){
                can_invoke = false;
                break;
            }
        }

        if(can_invoke){
            LogMessage log;
            log.type = "#RuoyuWake";
            log.from = liushan;
            log.arg = QString::number(liushan->getHp());
            room->sendLog(log);

            room->setPlayerMark(liushan, "ruoyu", 1);
            room->setPlayerProperty(liushan, "maxhp", liushan->getMaxHP() + 1);

            RecoverStruct recover;
            recover.who = liushan;
            room->recover(liushan, recover);

            room->acquireSkill(liushan, "jijiang");
        }

        return false;
    }
};

class Huashen: public GameStartSkill{
public:
    Huashen():GameStartSkill("huashen"){

    }

    static void AcquireGenerals(ServerPlayer *zuoci, int n){
        QStringList list = GetAvailableGenerals(zuoci);
        qShuffle(list);

        QStringList acquired = list.mid(0, n);
        QVariantList huashens = zuoci->tag["Huashens"].toList();
        foreach(QString huashen, acquired){
            huashens << huashen;

            // FIXME: do general move animation
        }

        zuoci->tag["Huashens"] = huashens;
    }

    static QStringList GetAvailableGenerals(ServerPlayer *zuoci){
        QSet<QString> all = Sanguosha->getLimitedGeneralNames().toSet();
        QSet<QString> huashen_set, room_set;
        QVariantList huashens = zuoci->tag["Huashens"].toList();
        foreach(QVariant huashen, huashens)
            huashen_set << huashen.toString();

        Room *room = zuoci->getRoom();
        QList<const ServerPlayer *> players = room->findChildren<const ServerPlayer *>();
        foreach(const ServerPlayer *player, players){
            room_set << player->getGeneralName();
            if(player->getGeneral2())
                room_set << player->getGeneral2Name();
        }

        all.remove("zuoci");
        all.remove("zuocif");

        return (all - huashen_set - room_set).toList();
    }

    static QString SelectSkill(ServerPlayer *zuoci){
        Room *room = zuoci->getRoom();

        QString huashen_skill = zuoci->tag["HuashenSkill"].toString();
        if(!huashen_skill.isEmpty())
            room->detachSkillFromPlayer(zuoci, huashen_skill);

        QVariantList huashens = zuoci->tag["Huashens"].toList();
        QStringList huashen_generals;
        foreach(QVariant huashen, huashens)
            huashen_generals << huashen.toString();

        QString general_name = room->askForGeneral(zuoci, huashen_generals);
        const General *general = Sanguosha->getGeneral(general_name);
        if(zuoci->getKingdom() != general->getKingdom())
            room->setPlayerProperty(zuoci, "kingdom", general->getKingdom());
        if(zuoci->getGeneral()->isMale() != general->isMale())
            room->setPlayerProperty(zuoci, "general", general->isMale() ? "zuoci" : "zuocif");

        QStringList skill_names;
        foreach(const Skill *skill, general->getVisibleSkillList()){
            if(skill->isLordSkill() || skill->getFrequency() == Skill::Limited
               || skill->getFrequency() == Skill::Wake)
                continue;

            skill_names << skill->objectName();
        }

        if(skill_names.isEmpty())
            return QString();

        QString skill_name;
        if(skill_names.length() == 1)
            skill_name = skill_names.first();
        else
            skill_name = room->askForChoice(zuoci, "huashen", skill_names.join("+"));

        zuoci->tag["HuashenSkill"] = skill_name;
        room->acquireSkill(zuoci, skill_name);

        return skill_name;
    }

    virtual void onGameStart(ServerPlayer *zuoci) const{
        AcquireGenerals(zuoci, 2);
        SelectSkill(zuoci);
    }
};

class HuashenBegin: public PhaseChangeSkill{
public:
    HuashenBegin():PhaseChangeSkill("#huashen-begin"){

    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *zuoci) const{
        QString skill_name = Huashen::SelectSkill(zuoci);
        const TriggerSkill *skill = Sanguosha->getTriggerSkill(skill_name);
        if(skill && zuoci->getRoom()->getThread()->getRefCount(skill) == 1
           && skill->getTriggerEvents().contains(PhaseChange)
            && skill->triggerable(zuoci)){

            QVariant void_data;
            skill->trigger(PhaseChange, zuoci, void_data);
        }


        return false;
    }
};

class HuashenEnd: public PhaseChangeSkill{
public:
    HuashenEnd():PhaseChangeSkill("#huashen-end"){

    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::NotActive;
    }

    virtual bool onPhaseChange(ServerPlayer *zuoci) const{
        Huashen::SelectSkill(zuoci);

        return false;
    }
};

class Xinsheng: public MasochismSkill{
public:
    Xinsheng():MasochismSkill("xinsheng"){

    }

    virtual void onDamaged(ServerPlayer *zuoci, const DamageStruct &damage) const{
        int n = damage.damage;
        if(n == 0)
            return;

        Huashen::AcquireGenerals(zuoci, n);
    }
};

MountainPackage::MountainPackage()
    :Package("mountain")
{
    General *zhanghe = new General(this, "zhanghe", "wei");
    zhanghe->addSkill(new Qiaobian);

    /*
    General *dengai = new General(this, "dengai", "wei", 3);
    dengai->addSkill(new Tuntian);
    dengai->addSkill(new TuntianGet);
    dengai->addSkill(new Jixi);
    dengai->addSkill(new JixiWake);
    dengai->addSkill(new JixiThrow);
    */

    General *liushan = new General(this, "liushan$", "shu", 3);
    liushan->addSkill(new Xiangle);
    liushan->addSkill(new Fangquan);
    liushan->addSkill(new Ruoyu);

    General *jiangwei = new General(this, "jiangwei", "shu");
    jiangwei->addSkill(new Tiaoxin);
    jiangwei->addSkill(new Zhiji);

    General *sunce = new General(this, "sunce$", "wu");
    sunce->addSkill(new Jiang);
    sunce->addSkill(new Hunzi);
    sunce->addSkill(new SunceZhiba);

    General *erzhang = new General(this, "erzhang", "wu", 3);
    erzhang->addSkill(new Zhijian);
    erzhang->addSkill(new Guzheng);
    erzhang->addSkill(new GuzhengGet);

    related_skills.insertMulti("guzheng", "#guzheng-get");

    General *caiwenji = new General(this, "caiwenji", "qun", 3, false);
    caiwenji->addSkill(new Beige);
    caiwenji->addSkill(new Duanchang);

    General *zuoci = new General(this, "zuoci", "qun", 3);
    zuoci->addSkill(new Huashen);
    zuoci->addSkill(new HuashenBegin);
    zuoci->addSkill(new HuashenEnd);
    zuoci->addSkill(new Xinsheng);

    General *zuocif = new General(this, "zuocif", "qun", 3, false, true);
    zuocif->addSkill("huashen");
    zuocif->addSkill("#huashen-begin");
    zuocif->addSkill("#huashen-end");
    zuocif->addSkill("xinsheng");

    related_skills.insertMulti("huashen", "#huashen-begin");
    related_skills.insertMulti("huashen", "#huashen-end");

    addMetaObject<QiaobianCard>();
    addMetaObject<TiaoxinCard>();
    addMetaObject<ZhijianCard>();
    addMetaObject<ZhibaCard>();

    skills << new ZhibaPindian;

    patterns[".basic"] = new BasicPattern;
}

ADD_PACKAGE(Mountain);
