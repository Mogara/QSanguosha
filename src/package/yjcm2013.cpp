#include "yjcm2013.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"

class Chengxiang: public MasochismSkill {
public:
    Chengxiang(): MasochismSkill("chengxiang") {
    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &) const{
        Room *room = target->getRoom();
        if(!target->askForSkillInvoke(objectName()))
            return;
        room->playSkillEffect(objectName());

        QList<int> card_ids = room->getNCards(4);
        room->fillAG(card_ids);

        int sum = 0;
        while(!card_ids.isEmpty()){
            int card_id = room->askForAG(target, card_ids, true, objectName());
            if(card_id < 0)
                break;
            const Card *card = Sanguosha->getCard(card_id);
            if(sum + card->getNumber() >= 13)
                continue;
            card_ids.removeOne(card_id);
            room->takeAG(target, card_id);

            sum += card->getNumber();

            bool invoke = false;
            foreach(int card_id, card_ids){
                card = Sanguosha->getCard(card_id);
                if(sum + card->getNumber() < 13){
                    invoke = true;
                    break;
                }
            }
            if(!invoke)
                break;
        }

        room->broadcastInvoke("clearAG");
        foreach(int card_id, card_ids)
            room->throwCard(card_id);
    }
};

class Renxin: public TriggerSkill {
public:
    Renxin(): TriggerSkill("renxin") {
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        QList<ServerPlayer *> caochs = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *caochong, caochs){
            if(caochong == player)
                continue;
            if(!caochong->isKongcheng() && caochong->askForSkillInvoke(objectName(), data)){
                room->playSkillEffect(objectName());
                caochong->turnOver();
                DummyCard *dcard = caochong->wholeHandCards();
                room->moveCardTo(dcard, player, Player::Hand, false);
                delete dcard;

                RecoverStruct recover;
                room->recover(player, recover, true);
                if(player->getHp() > 0)
                    break;
            }
        }
        return false;
    }
};

class Jingce: public TriggerSkill {
public:
    Jingce(): TriggerSkill("jingce") {
        events << CardUsed << PhaseEnd;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        if(event == CardUsed){
            if(player->getPhase() <= Player::Play){
                CardStar card = data.value<CardUseStruct>().card;
                if(card && !card->isVirtualCard())
                    player->addMark(objectName());
            }
        }else{
            if(player->getPhase() == Player::Play){
                if(player->getMark(objectName()) >= player->getHp() && player->askForSkillInvoke(objectName())){
                    room->playSkillEffect(objectName());
                    player->drawCards(2);
                }
                player->loseAllMarks(objectName());
            }
        }
        return false;
    }
};
class Longyin: public TriggerSkill {
public:
    Longyin(): TriggerSkill("longyin") {
        events << CardRecord << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::Play)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card->isKindOf("Slash"))
            return false;
        if(event == CardRecord){
            ServerPlayer *guanping = room->findPlayerBySkillName(objectName());
            if(guanping && !guanping->isNude()){
                QString propty = QString("@longyin:%1:%2:%3:%4")
                                 .arg(use.from->objectName())
                                 .arg(use.to.first()->objectName())
                                 .arg(use.card->objectName())
                                 .arg(use.card->getSuitString());
                const Card *exc = room->askForCard(guanping, "..", propty, data, CardDiscarded);
                if(exc && exc != use.card){
                    room->playSkillEffect(objectName());
                    use.card->setFlags("longyin");
                    return true;
                }
            }
        }
        else{
            if(use.from == player && use.card->hasFlag("longyin")){
                use.card->setFlags("-longyin");
                ServerPlayer *guanping = room->findPlayerBySkillName(objectName());
                LogMessage log;
                log.type = "#LongyiN";
                log.from = guanping;
                log.to << player;
                log.arg = objectName();
                room->sendLog(log);

                if(use.card->isRed())
                    guanping->drawCards(1);
            }
        }
        return false;
    }
};

XiansiCard::XiansiCard() {
}

bool XiansiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    return targets.length() < 2 && !to_select->isNude();
}

void XiansiCard::onEffect(const CardEffectStruct &effect) const{
    if (effect.to->isNude()) return;
    int id = effect.from->getRoom()->askForCardChosen(effect.from, effect.to, "he", skill_name);
    effect.from->addToPile("counter", id);
}

class XiansiViewAsSkill: public ZeroCardViewAsSkill {
public:
    XiansiViewAsSkill(): ZeroCardViewAsSkill("xiansi") {
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@xiansi";
    }

