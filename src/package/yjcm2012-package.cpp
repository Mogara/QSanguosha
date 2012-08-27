#include "yjcm2012-package.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"
#include "god.h"
#include "maneuvering.h"

class Zhenlie: public TriggerSkill{
public:
    Zhenlie():TriggerSkill("zhenlie"){
        events << AskForRetrial;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        if(judge->who->objectName() != player->objectName())
            return false;

        if(player->askForSkillInvoke(objectName(), data)){
            int card_id = room->drawCard();
            room->broadcastSkillInvoke(objectName(), room->getCurrent() == player ? 2 : 1);
            room->getThread()->delay();
            const Card *card = Sanguosha->getCard(card_id);

            room->retrial(card, player, judge, objectName());
        }
        return false;
    }
};

class Miji: public PhaseChangeSkill{
public:
    Miji():PhaseChangeSkill("miji"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *wangyi) const{
        if(!wangyi->isWounded())
            return false;
        if(wangyi->getPhase() == Player::Start || wangyi->getPhase() == Player::Finish){
            if(!wangyi->askForSkillInvoke(objectName()))
                return false;
            Room *room = wangyi->getRoom();
            room->broadcastSkillInvoke(objectName(), 1);
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club|spade):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = wangyi;

            room->judge(judge);

            if(judge.isGood()){
                int x = wangyi->getLostHp();
                wangyi->drawCards(x); //It should be preview, not draw
                ServerPlayer *target = room->askForPlayerChosen(wangyi, room->getAllPlayers(), objectName());

                if (target == wangyi)
                    room->broadcastSkillInvoke(objectName(), 2);
                else if (target->getGeneralName().contains("machao"))
                    room->broadcastSkillInvoke(objectName(), 4);
                else
                    room->broadcastSkillInvoke(objectName(), 3);

                QList<const Card *> miji_cards = wangyi->getHandcards().mid(wangyi->getHandcardNum() - x);
                foreach(const Card *card, miji_cards){
                    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, wangyi->objectName());
                    reason.m_playerId == target->objectName();
                    room->obtainCard(target, card, reason, false);
                }
            }
        }
        return false;
    }
};

QiceCard::QiceCard(){
    will_throw = false;
}

bool QiceCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    CardStar card = Self->tag.value("qice").value<CardStar>();
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card);
}

bool QiceCard::targetFixed() const{
    /*if (Sanguosha->currentRoomState()->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_RESPONSE)
        return true;*/

    CardStar card = Self->tag.value("qice").value<CardStar>();
    return card && card->targetFixed();
}

bool QiceCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    CardStar card = Self->tag.value("qice").value<CardStar>();
    return card && card->targetsFeasible(targets, Self);
}

Card::Suit QiceCard::getSuit(QList<int> cardid_list) const{
    QSet<QString> suit_set;
    QSet<Card::Color> color_set;
    foreach(int id, cardid_list){
        const Card *cd = Sanguosha->getCard(id);
        suit_set << cd->getSuitString();
        color_set << cd->getColor();
    }
    if(color_set.size() == 2)
        return Card::NoSuit;
    else{
        if(suit_set.size() == 1)
            return Sanguosha->getCard(cardid_list.first())->getSuit();
        else{
            if(Sanguosha->getCard(cardid_list.first())->isBlack())
                return Card::Spade;
            else
                return Card::Heart;
        }
    }
}

int QiceCard::getNumber(QList<int> cardid_list) const{
    if(cardid_list.length() == 1)
        return Sanguosha->getCard(cardid_list.first())->getNumber();
    else
        return 0;
}

const Card *QiceCard::validate(const CardUseStruct *card_use) const{
    card_use->from->setFlags("QiceUsed");
    Card *use_card = Sanguosha->cloneCard(user_string, getSuit(this->getSubcards()), getNumber(this->getSubcards()));
    use_card->setSkillName("qice");
    foreach(int id, this->getSubcards())
        use_card->addSubcard(id);
    use_card->deleteLater();
    return use_card;
}

