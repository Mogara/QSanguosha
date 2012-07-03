#include "mountainpackage.h"

#include "general.h"
#include "skill.h"
#include "engine.h"
#include "standard.h"
#include "carditem.h"
#include "generaloverview.h"
#include "clientplayer.h"
#include "client.h"
#include "ai.h"

#include <QCommandLinkButton>

QiaobianCard::QiaobianCard(){
    mute = true;
}

bool QiaobianCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(Self->getPhase() == Player::Draw)
        return targets.length() <= 2;
    else if(Self->getPhase() == Player::Play)
        return targets.length() <= 1;
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

void QiaobianCard::use(Room *room, ServerPlayer *zhanghe, QList<ServerPlayer *> &targets) const{
    room->throwCard(this, zhanghe);

    if(zhanghe->getPhase() == Player::Draw){
        room->broadcastSkillInvoke("qiaobian", 2);
        if(targets.isEmpty())
            return;

        QList<ServerPlayer *> players = targets;
        qSort(players.begin(), players.end(), ServerPlayer::CompareByActionOrder);
        foreach(ServerPlayer *target, players){
            room->cardEffect(this, zhanghe, target);
        }
    }else if(zhanghe->getPhase() == Player::Play){
        room->broadcastSkillInvoke("qiaobian", 3);
        if(targets.isEmpty())
            return;

        PlayerStar from = targets.first();
        if(!from->hasEquip() && from->getJudgingArea().isEmpty())
            return;

        int card_id = room->askForCardChosen(zhanghe, from , "ej", "qiaobian");
        const Card *card = Sanguosha->getCard(card_id);
        Player::Place place = room->getCardPlace(card_id);

        int equip_index = -1;
        const DelayedTrick *trick = NULL;
        if(place == Player::PlaceEquip){
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

        room->setTag("QiaobianTarget", QVariant::fromValue(from));
        ServerPlayer *to = room->askForPlayerChosen(zhanghe, tos, "qiaobian");
        if(to)
            room->moveCardTo(card, from, to, place,
                CardMoveReason(CardMoveReason::S_REASON_TRANSFER, zhanghe->objectName(), "qiaobian", QString()));
        room->removeTag("QiaobianTarget");
    }
    else if(zhanghe->getPhase() == Player::Judge)
        room->broadcastSkillInvoke("qiaobian", 1);
    else
        room->broadcastSkillInvoke("qiaobian", 4);
}

void QiaobianCard::onEffect(const CardEffectStruct &effect) const{
    if(effect.from->getPhase() == Player::Draw){
        Room *room = effect.from->getRoom();
        if(!effect.to->isKongcheng()){
            int card_id = room->askForCardChosen(effect.from, effect.to, "h", "qiaobian");
            CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, effect.from->objectName());
            room->obtainCard(effect.from, Sanguosha->getCard(card_id), reason, false);

            room->setEmotion(effect.to, "bad");
            room->setEmotion(effect.from, "good");
        }
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
        return target != NULL && PhaseChangeSkill::triggerable(target) && !target->isKongcheng();
    }

    virtual bool onPhaseChange(ServerPlayer *zhanghe) const{
        Room *room = zhanghe->getRoom();

        switch(zhanghe->getPhase()){
        case Player::RoundStart:
        case Player::Start:
        case Player::Finish:
        case Player::NotActive: return false;

        case Player::Judge: return room->askForUseCard(zhanghe, "@qiaobian", "@qiaobian-judge", 1);
        case Player::Draw: return room->askForUseCard(zhanghe, "@qiaobian", "@qiaobian-draw", 2);
        case Player::Play: return room->askForUseCard(zhanghe, "@qiaobian", "@qiaobian-play", 3);
        case Player::Discard: return room->askForUseCard(zhanghe, "@qiaobian", "@qiaobian-discard", 1);
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
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card == NULL || !damage.card->inherits("Slash") || damage.to->isDead())
            return false;

        QList<ServerPlayer *> cais = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *caiwenji, cais){
            if(!caiwenji->isNude() && caiwenji->askForSkillInvoke(objectName(), data)){
                room->askForDiscard(caiwenji, "beige", 1, 1, false, true);

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(.*):(.*)");
                judge.good = true;
                judge.who = player;
                judge.reason = objectName();

                room->judge(judge);

                switch(judge.card->getSuit()){
                case Card::Heart:{
                        room->broadcastSkillInvoke(objectName(), 4);
                        RecoverStruct recover;
                        recover.who = caiwenji;
                        room->recover(player, recover);

                        break;
                    }

                case Card::Diamond:{
                        room->broadcastSkillInvoke(objectName(), 3);
                        player->drawCards(2);
                        break;
                    }

                case Card::Club:{
                        room->broadcastSkillInvoke(objectName(), 1);
                        if(damage.from && damage.from->isAlive()){
                            int to_discard = qMin(2, damage.from->getCardCount(true));
                            if(to_discard != 0)
                                room->askForDiscard(damage.from, "beige", to_discard, to_discard, false, true);
                        }

                        break;
                    }

                case Card::Spade:{
                        room->broadcastSkillInvoke(objectName(), 2);
                        if(damage.from && damage.from->isAlive())
                            damage.from->turnOver();

                        break;
                    }

                default:
                    break;
                }
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
        return target != NULL && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();

        if(Config.EnableBasara){
            // @:todo if the killer is anjiang,do nothing;else turn him to anjiang again
            return false;
        }
        if(damage && damage->from && damage->from->getGeneralName() != "anjiang"){
            if (player == NULL) return false;

            LogMessage log;
            log.type = "#DuanchangLoseSkills";
            log.from = player;
            log.to << damage->from;
            log.arg = objectName();
            room->sendLog(log);
            room->broadcastSkillInvoke(objectName());

            QList<const Skill *> skills = damage->from->getVisibleSkillList();
            foreach(const Skill *skill, skills){
                if(skill->getLocation() == Skill::Right)
                    room->detachSkillFromPlayer(damage->from, skill->objectName());
            }
            damage->from->gainMark("@duanchang");
            if(damage->from->getKingdom() != damage->from->getGeneral()->getKingdom())
                room->setPlayerProperty(damage->from, "kingdom", damage->from->getGeneral()->getKingdom());
            if(damage->from->getGeneralName() == "zuocif")
                room->setPlayerProperty(damage->from, "general", "zuoci");
            if(damage->from->getHp() <= 0)
                room->enterDying(damage->from,NULL);
            damage->from->clearPrivatePiles();
            if(damage->from->hasSkill("qixing")){
                if(damage->from->getMark("@star") > 0)
                    damage->from->loseAllMarks("@star");
                QList<ServerPlayer *> players = room->getAllPlayers();
                foreach(ServerPlayer *player, players){
                    player->loseAllMarks("@gale");
                    player->loseAllMarks("@fog");
                }
            }
            else if(damage->from->hasSkill("wuhun")){
                QList<ServerPlayer *> players = room->getAllPlayers();
                foreach(ServerPlayer *player, players){
                    player->loseAllMarks("@nightmare");
                }
            }
            else if(damage->from->hasSkill("renjie")){
                damage->from->loseAllMarks("@bear");
            }
        }

        return false;
    }
};

class Tuntian: public DistanceSkill{
public:
    Tuntian():DistanceSkill("tuntian"){
        frequency = NotFrequent;
    }

    virtual int getCorrect(const Player *from, const Player *) const{
        if(from->hasSkill(objectName()))
            return -from->getPile("field").length();
        else
            return 0;
    }
};

class TuntianGet: public TriggerSkill{
public:
    TuntianGet():TriggerSkill("#tuntian-get"){
        events << CardLostOneTime << FinishJudge;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && TriggerSkill::triggerable(target) && target->getPhase() == Player::NotActive;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if (player == NULL) return false;
        if(event == CardLostOneTime){
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            if((move->from_places.contains(Player::PlaceHand) || move->from_places.contains(Player::PlaceEquip)) &&
                player->getRoom()->getCurrent() != player
                && player->askForSkillInvoke("tuntian", data)){                
                room->broadcastSkillInvoke("tuntian");
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart):(.*)");
                judge.good = false;
                judge.reason = "tuntian";
                judge.who = player;
                room->judge(judge);
            }
        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == "tuntian" && judge->isGood()){
                player->addToPile("field", judge->card->getEffectiveId());
                return true;
            }
        }

        return false;
    }
};