    virtual const Card *viewAs() const{
        return new XiansiCard;
    }
};

class Xiansi: public PhaseChangeSkill{
public:
    Xiansi():PhaseChangeSkill("xiansi"){
        view_as_skill = new XiansiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Start)
            player->getRoom()->askForUseCard(player, "@@xiansi", "@xiansi-card");
        return false;
    }
};

class XiansiAttach: public GameStartSkill{
public:
    XiansiAttach():GameStartSkill("#xiansi-attach"){
    }

    virtual void onGameStart(ServerPlayer *liufeng) const{
        Room *room = liufeng->getRoom();
        QList<ServerPlayer *> players = room->getAlivePlayers();
        foreach(ServerPlayer *player, players)
            room->attachSkillToPlayer(player, "xiansiv");
    }

    virtual void onIdied(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(room->findPlayerBySkillName("xiansi"))
            return;
        QList<ServerPlayer *> players = room->getAlivePlayers();
        foreach(ServerPlayer *tmp, players)
            room->detachSkillFromPlayer(tmp, "xiansiv", false);
    }
};

XiansiSlashCard::XiansiSlashCard() {
    target_fixed = true;
}

void XiansiSlashCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    ServerPlayer *liufeng = room->findPlayerBySkillName("xiansi");
    if (!liufeng || liufeng->getPile("counter").length() < 2)
        return;

    room->throwCard(liufeng->getPile("counter").first(), source);
    room->throwCard(liufeng->getPile("counter").first(), source);

    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("xiansislash");
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to << liufeng;
    room->useCard(use);
}

class Xiansiv: public ZeroCardViewAsSkill {
public:
    Xiansiv(): ZeroCardViewAsSkill("xiansiv"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player) && canSlashLiufeng(player);
    }
/*
    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "slash" && !ClientInstance->hasNoTargetResponding()
               && canSlashLiufeng(player);
    }
*/
    virtual const Card *viewAs() const{
        return new XiansiSlashCard;
    }

private:
    static bool canSlashLiufeng(const Player *player) {
        const Player *liufeng = NULL;
        foreach (const Player *p, player->getSiblings()) {
            if (p->isAlive() && p->hasSkill("xiansi") && p->getPile("counter").count() >= 2) {
                liufeng = p;
                break;
            }
        }
        if (!liufeng) return false;

        QList<const Player *> empty_list;
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->deleteLater();
        return slash->targetFilter(empty_list, liufeng, player);
    }
};

class Duodao:public MasochismSkill{
public:
    Duodao():MasochismSkill("duodao"){
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        if(!damage.from || !damage.from->getWeapon())
            return;
        const Card *card = damage.card;
        if(card->isKindOf("Slash") && room->askForCard(player, "..", "@duodao", QVariant::fromValue(damage), CardDiscarded)){
            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);
            room->playSkillEffect(objectName());

            player->obtainCard(damage.from->getWeapon());
        }
    }
};

class Anjian: public TriggerSkill{
public:
    Anjian():TriggerSkill("anjian"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->isKindOf("Slash") && !damage.to->inMyAttackRange(player)){

            LogMessage log;
            log.type = "#AnjianEffect";
            log.from = player;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            room->sendLog(log);
            room->playSkillEffect(objectName());

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class Danshou: public TriggerSkill{
public:
    Danshou():TriggerSkill("danshou"){
        events << Damage;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *zhu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(room->getMode() == "02_1v1" && damage.to->isDead())
            return false;
        if(zhu->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            zhu->drawCards(1);
            room->getThread()->trigger(Interrupt, room->getCurrent(), data);
            return true;
        }
        return false;
    }
};

class Zhuikong: public PhaseChangeSkill{
public:
    Zhuikong():PhaseChangeSkill("zhuikong"){
        //view_as_skill = new ZhuikongViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::NotActive){
            PlayerStar fuhuanghou = player->tag["Zhuikong"].value<PlayerStar>();
            if(fuhuanghou)
                player->setFixedDistance(fuhuanghou, -1);
            return false;
        }
        if(player->getPhase() != Player::RoundStart || player->isKongcheng())
            return false;

        Room *room = player->getRoom();
        QList<ServerPlayer *> fus = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *fuhuanghou, fus){
            if(player->isKongcheng())
                break;
            if(fuhuanghou == player || !fuhuanghou->isWounded() || fuhuanghou->isKongcheng())
                continue;
            if(fuhuanghou->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());
                bool success = fuhuanghou->pindian(player, objectName());
                if(success)
                    player->skip(Player::Play);
                else{
                    player->tag["Zhuikong"] = QVariant::fromValue((PlayerStar)fuhuanghou);
                    player->setFixedDistance(fuhuanghou, 1);
                }
            }
        }
        return false;
    }
};