const Card *QiceCard::validateInResposing(ServerPlayer *xunyou, bool &continuable) const{
    continuable = true;
    Room *room = xunyou->getRoom();
    room->broadcastSkillInvoke("qice");
    xunyou->setFlags("QiceUsed");

    Card *use_card = Sanguosha->cloneCard(user_string, getSuit(this->getSubcards()), getNumber(this->getSubcards()));
    use_card->setSkillName("qice");
    foreach(int id, this->getSubcards())
        use_card->addSubcard(id);
    use_card->deleteLater();
    return use_card;
}

class Qice: public ViewAsSkill{
public:
    Qice():ViewAsSkill("qice"){
    }

    virtual QDialog *getDialog() const{
        return GuhuoDialog::getInstance("qice", false);
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() < Self->getHandcardNum())
            return NULL;

        /*if (Sanguosha->currentRoomState()->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_RESPONSE){
            QiceCard *card = new QiceCard;
            card->setUserString("nullification");
            card->addSubcards(cards);
            return card;
        }*/

        CardStar c = Self->tag.value("qice").value<CardStar>();
        if(c){
            QiceCard *card = new QiceCard;
            card->setUserString(c->objectName());
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->isKongcheng())
            return false;
        else
            return !player->hasUsed("QiceCard");
    }

};

class Zhiyu: public MasochismSkill{
public:
    Zhiyu():MasochismSkill("zhiyu"){

    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{
        if(damage.from && damage.from->isAlive()
            && target->askForSkillInvoke(objectName(), QVariant::fromValue(damage))){
            target->drawCards(1);
            if (target->isKongcheng())
                return;

            Room *room = target->getRoom();
            room->broadcastSkillInvoke(objectName());
            room->showAllCards(target);

            QList<const Card *> cards = target->getHandcards();
            Card::Color color = cards.first()->getColor();
            bool same_color = true;
            foreach(const Card *card, cards){
                if(card->getColor() != color){
                    same_color = false;
                    break;
                }
            }

            if(same_color && !damage.from->isKongcheng())
                room->askForDiscard(damage.from, objectName(), 1, 1);
        }
    }
};

class Jiangchi:public DrawCardsSkill{
public:
    Jiangchi():DrawCardsSkill("jiangchi"){
    }

    virtual int getDrawNum(ServerPlayer *caozhang, int n) const{
        Room *room = caozhang->getRoom();
        QString choice = room->askForChoice(caozhang, objectName(), "jiang+chi+cancel");
        if(choice == "cancel")
            return n;
        LogMessage log;
        log.from = caozhang;
        log.arg = objectName();
        if(choice == "jiang"){
            log.type = "#Jiangchi1";
            room->sendLog(log);
            room->broadcastSkillInvoke(objectName(), 1);
            room->setPlayerCardLock(caozhang, "Slash");
            return n + 1;
        }else{
            log.type = "#Jiangchi2";
            room->sendLog(log);
            room->broadcastSkillInvoke(objectName(), 2);
            room->setPlayerFlag(caozhang, "jiangchi_invoke");
            return n - 1;
        }
    }
};

class JiangchiClear: public PhaseChangeSkill{
public:
    JiangchiClear():PhaseChangeSkill("#jiangchi-clear"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && PhaseChangeSkill::triggerable(target);
    }

    virtual bool onPhaseChange(ServerPlayer *caozhang) const{
        if(caozhang->getPhase() == Player::NotActive && caozhang->hasCardLock("Slash"))
            caozhang->getRoom()->setPlayerCardLock(caozhang, "-Slash");
        return false;
    }
};

class Qianxi: public TriggerSkill{
public:
    Qianxi():TriggerSkill("qianxi"){
        events << DamageCaused;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(player->distanceTo(damage.to) == 1 && damage.card && damage.card->isKindOf("Slash") &&
           !damage.chain && !damage.transfer && player->askForSkillInvoke(objectName(), data)){
            room->broadcastSkillInvoke(objectName(), 1);
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart):(.*)");
            judge.good = false;
            judge.who = player;
            judge.reason = objectName();

            room->judge(judge);
            if(judge.isGood()){
                room->broadcastSkillInvoke(objectName(), 2);
                LogMessage log;
                log.type = "#Qianxi";
                log.from = player;
                log.arg = objectName();
                log.to << damage.to;
                room->sendLog(log);
                room->loseMaxHp(damage.to);
                return true;
            }
            else
                room->broadcastSkillInvoke(objectName(), 3);
        }
        return false;
    }
};