class Zaoxian: public PhaseChangeSkill{
public:
    Zaoxian():PhaseChangeSkill("zaoxian"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("zaoxian") == 0
                && target->getPile("field").length() >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *dengai) const{
        Room *room = dengai->getRoom();

        room->setPlayerMark(dengai, "zaoxian", 1);
        dengai->gainMark("@waked");
        room->loseMaxHp(dengai);

        LogMessage log;
        log.type = "#ZaoxianWake";
        log.from = dengai;
        log.arg = QString::number(dengai->getPile("field").length());
        log.arg2 = objectName();
        room->sendLog(log);

        room->broadcastSkillInvoke("zaoxian");
        room->broadcastInvoke("animate", "lightbox:$zaoxian:4000");
        room->getThread()->delay(4000);

        room->acquireSkill(dengai, "jixi");

        return false;
    }
};

JixiCard::JixiCard(){
    target_fixed = true;
}

void JixiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *dengai = card_use.from;

    QList<int> fields = dengai->getPile("field");
    if(fields.isEmpty())
        return ;

    int card_id;
    if(fields.length() == 1)
        card_id = fields.first();
    else{
        room->fillAG(fields, dengai);
        card_id = room->askForAG(dengai, fields, true, "jixi");
        dengai->invoke("clearAG");

        if(card_id == -1)
            return;
    }

    const Card *card = Sanguosha->getCard(card_id);
    Snatch *snatch = new Snatch(card->getSuit(), card->getNumber());
    snatch->setSkillName("jixi");
    snatch->addSubcard(card_id);

    QList<ServerPlayer *> targets;
    QList<const Player *> empty_list;
    foreach(ServerPlayer *p, room->getAlivePlayers()){
        if(!snatch->targetFilter(empty_list, p, dengai))
            continue;
        if(dengai->distanceTo(p,1) > 1)
            continue;
        if(dengai->isProhibited(p, snatch))
            continue;

        targets << p;
    }

    if(targets.isEmpty())
        return;

    ServerPlayer *target = room->askForPlayerChosen(dengai, targets, "jixi");

    CardUseStruct use;
    use.card = snatch;
    use.from = dengai;
    use.to << target;

    room->useCard(use);
}