class Qiuyuan:public TriggerSkill{
public:
    Qiuyuan():TriggerSkill("qiuyuan"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card->isKindOf("Slash"))
            return false;
        foreach(ServerPlayer *fuhuanghou, use.to){
            if(!fuhuanghou->hasSkill(objectName()))
                continue;
            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *tmp, room->getOtherPlayers(use.from)){
                if(tmp->isKongcheng())
                    continue;
                if(!use.to.contains(tmp))
                    targets << tmp;
            }
            if(!targets.isEmpty() && fuhuanghou->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());
                ServerPlayer *target = room->askForPlayerChosen(fuhuanghou, targets, objectName());
                const Card *card = room->askForCardShow(target, fuhuanghou, objectName());
                fuhuanghou->obtainCard(card);
                if(!card->inherits("Jink"))
                    use.to << target;
            }
        }
        data = QVariant::fromValue(use);
        return false;
    }
};

class Juece: public TriggerSkill{
public:
    Juece():TriggerSkill("juece"){
        events << CardLost;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *liru = room->getCurrent();
        if(!liru || !liru->hasSkill(objectName()))
            return false;
        if(player->isKongcheng() && player->isAlive()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand){
                if(room->askForSkillInvoke(liru, objectName(), data)){
                    room->playSkillEffect(objectName());
                    DamageStruct damage;
                    damage.from = liru;
                    damage.to = player;
                    room->damage(damage);
                }
            }
        }

        return false;
    }
};

class Mieji: public TargetModSkill {
public:
    Mieji(): TargetModSkill("mieji"){
        pattern = "SingleTargetTrick|.|.|.|black";
        frequency = NotFrequent;
    }

    virtual int getExtraTargetNum(const Player *from, const Card *card) const{
        if(from->hasSkill("mieji") && card->isNDTrick() && card->isBlack() &&
           (!card->isKindOf("Nullification") && !card->isKindOf("Collateral")))
            return 1;
        else
            return 0;
    }
};

FenchengCard::FenchengCard(){
    target_fixed = true;
}

void FenchengCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->loseMark("@conflagration");
    if(!Config.DisableLightbox)
        room->broadcastInvoke("animate", "lightbox:$fencheng");
    else
        room->setEmotion(source, "skill/" + skill_name);

    QList<ServerPlayer *> players = room->getOtherPlayers(source);
    foreach(ServerPlayer *player, players){
        if(player->isAlive())
            room->cardEffect(this, source, player);
    }
}

void FenchengCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    int x = qMax(1, effect.to->getEquips().count());
    QString choice = effect.to->getCardCount(true) < x ? "fire" :
                                                         room->askForChoice(effect.to, skill_name, "fire+discard");
    if(choice == "discard")
        room->askForDiscard(effect.to, skill_name, x, false, true);
    else{
        DamageStruct damage;
        damage.nature = DamageStruct::Fire;
        damage.to = effect.to;
        room->damage(damage);
    }
}

class Fencheng: public ZeroCardViewAsSkill{
public:
    Fencheng():ZeroCardViewAsSkill("fencheng"){
        frequency = Limited;
    }

    virtual QString getDefaultChoice(ServerPlayer *) const{
        return "discard";
    }

    virtual const Card *viewAs() const{
        return new FenchengCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@conflagration") >= 1;
    }
};

class Zongxuan: public TriggerSkill{
public:
    Zongxuan():TriggerSkill("zongxuan"){
        events << CardDiscard;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardStar card = data.value<CardStar>();
        if(card && player->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            room->moveCardTo(card, NULL, Player::DrawPile);
            return true;
        }
        return false;
    }
};

class Zhiyan: public PhaseChangeSkill{
public:
    Zhiyan():PhaseChangeSkill("zhiyan"){
    }