class Dangxian: public TriggerSkill{
public:
    Dangxian():TriggerSkill("dangxian"){
        frequency = Compulsory;
        events << EventPhaseChanging;
    }


    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *liaohua, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if(change.to == Player::Start){
            room->broadcastSkillInvoke(objectName());
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = liaohua;
            log.arg = objectName();
            room->sendLog(log);

            liaohua->insertPhase(Player::Play);
            return true;
        }
        return false;
    }
};

class Fuli: public TriggerSkill{
public:
    Fuli():TriggerSkill("fuli"){
        events << Dying;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && TriggerSkill::triggerable(target) && target->getMark("@laoji") > 0;
    }

    int getKingdoms(Room *room) const{
        QSet<QString> kingdom_set;
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            kingdom_set << p->getKingdom();
        }
        return kingdom_set.size();
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *liaohua, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();
        if(dying_data.who != liaohua)
            return false;
        if(liaohua->askForSkillInvoke(objectName(), data)){
            room->broadcastInvoke("animate", "lightbox:$fuli");
            room->broadcastSkillInvoke(objectName());

            liaohua->loseMark("@laoji");
            int x = getKingdoms(room);
            RecoverStruct rev;
            rev.recover = x - liaohua->getHp();
            room->recover(liaohua, rev);
            liaohua->turnOver();
        }
        return false;
    }
};

class Fuhun: public PhaseChangeSkill{
public:
    Fuhun():PhaseChangeSkill("fuhun"){

    }

    virtual bool onPhaseChange(ServerPlayer *shuangying) const{
        switch(shuangying->getPhase()){
        case Player::Draw:{
            if(shuangying->askForSkillInvoke(objectName())){
                Room* room = shuangying->getRoom();
                int card1 = room->drawCard();
                int card2 = room->drawCard();
                CardsMoveStruct move, move2;
                move.card_ids.append(card1);
                move.card_ids.append(card2);
                move.reason = CardMoveReason(CardMoveReason::S_REASON_TURNOVER, shuangying->getGeneralName(), "fuhun", QString());
                move.to_place = Player::PlaceTable;
                room->moveCardsAtomic(move, true);
                room->getThread()->delay();
                move2 = move;
                move2.to_place = Player::PlaceHand;
                move2.to = shuangying;
                move2.reason.m_reason = CardMoveReason::S_REASON_DRAW;
                room->moveCardsAtomic(move2, true);

                if (Sanguosha->getCard(card1)->getColor() != Sanguosha->getCard(card2)->getColor()){
                    room->broadcastSkillInvoke(objectName(), 1);
                    room->setEmotion(shuangying, "good");
                    room->acquireSkill(shuangying, "wusheng");
                    room->acquireSkill(shuangying, "paoxiao");

                    shuangying->setFlags(objectName());
                }else{
                    room->broadcastSkillInvoke(objectName(), 2);
                    room->setEmotion(shuangying, "bad");
                }

                return true;
            }

            break;
        }

        case Player::NotActive:{
            if(shuangying->hasFlag(objectName())){
                Room *room = shuangying->getRoom();
                room->detachSkillFromPlayer(shuangying, "wusheng");
                room->detachSkillFromPlayer(shuangying, "paoxiao");
            }

            break;

        }

        default:
            break;
        }

        return false;
    }
};

class Zishou:public DrawCardsSkill{
public:
    Zishou():DrawCardsSkill("zishou"){
    }