class Jixi:public ZeroCardViewAsSkill{
public:
    Jixi():ZeroCardViewAsSkill("jixi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->getPile("field").isEmpty();
    }

    virtual const Card *viewAs() const{
        return new JixiCard;
    }

    virtual Location getLocation() const{
        return Right;
    }
};

class Jiang: public TriggerSkill{
public:
    Jiang():TriggerSkill("jiang"){
        events << TargetConfirmed;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *sunce, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.from->objectName() == sunce->objectName() || use.to.contains(sunce)){
            if(use.card->inherits("Duel") || (use.card->inherits("Slash") && use.card->isRed())){
                if(sunce->askForSkillInvoke(objectName(), data)){
                    sunce->getRoom()->broadcastSkillInvoke(objectName());
                    sunce->drawCards(1);
                }
            }
        }

        return false;
    }
};

class Hunzi: public PhaseChangeSkill{
public:
    Hunzi():PhaseChangeSkill("hunzi"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && PhaseChangeSkill::triggerable(target)
                && target->getMark("hunzi") == 0
                && target->getPhase() == Player::Start
                && target->getHp() == 1;
    }

    virtual bool onPhaseChange(ServerPlayer *sunce) const{
        Room *room = sunce->getRoom();

        LogMessage log;
        log.type = "#HunziWake";
        log.from = sunce;
        log.arg = objectName();
        room->sendLog(log);

        room->broadcastSkillInvoke(objectName());
        room->broadcastInvoke("animate", "lightbox:$Hunzi:5000");
        room->getThread()->delay(5000);

        room->loseMaxHp(sunce);

        room->acquireSkill(sunce, "yinghun");
        room->acquireSkill(sunce, "yingzi");

        room->setPlayerMark(sunce, "hunzi", 1);

        return false;
    }
};