    virtual bool onPhaseChange(ServerPlayer *yufan) const{
        if(yufan->getPhase() == Player::Finish){
            Room *room = yufan->getRoom();
            if(room->askForSkillInvoke(yufan, objectName())){
                room->playSkillEffect(objectName());
                ServerPlayer *target = room->askForPlayerChosen(yufan, room->getAlivePlayers(), objectName());
                const Card *card = room->peek();
                target->drawCards(1);
                room->showCard(target, card->getId());
                if(card->isKindOf("EquipCard")){
                    if(target->isWounded())
                        room->recover(target, RecoverStruct(), true);
                    const EquipCard *equipped = qobject_cast<const EquipCard *>(card);
                    equipped->use(room, target, QList<ServerPlayer *>());
                }
            }
        }

        return false;
    }
};

JunxingCard::JunxingCard(){
}

bool JunxingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void JunxingCard::onEffect(const CardEffectStruct &effect) const{
    QStringList intypes;
    foreach(int id, getSubcards()){
        QString type = Sanguosha->getCard(id)->getType();
        if(!intypes.contains(type))
            intypes << type;
    }

    Room *room = effect.from->getRoom();
    const Card *card = room->askForCard(effect.to, ".", QString("@junxing:%1::%2").arg(effect.from->objectName()).arg(intypes.join(",")), QVariant::fromValue(intypes.join(",")), NonTrigger);
    if(card && !intypes.contains(card->getType())){
        room->throwCard(card, effect.to);
        return;
    }

    effect.to->turnOver();
    effect.to->drawCards(getSubcards().length());
}

class Junxing: public ViewAsSkill{
public:
    Junxing():ViewAsSkill("junxing"){
    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        JunxingCard *card = new JunxingCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("JunxingCard");
    }
};

class Yuce:public MasochismSkill{
public:
    Yuce():MasochismSkill("yuce"){

    }

    virtual void onDamaged(ServerPlayer *manchong, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = manchong->getRoom();
        DamageStruct damat = damage;
        if(from && !manchong->isKongcheng() && room->askForSkillInvoke(manchong, objectName())){
            room->playSkillEffect(objectName());
            const Card *card1 = room->askForCardShow(manchong, manchong, objectName());
            damat.card = card1;
            QVariant data = QVariant::fromValue(damat);
            const Card *card2 = room->askForCard(from, ".", QString("@yuce:%1::%2").arg(manchong->objectName()).arg(card1->getType()), data, NonTrigger);
            if(card2 && card2->getType() != card1->getType()){
                room->throwCard(card2, from);
                return;
            }
            room->recover(manchong, RecoverStruct(), true);
        }
    }
};

QiaoshuiCard::QiaoshuiCard() {
    once = true;
    will_throw = false;
}

bool QiaoshuiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void QiaoshuiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    bool success = source->pindian(targets.first(), skill_name, this);
    LogMessage log;
    log.from = source;
    if(success){
        log.type = "#QiaoshuiS";
        room->setPlayerFlag(source, "qiaoshui_success");
    }
    else{
        log.type = "#QiaoshuiF";
        room->setPlayerFlag(source, "qiaoshui_failed");
        room->setPlayerCardLock(source, "TrickCard");
    }
    room->sendLog(log);
}