    virtual int getDrawNum(ServerPlayer *liubiao, int n) const{
        Room *room = liubiao->getRoom();
        if(liubiao->isWounded() && room->askForSkillInvoke(liubiao, objectName())){
            room->broadcastSkillInvoke(objectName());
            liubiao->clearHistory();
            liubiao->skip(Player::Play);
            return n + liubiao->getLostHp();
        }else
            return n;
    }
};

class Zongshi: public MaxCardsSkill{
public:
    Zongshi():MaxCardsSkill("zongshi"){
    }
    virtual int getExtra(const Player *target) const{
        int extra = 0;
        QSet<QString> kingdom_set;
        if(target->parent()){
            foreach(const Player *player, target->parent()->findChildren<const Player *>()){
                if(player->isAlive()){
                    kingdom_set << player->getKingdom();
                }
            }
        }
        extra = kingdom_set.size();
        if(target->hasSkill(objectName()))
            return extra;
        else
            return 0;
    }
};

class Shiyong: public TriggerSkill{
public:
    Shiyong():TriggerSkill("shiyong"){
        events << Damaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , Room* room, ServerPlayer *player, QVariant &data) const{
        if (player == NULL) return false;

        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->isKindOf("Slash") &&
                (damage.card->isRed() || damage.card->hasFlag("drank"))){

            room->broadcastSkillInvoke(objectName());
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);

            room->loseMaxHp(player);
        }
        return false;
    }
};

class Gongqi : public OneCardViewAsSkill{
public:
    Gongqi():OneCardViewAsSkill("gongqi"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "slash";
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->getTypeId() == Card::Equip;
    }

    const Card *viewAs(const Card *originalCard) const{
        WushenSlash *slash = new WushenSlash(originalCard->getSuit(), originalCard->getNumber());
        slash->addSubcard(originalCard);
        slash->setSkillName(objectName());
        return slash;
    }
};

class Jiefan : public TriggerSkill{
public:
    Jiefan():TriggerSkill("jiefan"){
        events << AskForPeaches << DamageCaused << CardFinished << CardUsed;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *handang, QVariant &data) const{
        if (handang == NULL) return false;

        ServerPlayer *current = room->getCurrent();
        if(!current || current->isDead())
            return false;
        if(triggerEvent == CardUsed)
        {
            if(!handang->hasFlag("jiefanUsed"))
                return false;

            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->isKindOf("Slash"))
            {
                room->broadcastSkillInvoke(objectName());
                room->setPlayerFlag(handang, "-jiefanUsed");
                room->setCardFlag(use.card, "jiefan-slash");
            }
        }else if(triggerEvent == AskForPeaches  && handang->getPhase() == Player::NotActive && handang->askForSkillInvoke(objectName(), data)){
            DyingStruct dying = data.value<DyingStruct>();

            forever{
                if(handang->hasFlag("jiefan_failed")){
                    room->setPlayerFlag(handang, "-jiefan_failed");
                    break;
                }

                if(dying.who->getHp() > 0 || handang->isNude() || current->isDead() || !handang->canSlash(current, false))
                    break;

                room->setPlayerFlag(handang, "jiefanUsed");
                room->setTag("JiefanTarget", data);
                if(!room->askForUseSlashTo(handang, current, "jiefan-slash:" + dying.who->objectName()))
                {
                    room->setPlayerFlag(handang, "-jiefanUsed");
                    room->removeTag("JiefanTarget");
                    break;
                }
            }
        }
        else if(triggerEvent == DamageCaused){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.card && damage.card->isKindOf("Slash") && damage.card->hasFlag("jiefan-slash")){

                DyingStruct dying = room->getTag("JiefanTarget").value<DyingStruct>();

                ServerPlayer *target = dying.who;
                if(target && target->getHp() > 0){
                    LogMessage log;
                    log.type = "#JiefanNull1";
                    log.from = dying.who;
                    room->sendLog(log);
                }
                else if(target && target->isDead()){
                    LogMessage log;
                    log.type = "#JiefanNull2";
                    log.from = dying.who;
                    log.to << handang;
                    room->sendLog(log);
                }
                else if(current->hasSkill("wansha") && current->isAlive()  && target->objectName() != handang->objectName()){
                    LogMessage log;
                    log.type = "#JiefanNull3";
                    log.from = current;
                    room->sendLog(log);
                }
                else
                {
                    Peach *peach = new Peach(damage.card->getSuit(), damage.card->getNumber());
                    peach->setSkillName(objectName());
                    CardUseStruct use;
                    use.card = peach;
                    use.from = handang;
                    use.to << target;

                    room->setCardFlag(damage.card, "jiefan_success");
                    room->useCard(use);
                }
                return true;
            }
            return false;
        }
        else if(triggerEvent == CardFinished && !room->getTag("JiefanTarget").isNull()){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->hasFlag("jiefan-slash")){
                if(!use.card->hasFlag("jiefan_success"))
                    room->setPlayerFlag(handang, "jiefan_failed");
                room->removeTag("JiefanTarget");
            }
        }

        return false;
    }
};