ZhibaCard::ZhibaCard(){
    mute = true;
    will_throw = false;
}

bool ZhibaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("sunce_zhiba") && to_select != Self
            && !to_select->isKongcheng() && !to_select->hasFlag("ZhibaInvoked");
}

void ZhibaCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *sunce = targets.first();
    room->setPlayerFlag(sunce, "ZhibaInvoked");
    if(sunce->getMark("hunzi") > 0 &&
       room->askForChoice(sunce, "zhiba_pindian", "accept+reject") == "reject")
    {
        room->broadcastSkillInvoke("sunce_zhiba", 4);
        return;
    }

    room->broadcastSkillInvoke("sunce_zhiba", 1);
    source->pindian(sunce, "zhiba", this);
    QList<ServerPlayer *> sunces;
    QList<ServerPlayer *> players = room->getOtherPlayers(source);
    ServerPlayer *lordplayer = NULL;
    if(source->isLord())
        lordplayer = source;
    foreach(ServerPlayer *p, players){
        if(p->hasLordSkill("sunce_zhiba") && !p->hasFlag("ZhibaInvoked")){
            sunces << p;
        }
        if(p->isLord())
            lordplayer = p;
    }
    if(sunces.empty())
        room->setPlayerFlag(source, "ForbidZhiba");
}

class ZhibaPindian: public OneCardViewAsSkill{
public:
    ZhibaPindian():OneCardViewAsSkill("zhiba_pindian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        const Player *lord = NULL;
        if(player->isLord())
            lord = player;
        QList<const Player *> players = player->getSiblings();
        foreach(const Player *p, players){
            if(p->isLord())
                lord = p;
        }
        return player->getKingdom() == "wu" && !player->isKongcheng() && !player->hasFlag("ForbidZhiba");
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
        events << GameStart << Pindian << PhaseChange;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == GameStart && player->hasLordSkill(objectName())){
            foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                if(!p->hasSkill("zhiba_pindian"))
                    room->attachSkillToPlayer(p, "zhiba_pindian");
            }
        }else if(event == Pindian){
            PindianStar pindian = data.value<PindianStar>();
            if(pindian->reason != "zhiba" || !pindian->to->hasLordSkill(objectName()))
                return false;
            if(!pindian->isSuccess()){
                room->broadcastSkillInvoke(objectName(), 2);
                pindian->to->obtainCard(pindian->from_card);
                pindian->to->obtainCard(pindian->to_card);
            }
            else
                room->broadcastSkillInvoke(objectName(), 3);
        }else if(event == PhaseChange && player->getPhase() == Player::NotActive){
            if(player->hasFlag("ForbidZhiba")){
                room->setPlayerFlag(player, "-ForbidZhiba");
            }
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            foreach(ServerPlayer *p, players){
                if(p->hasFlag("ZhibaInvoked")){
                    room->setPlayerFlag(p, "-ZhibaInvoked");
                }
            }
        }

        return false;
    }
};

TiaoxinCard::TiaoxinCard(){
    once = true;
    mute = true;
}

bool TiaoxinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->canSlash(Self);
}

void TiaoxinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    if(effect.from->hasArmorEffect("eight_diagram") || effect.from->hasSkill("bazhen"))
        room->broadcastSkillInvoke("tiaoxin", 3);
    else
        room->broadcastSkillInvoke("tiaoxin", qrand() % 2 + 1);

    if(!room->askForUseSlashTo(effect.to, effect.from, "@tiaoxin-slash:" + effect.from->objectName()) && !effect.to->isNude())
        room->throwCard(room->askForCardChosen(effect.from, effect.to, "he", "tiaoxin"), effect.to);
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
        return target != NULL && PhaseChangeSkill::triggerable(target)
                && target->getMark("zhiji") == 0
                && target->getPhase() == Player::Start
                && target->isKongcheng();
    }

    virtual bool onPhaseChange(ServerPlayer *jiangwei) const{
        Room *room = jiangwei->getRoom();

        LogMessage log;
        log.type = "#ZhijiWake";
        log.from = jiangwei;
        log.arg = objectName();
        room->sendLog(log);

        room->broadcastSkillInvoke("zhiji");
        room->broadcastInvoke("animate", "lightbox:$Zhiji:5000");
        room->getThread()->delay(5000);
        QStringList choicelist;
        choicelist << "draw";
        if (jiangwei->getLostHp() != 0)
            choicelist << "recover";
        QString choice;
        if (choicelist.length() >=2)
            choice = room->askForChoice(jiangwei, "objectName()", choicelist.join("+"));
        else
            choice = "draw";
        if(choice == "recover"){
            RecoverStruct recover;
            recover.who = jiangwei;
            room->recover(jiangwei, recover);
        }else
            room->drawCards(jiangwei, 2);

        room->setPlayerMark(jiangwei, "zhiji", 1);
        jiangwei->gainMark("@waked");
        room->acquireSkill(jiangwei, "guanxing");

        room->loseMaxHp(jiangwei);

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
    erzhang->getRoom()->moveCardTo(this, erzhang, effect.to, Player::PlaceEquip,
        CardMoveReason(CardMoveReason::S_REASON_USE, erzhang->objectName(), "zhijian", QString()));
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
        events << CardLostOneTime;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *erzhang = room->findPlayerBySkillName(objectName());
        ServerPlayer *current = room->getCurrent();

        if(erzhang == NULL)
            return false;
        if(erzhang == current)
            return false;
        if(current->getPhase() == Player::Discard){
            QVariantList guzheng = erzhang->tag["Guzheng"].toList();
            
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            foreach (int card_id, move->card_ids)
                guzheng << card_id;

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
        return target != NULL && !target->hasSkill("guzheng");
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
            if(room->getCardPlace(card_id) == Player::DiscardPile)
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
            
            CardsMoveStruct move;
            move.card_ids = cards;
            move.to = erzhang;
            move.to_place = Player::PlaceHand;
            QList<CardsMoveStruct> moves;
            moves.append(move);
            room->moveCards(moves, true, true);
        }

        return false;
    }
};

class Xiangle: public TriggerSkill{
public:
    Xiangle():TriggerSkill("xiangle"){
        events << SlashEffected << CardFinished << TargetConfirming;

        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *liushan, QVariant &data) const{

        if(event == TargetConfirming){

            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card && use.card->inherits("Slash")){

                room->broadcastSkillInvoke(objectName());

                LogMessage log;
                log.type = "#Xiangle";
                log.from = use.from;
                log.to << liushan;
                log.arg = objectName();
                room->sendLog(log);
                QVariant dataforai = QVariant::fromValue(liushan);
                if(!room->askForCard(use.from, ".Basic", "@xiangle-discard", dataforai, CardDiscarded))
                    liushan->addMark("xiangle");
            }
        }
        else if(event == CardFinished){
            liushan->setMark("xiangle", 0);
        }
        else {
            if(liushan->getMark("xiangle") > 0){
                liushan->loseMark("xiangle");
                return true;
            }
        }

        return false;
    }
};