class QiaoshuiViewAsSkill: public OneCardViewAsSkill {
public:
    QiaoshuiViewAsSkill(): OneCardViewAsSkill("qiaoshui") {
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@qiaoshui";
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new QiaoshuiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Qiaoshui: public TriggerSkill {
public:
    Qiaoshui(): TriggerSkill("qiaoshui") {
        events << PhaseChange << CardUsed;
        view_as_skill = new QiaoshuiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *jianyong, QVariant &data) const{
        if(event == PhaseChange){
            if(jianyong->getPhase() == Player::Play)
                room->askForUseCard(jianyong, "@@qiaoshui", "@qiaoshui");
            else if(jianyong->getPhase() == Player::NotActive && jianyong->hasFlag("qiaoshui_failed"))
                room->setPlayerCardLock(jianyong, "-TrickCard");
            return false;
        }
        if(!jianyong->hasFlag("qiaoshui_success"))
            return false;

        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->isKindOf("Nullification") || use.card->isKindOf("Collateral"))
            return false;
        if(use.card->isKindOf("Shit") || use.card->isKindOf("Jink"))
            return false;
        if(use.card->getTypeId() == Card::Basic || use.card->isNDTrick()){
            room->setPlayerFlag(jianyong, "-qiaoshui_success");

            QList<ServerPlayer *> available_targets;
            if (!use.card->isKindOf("AOE") && !use.card->isKindOf("GlobalEffect")) {
                foreach (ServerPlayer *p, room->getAlivePlayers()) {
                    if (use.to.contains(p) || room->isProhibited(jianyong, p, use.card)) continue;
                    if (use.card->targetFixed()) {
                        available_targets << p;
                    } else {
                        if (use.card->targetFilter(QList<const Player *>(), p, jianyong))
                            available_targets << p;
                    }
                }
                if(use.card->targetFixed() && !use.to.contains(use.from))
                    use.to << use.from;
            }
            QStringList choices;
            choices << "remove" << "cancel";
            if(!available_targets.isEmpty())
                choices.prepend("add");

            QString choice = room->askForChoice(jianyong, objectName(), choices.join("+"), data);
            if (choice == "cancel")
                return false;
            else if (choice == "add") {
                ServerPlayer *extra = room->askForPlayerChosen(jianyong, available_targets, objectName());
                use.to.append(extra);
                qSort(use.to.begin(), use.to.end(), ServerPlayer::CompareByActionOrder);

                LogMessage log;
                log.type = "#QiaoshuiAdd";
                log.from = jianyong;
                log.to << extra;
                log.arg = use.card->objectName();
                log.arg2 = objectName();
                room->sendLog(log);
            } else {
                ServerPlayer *removed = room->askForPlayerChosen(jianyong, use.to, objectName());
                use.to.removeOne(removed);

                LogMessage log;
                log.type = "#QiaoshuiRemove";
                log.from = jianyong;
                log.to << removed;
                log.arg = use.card->objectName();
                log.arg2 = objectName();
                room->sendLog(log);
            }
        }
        data.setValue(QVariant::fromValue(use));

        return false;
    }
};

class Z0ngshi: public TriggerSkill {
public:
    Z0ngshi(): TriggerSkill("z0ngshi") {
        events << Pindian;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        if(pindian->from->hasSkill(objectName()) && pindian->from->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            if(pindian->isSuccess())
                pindian->from->obtainCard(pindian->to_card);
            else
                pindian->from->obtainCard(pindian->from_card);
        }
        if(pindian->to->hasSkill(objectName()) && pindian->to->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            if(!pindian->isSuccess())
                pindian->to->obtainCard(pindian->from_card);
            else
                pindian->to->obtainCard(pindian->to_card);
        }

        return false;
    }
};

YJCM2013Package::YJCM2013Package()
    : Package("YJCM2013")
{
    General *caochong = new General(this, "caochong", "wei", 3);
    caochong->addSkill(new Chengxiang);
    caochong->addSkill(new Renxin);

    General *guohuai = new General(this, "guohuai", "wei");
    guohuai->addSkill(new Jingce);

    General *guanping = new General(this, "guanping", "shu");
    guanping->addSkill(new Longyin);

    General *liufeng = new General(this, "liufeng", "shu");
    liufeng->addSkill(new Xiansi);
    liufeng->addSkill(new XiansiAttach);
    related_skills.insertMulti("xiansi", "#xiansi-attach");
    skills << new Xiansiv;
    addMetaObject<XiansiCard>();
    addMetaObject<XiansiSlashCard>();

    General *panzhangmazhong = new General(this, "panzhangmazhong", "wu");
    panzhangmazhong->addSkill(new Duodao);
    panzhangmazhong->addSkill(new Anjian);

    General *zhuran = new General(this, "zhuran", "wu");
    zhuran->addSkill(new Danshou);

    General *fuhuanghou = new General(this, "fuhuanghou", "qun", 3, false);
    fuhuanghou->addSkill(new Zhuikong);
    fuhuanghou->addSkill(new Qiuyuan);

    General *liru = new General(this, "liru", "qun", 3);
    liru->addSkill(new Juece);
    liru->addSkill(new Mieji);
    liru->addSkill(new Fencheng);
    liru->addSkill(new MarkAssignSkill("@conflagration", 1));
    addMetaObject<FenchengCard>();

    General *yufan = new General(this, "yufan", "wu", 3);
    yufan->addSkill(new Zongxuan);
    yufan->addSkill(new Zhiyan);

    General *manchong = new General(this, "manchong", "wei", 3);
    manchong->addSkill(new Junxing);
    manchong->addSkill(new Yuce);
    addMetaObject<JunxingCard>();

    General *jianyong = new General(this, "jianyong", "shu", 3);
    jianyong->addSkill(new Qiaoshui);
    jianyong->addSkill(new Z0ngshi);
    addMetaObject<QiaoshuiCard>();

}

ADD_PACKAGE(YJCM2013)