AnxuCard::AnxuCard(){
    once = true;
}

bool AnxuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;
    if(targets.isEmpty())
        return true;
    else if(targets.length() == 1)
        return to_select->getHandcardNum() != targets.first()->getHandcardNum();
    else
        return false;
}

bool AnxuCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void AnxuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> selecteds = targets;
    ServerPlayer *from = selecteds.first()->getHandcardNum() < selecteds.last()->getHandcardNum() ? selecteds.takeFirst() : selecteds.takeLast();
    ServerPlayer *to = selecteds.takeFirst();
    int id = room->askForCardChosen(from, to, "h", "anxu");
    const Card *cd = Sanguosha->getCard(id);
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName());
    room->obtainCard(from, cd, reason);
    if(cd->getSuit() != Card::Spade){
        source->drawCards(1);
    }
}

class Anxu: public ZeroCardViewAsSkill{
public:
    Anxu():ZeroCardViewAsSkill("anxu"){
    }

    virtual const Card *viewAs() const{
        return new AnxuCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("AnxuCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }
};

class Zhuiyi: public TriggerSkill{
public:
    Zhuiyi():TriggerSkill("zhuiyi"){
        events << Death ;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        QList<ServerPlayer *> targets = (damage && damage->from) ?
                            room->getOtherPlayers(damage->from) : room->getAlivePlayers();

        if(targets.isEmpty() || !player->askForSkillInvoke(objectName(), data))
            return false;

        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());

        room->broadcastSkillInvoke(objectName());
        target->drawCards(3);
        RecoverStruct recover;
        recover.who = target;
        recover.recover = 1;
        room->recover(target, recover, true);
        return false;
    }
};

class LihuoViewAsSkill:public OneCardViewAsSkill{
public:
    LihuoViewAsSkill():OneCardViewAsSkill("lihuo"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "slash";
    }

    virtual bool viewFilter(const Card* to_select) const{
        return to_select->objectName() == "slash";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        
        Card *acard = new FireSlash(originalCard->getSuit(), originalCard->getNumber());
        acard->addSubcard(originalCard->getId());
        acard->setSkillName(objectName());
        return acard;
    }
};

class Lihuo: public TriggerSkill{
public:
    Lihuo():TriggerSkill("lihuo"){
        events << DamageDone << CardFinished;
        view_as_skill = new LihuoViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(triggerEvent == DamageDone){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.card && damage.card->isKindOf("Slash") && damage.card->getSkillName() == objectName())
                damage.from->tag["Invokelihuo"] = true;
        }
        else if(TriggerSkill::triggerable(player) && player->tag.value("Invokelihuo", false).toBool()){
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);

            player->tag["Invokelihuo"] = false;
            room->loseHp(player, 1);
        }
        return false;
    }
};

ChunlaoCard::ChunlaoCard(){
    will_throw = false;
    target_fixed = true;
}

void ChunlaoCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const{
    foreach(int id, this->subcards){
        source->addToPile("wine", id, true);
    }
}