class Fangquan: public TriggerSkill{
public:
    Fangquan():TriggerSkill("fangquan"){
        events << PhaseChange;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *liushan, QVariant &data) const{
        if(liushan->getPhase() == Player::Play){
            bool invoked = liushan->askForSkillInvoke(objectName());
            if(invoked)
                liushan->gainMark("fangquan");
            return invoked;
        }
        else if(liushan->getPhase() == Player::NotActive){
                foreach(ServerPlayer* p, room->getAlivePlayers()){
                    if(p->getMark("fangquan") > 0){
                        p->setMark("fangquan", 0);
                        p->gainAnExtraTurn(liushan);
                    }
                }
        }
        return false;
    }
};

class FangquanGive: public TriggerSkill{
public:
    FangquanGive():TriggerSkill("#fangquan-give"){
        events << PhaseChange;
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *liushan, QVariant &data) const{
        if(liushan->getPhase() == Player::Finish){
            if(liushan->getMark("fangquan") > 0){
                liushan->setMark("fangquan", 0);
                Room *room = liushan->getRoom();
                if(liushan->isKongcheng())
                    return false;
                if(room->askForDiscard(liushan, "fangquan", 1, 1, true)){
                    ServerPlayer *player = room->askForPlayerChosen(liushan, room->getOtherPlayers(liushan), "fangquan");
                    QString name = player->getGeneralName();
                    if(name == "zhugeliang" || name == "shenzhugeliang" || name == "wolong")
                        room->broadcastSkillInvoke("fangquan", 1);
                    else
                        room->broadcastSkillInvoke("fangquan", 2);

                    LogMessage log;
                    log.type = "#Fangquan";
                    log.from = liushan;
                    log.to << player;
                    room->sendLog(log);
                    player->gainMark("fangquan");
                }
             }
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
        return target != NULL && target->getPhase() == Player::Start
                && target->hasLordSkill("ruoyu")
                && target->isAlive()
                && target->getMark("ruoyu") == 0;
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
            room->broadcastSkillInvoke(objectName());

            LogMessage log;
            log.type = "#RuoyuWake";
            log.from = liushan;
            log.arg = QString::number(liushan->getHp());
            log.arg2 = objectName();
            room->sendLog(log);

            room->setPlayerMark(liushan, "ruoyu", 1);
            liushan->gainMark("@waked");
            room->setPlayerProperty(liushan, "maxhp", liushan->getMaxHp() + 1);

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

	static void playAudioEffect(ServerPlayer *zuoci, const QString &skill_name){
		zuoci->getRoom()->broadcastSkillInvoke(skill_name,  zuoci->getGender() == General::Male, -1);
    }

    static void AcquireGenerals(ServerPlayer *zuoci, int n){
        QStringList list = GetAvailableGenerals(zuoci);
        qShuffle(list);

        QStringList acquired = list.mid(0, n);
        QVariantList huashens = zuoci->tag["Huashens"].toList();
        foreach(QString huashen, acquired){
            huashens << huashen;
            const General *general = Sanguosha->getGeneral(huashen);
            foreach(const TriggerSkill *skill, general->getTriggerSkills()){
                zuoci->getRoom()->getThread()->addTriggerSkill(skill);
            }
        }

        zuoci->tag["Huashens"] = huashens;

        zuoci->invoke("animate", "huashen:" + acquired.join(":"));

        LogMessage log;
        log.type = "#GetHuashen";
        log.from = zuoci;
        log.arg = QString::number(n);
        log.arg2 = QString::number(huashens.length());
        zuoci->getRoom()->sendLog(log);
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

        static QSet<QString> banned;
        if(banned.isEmpty()){
            banned << "zuoci" << "zuocif" << "guzhielai" << "dengshizai" << "caochong"
                   << "jiangboyue" << "shenzhugeliang" << "shenlvbu" << "huaxiong" << "zhugejin";
        }

        return (all - banned - huashen_set - room_set).toList();
    }

    static QString SelectSkill(ServerPlayer *zuoci, bool acquire_instant = true){
        Room *room = zuoci->getRoom();
        playAudioEffect(zuoci, "huashen");

        QString huashen_skill = zuoci->tag["HuashenSkill"].toString();
        if(!huashen_skill.isEmpty()){
            room->detachSkillFromPlayer(zuoci, huashen_skill);
            zuoci->clearPrivatePiles();
            if(zuoci->getHp() <= 0 )
                room->enterDying(zuoci, NULL);
        }

        QVariantList huashens = zuoci->tag["Huashens"].toList();
        if(huashens.isEmpty())
            return QString();

        QStringList huashen_generals;
        foreach(QVariant huashen, huashens)
            huashen_generals << huashen.toString();

        QStringList skill_names;
        QString skill_name;
        AI* ai = zuoci->getAI();
        if(ai){
            QHash<QString, const General*>hash;
            foreach(QString general_name, huashen_generals){
                const General* general = Sanguosha->getGeneral(general_name);
                foreach(const Skill *skill, general->getVisibleSkillList()){
                    if(skill->isLordSkill() || skill->getFrequency() == Skill::Limited
                            || skill->getFrequency() == Skill::Wake)
                        continue;

                    if(!skill_names.contains(skill->objectName())){
                        hash[skill->objectName()] = general;
                        skill_names << skill->objectName();
                    }
                }
            }
            Q_ASSERT(skill_names.length() > 0);
            skill_name = ai->askForChoice("huashen", skill_names.join("+"), QVariant());
            const General* general = hash[skill_name];
            Q_ASSERT(general != NULL);
            QString kingdom = general->getKingdom();
            if(zuoci->getKingdom() != kingdom){
                if(kingdom == "god")
                    kingdom = room->askForKingdom(zuoci);
                room->setPlayerProperty(zuoci, "kingdom", kingdom);
            }
            if(zuoci->getGeneral()->isMale() != general->isMale())
                room->setPlayerProperty(zuoci, "general", general->isMale() ? "zuoci" : "zuocif");
        }
        else{
            QString general_name = room->askForGeneral(zuoci, huashen_generals);
            const General *general = Sanguosha->getGeneral(general_name);
            QString kingdom = general->getKingdom();
            if(zuoci->getKingdom() != kingdom){
                if(kingdom == "god")
                    kingdom = room->askForKingdom(zuoci);
                room->setPlayerProperty(zuoci, "kingdom", kingdom);
            }
            if(zuoci->getGeneral()->isMale() != general->isMale())
                room->setPlayerProperty(zuoci, "general", general->isMale() ? "zuoci" : "zuocif");

            foreach(const Skill *skill, general->getVisibleSkillList()){
                if(skill->isLordSkill() || skill->getFrequency() == Skill::Limited
                   || skill->getFrequency() == Skill::Wake)
                    continue;

                skill_names << skill->objectName();
            }

            if(skill_names.isEmpty())
                return QString();

            if(skill_names.length() == 1)
                skill_name = skill_names.first();
            else
                skill_name = room->askForChoice(zuoci, "huashen", skill_names.join("+"));
        }

        zuoci->tag["HuashenSkill"] = skill_name;

        if(acquire_instant)
            room->acquireSkill(zuoci, skill_name);

        return skill_name;
    }

    virtual void onGameStart(ServerPlayer *zuoci) const{
        if(zuoci->getGeneral2Name().startsWith("zuoci")){
            zuoci->getRoom()->setPlayerProperty(zuoci, "general2", zuoci->getGeneralName());
            zuoci->getRoom()->setPlayerProperty(zuoci, "general", "zuoci");
        }

        AcquireGenerals(zuoci, 2);
        SelectSkill(zuoci);
    }

    virtual QDialog *getDialog() const{
        static HuashenDialog *dialog;

        if(dialog == NULL)
            dialog = new HuashenDialog;

        return dialog;
    }
};

HuashenDialog::HuashenDialog()
{
    setWindowTitle(Sanguosha->translate("huashen"));
}

void HuashenDialog::popup(){
    QVariantList huashen_list = Self->tag["Huashens"].toList();
    QList<const General *> huashens;
    foreach(QVariant huashen, huashen_list)
        huashens << Sanguosha->getGeneral(huashen.toString());

    fillGenerals(huashens);

    show();
}

class HuashenBegin: public TriggerSkill{
public:
    HuashenBegin():TriggerSkill("#huashen-begin"){
        events << TurnStart;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *zuoci = room->findPlayerBySkillName(objectName());
        if(!zuoci || player->objectName() != zuoci->objectName() || !zuoci->faceUp())
            return false;
        if(zuoci->askForSkillInvoke("huashen")){
            QString skill_name = Huashen::SelectSkill(zuoci, false);
            if(!skill_name.isEmpty())
                zuoci->getRoom()->acquireSkill(zuoci, skill_name);
        }
        return false;
    }
};

class HuashenEnd: public TriggerSkill{
public:
    HuashenEnd():TriggerSkill("#huashen-end"){
        events << PhaseChange;
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *zuoci, QVariant &data) const{
        PhaseChangeStruct phase_change = data.value<PhaseChangeStruct>();
        if(phase_change.from == Player::Finish && zuoci->askForSkillInvoke("huashen"))
            Huashen::SelectSkill(zuoci);
        return false;
    }
};

class Xinsheng: public MasochismSkill{
public:
    Xinsheng():MasochismSkill("xinsheng"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *zuoci, const DamageStruct &damage) const{
        int n = damage.damage;
        if(n == 0)
            return;

        if(zuoci->askForSkillInvoke(objectName())){
            Huashen::playAudioEffect(zuoci, objectName());
            Huashen::AcquireGenerals(zuoci, n);
        }
    }
};

MountainPackage::MountainPackage()
    :Package("mountain")
{
    General *zhanghe = new General(this, "zhanghe", "wei");
    zhanghe->addSkill(new Qiaobian);

    General *dengai = new General(this, "dengai", "wei", 4);
    dengai->addSkill(new Tuntian);
    dengai->addSkill(new TuntianGet);
    dengai->addSkill(new Zaoxian);
    dengai->addRelateSkill("jixi");
    related_skills.insertMulti("tuntian", "#tuntian-get");

    General *jiangwei = new General(this, "jiangwei", "shu");
    jiangwei->addSkill(new Tiaoxin);
    jiangwei->addSkill(new Zhiji);
    related_skills.insertMulti("zhiji", "guanxing");

    General *liushan = new General(this, "liushan$", "shu", 3);
    liushan->addSkill(new Xiangle);
    liushan->addSkill(new Fangquan);
    liushan->addSkill(new FangquanGive);
    liushan->addSkill(new Ruoyu);
    related_skills.insertMulti("fangquan", "#fangquan-give");

    General *sunce = new General(this, "sunce$", "wu");
    sunce->addSkill(new Jiang);
    sunce->addSkill(new Hunzi);
    sunce->addSkill(new SunceZhiba);
    related_skills.insertMulti("hunzi", "yinghun");

    General *erzhang = new General(this, "erzhang", "wu", 3);
    erzhang->addSkill(new Zhijian);
    erzhang->addSkill(new Guzheng);
    erzhang->addSkill(new GuzhengGet);
    related_skills.insertMulti("guzheng", "#guzheng-get");

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

    General *caiwenji = new General(this, "caiwenji", "qun", 3, false);
    caiwenji->addSkill(new Beige);
    caiwenji->addSkill(new Duanchang);
    caiwenji->addSkill(new SPConvertSkill("guixiang", "caiwenji", "sp_caiwenji"));

    addMetaObject<QiaobianCard>();
    addMetaObject<TiaoxinCard>();
    addMetaObject<ZhijianCard>();
    addMetaObject<ZhibaCard>();
    addMetaObject<JixiCard>();

    skills << new ZhibaPindian << new Jixi;
}

ADD_PACKAGE(Mountain)