class ChunlaoViewAsSkill:public ViewAsSkill{
public:
    ChunlaoViewAsSkill():ViewAsSkill("chunlao"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@chunlao";
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card* to_select) const{
        return to_select->isKindOf("Slash");
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if(cards.length() == 0)
            return NULL;

        Card *acard = new ChunlaoCard;
        acard->addSubcards(cards);
        acard->setSkillName(objectName());
        return acard;
    }
};

class Chunlao: public TriggerSkill{
public:
    Chunlao():TriggerSkill("chunlao"){
        events << EventPhaseStart << AskForPeaches ;
        view_as_skill = new ChunlaoViewAsSkill;
    }


    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *chengpu, QVariant &data) const{

        if(triggerEvent == EventPhaseStart &&
                chengpu->getPhase() == Player::Finish &&
                !chengpu->isKongcheng() &&
                chengpu->getPile("wine").isEmpty()){
            room->askForUseCard(chengpu, "@@chunlao", "@chunlao");
        }else if(triggerEvent == AskForPeaches && !chengpu->getPile("wine").isEmpty()){
            DyingStruct dying = data.value<DyingStruct>();
            while(dying.who->getHp() < 1 && chengpu->askForSkillInvoke(objectName(), data)){
                QList<int> cards = chengpu->getPile("wine");
                room->fillAG(cards, chengpu);
                int card_id = room->askForAG(chengpu, cards, true, objectName());
                room->broadcastInvoke("clearAG");
                if(card_id != -1){
                    CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), "chunlao", QString());
                    room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
                    Analeptic *analeptic = new Analeptic(Card::NoSuit, 0);
                    analeptic->setSkillName(objectName());
                    CardUseStruct use;
                    use.card = analeptic;
                    use.from = dying.who;
                    use.to << dying.who;
                    room->useCard(use);
                }
            }
        }
        return false;
    }
};


YJCM2012Package::YJCM2012Package():Package("YJCM2012"){

    General *bulianshi = new General(this, "bulianshi", "wu", 3, false);
    bulianshi->addSkill(new Anxu);
    bulianshi->addSkill(new Zhuiyi);

    General *caozhang = new General(this, "caozhang", "wei");
    caozhang->addSkill(new Jiangchi);
    caozhang->addSkill(new JiangchiClear);
    related_skills.insertMulti("jiangchi", "#jiangchi-clear");

    General *chengpu = new General(this, "chengpu", "wu");
    chengpu->addSkill(new Lihuo);
    chengpu->addSkill(new Chunlao);

    General *guanxingzhangbao = new General(this, "guanxingzhangbao", "shu");
    guanxingzhangbao->addSkill(new Fuhun);

    General *handang = new General(this, "handang", "wu");
    handang->addSkill(new Gongqi);
    handang->addSkill(new Jiefan);

    General *huaxiong = new General(this, "huaxiong", "qun", 6);
    huaxiong->addSkill(new Shiyong);

    General *liaohua = new General(this, "liaohua", "shu");
    liaohua->addSkill(new Dangxian);
    liaohua->addSkill(new MarkAssignSkill("@laoji", 1));
    liaohua->addSkill(new Fuli);

    General *liubiao = new General(this, "liubiao", "qun", 4);
    liubiao->addSkill(new Zishou);
    liubiao->addSkill(new Zongshi);

    General *madai = new General(this, "madai", "shu");
    madai->addSkill(new Qianxi);
    madai->addSkill("mashu");

    General *wangyi = new General(this, "wangyi", "wei", 3, false);
    wangyi->addSkill(new Zhenlie);
    wangyi->addSkill(new Miji);

    General *xunyou = new General(this, "xunyou", "wei", 3);
    xunyou->addSkill(new Qice);
    xunyou->addSkill(new Zhiyu);

    addMetaObject<QiceCard>();
    addMetaObject<ChunlaoCard>();
    addMetaObject<AnxuCard>();
}

ADD_PACKAGE(YJCM2012)
