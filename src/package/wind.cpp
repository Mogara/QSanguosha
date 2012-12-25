#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "engine.h"
#include "ai.h"
#include "general.h"

// skill cards

GuidaoCard::GuidaoCard(){
    target_fixed = true;
    will_throw = false;
    can_jilei = true;
}

void GuidaoCard::use(Room *room, ServerPlayer *zhangjiao, QList<ServerPlayer *> &targets) const{

}

LeijiCard::LeijiCard(){

}

bool LeijiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void LeijiCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhangjiao = effect.from;
    ServerPlayer *target = effect.to;

    Room *room = zhangjiao->getRoom();

    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(spade):(.*)");
    judge.good = false;
    judge.reason = "leiji";
    judge.who = target;
    judge.play_animation = true;
    judge.negative = true;

    room->judge(judge);

    if(judge.isBad()){
        DamageStruct damage;
        damage.card = NULL;
        damage.damage = 2;
        damage.from = zhangjiao;
        damage.to = target;
        damage.nature = DamageStruct::Thunder;

        room->damage(damage);
    }else
        room->setEmotion(zhangjiao, "bad");
}

HuangtianCard::HuangtianCard(){
    will_throw = false;
    m_skillName = "huangtianv";
    mute = true;
}

void HuangtianCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *zhangjiao = targets.first();
    if(zhangjiao->hasLordSkill("huangtian")){
        room->setPlayerFlag(zhangjiao, "HuangtianInvoked");
        room->broadcastSkillInvoke("huangtian");
        zhangjiao->obtainCard(this);
        QList<int> subcards = this->getSubcards();
        foreach(int card_id, subcards)
            room->setCardFlag(card_id,"visible");
        room->setEmotion(zhangjiao, "good");
        QList<ServerPlayer *> zhangjiaos;
        QList<ServerPlayer *> players = room->getOtherPlayers(source);
        foreach(ServerPlayer *p, players){
            if(p->hasLordSkill("huangtian") && !p->hasFlag("HuangtianInvoked")){
                zhangjiaos << p;
            }
        }
        if(zhangjiaos.empty())
            room->setPlayerFlag(source, "ForbidHuangtian");
    }
}

bool HuangtianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("huangtian")
            && to_select != Self && !to_select->hasFlag("HuangtianInvoked");
}

class GuidaoViewAsSkill:public OneCardViewAsSkill{
public:
    GuidaoViewAsSkill():OneCardViewAsSkill(""){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@guidao";
    }

    virtual bool viewFilter(const Card* to_select) const{
        return to_select->isBlack();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        GuidaoCard *guidaoCard = new GuidaoCard;
        guidaoCard->setSuit(originalCard->getSuit());
        guidaoCard->addSubcard(originalCard);

        return guidaoCard;
    }
};

class Guidao: public TriggerSkill{
public:
    Guidao():TriggerSkill("guidao"){
        view_as_skill = new GuidaoViewAsSkill;

        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        
        if(target == NULL || !TriggerSkill::triggerable(target))
            return false;

        if(target->isKongcheng()){
            bool has_black = false;
            for(int i = 0; i < 4; i++){
                const EquipCard *equip = target->getEquip(i);
                if(equip && equip->isBlack()){
                    has_black = true;
                    break;
                }
            }

            return has_black;
        }else
            return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if (player == NULL) return false;
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@guidao-card" << judge->who->objectName()
                << objectName() << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");
        const Card *card = room->askForCard(player, "@guidao", prompt, data, AskForRetrial);

        if (card != NULL){
            room->broadcastSkillInvoke(objectName());
            room->retrial(card, player, judge, objectName(), true);
        }
        return false;
    }
};

class HuangtianViewAsSkill: public OneCardViewAsSkill{
public:
    HuangtianViewAsSkill():OneCardViewAsSkill("huangtianv"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getKingdom() == "qun" && !player->hasFlag("ForbidHuangtian");
    }

    virtual bool viewFilter(const Card* to_select) const{
        const Card *card = to_select;
        return card->objectName() == "jink" || card->objectName() == "lightning";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        HuangtianCard *card = new HuangtianCard;
        card->addSubcard(originalCard);

        return card;
    }
};

class Huangtian: public TriggerSkill{
public:
    Huangtian():TriggerSkill("huangtian$"){
        events << GameStart << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(triggerEvent == GameStart && player->hasLordSkill(objectName())){
            foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                if(!p->hasSkill("huangtianv"))
                    room->attachSkillToPlayer(p, "huangtianv");
            }
        }
        else if(triggerEvent == EventPhaseChanging){
            PhaseChangeStruct phase_change = data.value<PhaseChangeStruct>();
            if (phase_change.from != Player::Play)
                  return false;
            if(player->hasFlag("ForbidHuangtian")){
                room->setPlayerFlag(player, "-ForbidHuangtian");
            }
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            foreach(ServerPlayer *p, players){
                if(p->hasFlag("HuangtianInvoked")){
                    room->setPlayerFlag(p, "-HuangtianInvoked");
                }
            }
        }
        return false;
    }
};

class LeijiViewAsSkill: public ZeroCardViewAsSkill{
public:
    LeijiViewAsSkill():ZeroCardViewAsSkill("leiji"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@leiji";
    }

    virtual const Card *viewAs() const{
        return new LeijiCard;
    }
};

class Leiji: public TriggerSkill{
public:
    Leiji():TriggerSkill("leiji"){
        events << CardResponsed;
        view_as_skill = new LeijiViewAsSkill;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *zhangjiao, QVariant &data) const{
        if (zhangjiao == NULL) return false;
        CardStar card_star = data.value<ResponsedStruct>().m_card;
        if(!card_star->isKindOf("Jink"))
            return false;
        room->askForUseCard(zhangjiao, "@@leiji", "@leiji");

        return false;
    }
};

ShensuCard::ShensuCard(){
    mute = true;
}

bool ShensuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void ShensuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("shensu");
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to = targets;

    room->useCard(use);
}

class ShensuViewAsSkill: public ViewAsSkill{
public:
    ShensuViewAsSkill():ViewAsSkill("shensu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (Sanguosha->currentRoomState()->getCurrentCardUsePattern().endsWith("1"))
            return false;
        else
            return selected.isEmpty() && to_select->isKindOf("EquipCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern.startsWith("@@shensu");
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if(Sanguosha->currentRoomState()->getCurrentCardUsePattern().endsWith("1")){
            if(cards.isEmpty())
                return new ShensuCard;
            else
                return NULL;
        }else{
            if(cards.length() != 1)
                return NULL;

            ShensuCard *card = new ShensuCard;
            card->addSubcards(cards);

            return card;
        }
    }
};

class Shensu: public TriggerSkill{
public:
    Shensu():TriggerSkill("shensu"){
        events << EventPhaseChanging;
        view_as_skill = new ShensuViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *xiahouyuan, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if(change.to == Player::Judge && !xiahouyuan->isSkipped(Player::Judge)
                && !xiahouyuan->isSkipped(Player::Draw)){
            if(room->askForUseCard(xiahouyuan, "@@shensu1", "@shensu1", 1)){
                xiahouyuan->skip(Player::Judge);
                xiahouyuan->skip(Player::Draw);
            }
        }else if(change.to == Player::Play && !xiahouyuan->isSkipped(Player::Play)){
            if(room->askForUseCard(xiahouyuan, "@@shensu2", "@shensu2", 2)){
                xiahouyuan->skip(Player::Play);
            }
        }
        return false;
    }
};

class Jushou: public PhaseChangeSkill{
public:
    Jushou():PhaseChangeSkill("jushou"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            if(room->askForSkillInvoke(target, objectName())){
                target->drawCards(3);
                target->turnOver();

                room->broadcastSkillInvoke(objectName());
            }
        }

        return false;
    }
};

class Liegong:public TriggerSkill{
public:
    Liegong():TriggerSkill("liegong"){
        events << TargetConfirmed << SlashProceed << CardFinished;
    }

    virtual bool trigger(TriggerEvent triggerEvent , Room* room, ServerPlayer *player, QVariant &data) const{
        if(triggerEvent == TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(!use.card->isKindOf("Slash") || use.from != player || player->getPhase() != Player::Play)
                   return false;

            foreach(ServerPlayer *target, use.to){
                int handcardnum = target->getHandcardNum();
                if(handcardnum >= player->getHp() || handcardnum <= player->getAttackRange()){
                    bool invoke = player->askForSkillInvoke("liegong", QVariant::fromValue(target));
                    QVariantList liegongList = target->tag["Liegong"].toList();
                    liegongList << invoke;
                    target->tag["Liegong"] = liegongList;
                    target->setFlags("LiegongTarget");
                }
            }
        }
        else if(triggerEvent == SlashProceed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            effect.to->setFlags("-LiegongTarget");
            QVariantList liegongList = effect.to->tag["Liegong"].toList();
            if(!liegongList.isEmpty()){
                bool invoke = liegongList.takeFirst().toBool();
                effect.to->tag["Liegong"] = liegongList;
                if(invoke){
                    room->broadcastSkillInvoke(objectName());
                    room->slashResult(effect, NULL);
                    return true;
                }
            }
        }
        else if(triggerEvent == CardFinished){
            CardUseStruct use = data.value<CardUseStruct>();
            foreach(ServerPlayer *target, use.to){
                if(target->hasFlag("LiegongTarget"))
                    target->tag.remove("Liegong");
            }
        }

        return false;
    }
};

class Kuanggu: public TriggerSkill{
public:
    Kuanggu():TriggerSkill("kuanggu"){
        frequency = Compulsory;
        events << Damage << PreHpReduced;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(triggerEvent == PreHpReduced && TriggerSkill::triggerable(damage.from)){
            damage.from->tag["InvokeKuanggu"] = damage.from->distanceTo(damage.to) <= 1;
        }else if(triggerEvent == Damage && TriggerSkill::triggerable(player)){
            bool invoke = player->tag.value("InvokeKuanggu", false).toBool();
            player->tag["InvokeKuanggu"] = false;
            if(invoke && player->isWounded()){
                room->broadcastSkillInvoke(objectName());

                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);

                RecoverStruct recover;
                recover.who = player;
                recover.recover = damage.damage;
                room->recover(player, recover);
            }
        }

        return false;
    }
};

class BuquRemove: public TriggerSkill{
public:
    BuquRemove():TriggerSkill("#buqu-remove"){
        events << HpRecover << EventLoseSkill;
    }

    virtual int getPriority() const{
        return -1;
    }

    static void Remove(ServerPlayer *zhoutai){
        Room *room = zhoutai->getRoom();
        QList<int> buqu(zhoutai->getPile("buqu"));

        CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), "buqu", QString());
        int need = 1 - zhoutai->getHp();
        if(need <= 0){
            // clear all the buqu cards
            foreach(int card_id, buqu) {

                LogMessage log;
                log.type = "$BuquRemove";
                log.from = zhoutai;
                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);

                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
            }
        }else{
            int to_remove = buqu.length() - need;

            for(int i = 0; i < to_remove; i++){
                room->fillAG(buqu);
                int card_id = room->askForAG(zhoutai, buqu, false, "buqu");

                LogMessage log;
                log.type = "$BuquRemove";
                log.from = zhoutai;
                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);

                buqu.removeOne(card_id);
                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
                room->broadcastInvoke("clearAG");
            }
        }
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *zhoutai, QVariant &data) const{
        if(triggerEvent == HpRecover && TriggerSkill::triggerable(zhoutai)
                && zhoutai->getPile("buqu").length() > 0)
            Remove(zhoutai);
        else if(triggerEvent == EventLoseSkill && data.toString() == "buqu"){
            zhoutai->removePileByName("buqu");
            if(zhoutai->getHp() <= 0)
                room->enterDying(zhoutai, NULL);
        }
        return false;
    }
};

class Buqu: public TriggerSkill{
public:
    Buqu():TriggerSkill("buqu"){
        events << PostHpReduced << AskForPeachesDone;
    }
    
    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *zhoutai, QVariant &) const{
        if(event == PostHpReduced && zhoutai->getHp() < 1){
            if(room->askForSkillInvoke(zhoutai, objectName())){
                room->setTag("Buqu", zhoutai->objectName());
                room->broadcastSkillInvoke(objectName());
                const QList<int> &buqu = zhoutai->getPile("buqu");

                int need = 1 - zhoutai->getHp(); // the buqu cards that should be turned over
                int n = need - buqu.length();
                if(n > 0){
                    QList<int> card_ids = room->getNCards(n, false);
                    zhoutai->addToPile("buqu", card_ids);
                }
                const QList<int> &buqunew = zhoutai->getPile("buqu");
                QList<int> duplicate_numbers;

                QSet<int> numbers;
                foreach(int card_id, buqunew){
                    const Card *card = Sanguosha->getCard(card_id);
                    int number = card->getNumber();

                    if(numbers.contains(number)){
                        duplicate_numbers << number;
                    }else
                        numbers << number;
                }

                if(duplicate_numbers.isEmpty()){
                    room->setTag("Buqu", QVariant());
                    return true;
                }
            }
        }else if(event == AskForPeachesDone){
            const QList<int> &buqu = zhoutai->getPile("buqu");

            if(zhoutai->getHp() > 0)
                return false;
            if(room->getTag("Buqu").toString() != zhoutai->objectName())
                return false;
            room->setTag("Buqu", QVariant());
            QList<int> duplicate_numbers;

            QSet<int> numbers;
            foreach(int card_id, buqu){
                const Card *card = Sanguosha->getCard(card_id);
                int number = card->getNumber();

                if(numbers.contains(number) && !duplicate_numbers.contains(number)){
                    duplicate_numbers << number;
                }else
                    numbers << number;
            }

            if(duplicate_numbers.isEmpty()){
                room->broadcastSkillInvoke(objectName());
                room->setPlayerFlag(zhoutai, "-dying");
                return true;
            }else{
                LogMessage log;
                log.type = "#BuquDuplicate";
                log.from = zhoutai;
                log.arg = QString::number(duplicate_numbers.length());
                room->sendLog(log);

                for(int i=0; i<duplicate_numbers.length(); i++){
                    int number = duplicate_numbers.at(i);

                    LogMessage log;
                    log.type = "#BuquDuplicateGroup";
                    log.from = zhoutai;
                    log.arg = QString::number(i+1);
                    log.arg2 = Card::Number2String(number);
                    room->sendLog(log);

                    foreach(int card_id, buqu){
                        const Card *card = Sanguosha->getCard(card_id);
                        if(card->getNumber() == number){
                            LogMessage log;
                            log.type = "$BuquDuplicateItem";
                            log.from = zhoutai;
                            log.card_str = QString::number(card_id);
                            room->sendLog(log);
                        }
                    }
                }
            }
        }
        return false;
    }
};


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCommandLinkButton>

class Hongyan: public FilterSkill{
public:
    Hongyan():FilterSkill("hongyan"){

    }

    static WrappedCard *changeToHeart(int cardId){
        WrappedCard *new_card = Sanguosha->getWrappedCard(cardId);
        new_card->setSkillName("hongyan");
        new_card->setSuit(Card::Heart);
        new_card->setModified(true);
        return new_card;
    }

    virtual bool viewFilter(const Card* to_select) const{
        return to_select->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        return changeToHeart(originalCard->getEffectiveId());
    }
};

TianxiangCard::TianxiangCard()
{
}

void TianxiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    room->setPlayerFlag(effect.to, "TianxiangTarget");
    DamageStruct damage = effect.from->tag.value("TianxiangDamage").value<DamageStruct>();
    damage.to = effect.to;
    damage.transfer = true;
    room->damage(damage);
}

class TianxiangViewAsSkill: public OneCardViewAsSkill{
public:
    TianxiangViewAsSkill():OneCardViewAsSkill("tianxiang"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@tianxiang";
    }

    virtual bool viewFilter(const Card* to_select) const{
        return !to_select->isEquipped() && to_select->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        TianxiangCard *tianxiangCard = new TianxiangCard;
        tianxiangCard->addSubcard(originalCard);
        return tianxiangCard;
    }
};

class Tianxiang: public TriggerSkill{
public:
    Tianxiang():TriggerSkill("tianxiang"){
        events << DamageInflicted << DamageComplete;

        view_as_skill = new TianxiangViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *xiaoqiao, QVariant &data) const{
        if(triggerEvent == DamageInflicted && TriggerSkill::triggerable(xiaoqiao) && !xiaoqiao->isKongcheng())
        {
            DamageStruct damage = data.value<DamageStruct>();

            xiaoqiao->tag["TianxiangDamage"] = QVariant::fromValue(damage);
            if(room->askForUseCard(xiaoqiao, "@@tianxiang", "@tianxiang-card")){
                return true;
            }
        }else if(triggerEvent == DamageComplete && xiaoqiao->hasFlag("TianxiangTarget") && xiaoqiao->isAlive()){
            room->setPlayerFlag(xiaoqiao, "-TianxiangTarget");
            xiaoqiao->drawCards(xiaoqiao->getLostHp(), false);
        }
        return false;
    }
};

GuhuoCard::GuhuoCard(){
    mute = true;
}

bool GuhuoCard::guhuo(ServerPlayer* yuji, const QString& message) const{
    Room *room = yuji->getRoom();
    room->setTag("Guhuoing", true);
    room->setTag("GuhuoType", this->user_string);

    QList<ServerPlayer *> players = room->getOtherPlayers(yuji);
    QSet<ServerPlayer *> questioned;

    QList<int> used_cards;
    QList<CardsMoveStruct> moves;
    foreach(int card_id, getSubcards()){
        used_cards << card_id;
    }

    foreach(ServerPlayer *player, players){
        if(player->getHp() <= 0){
            LogMessage log;
            log.type = "#GuhuoCannotQuestion";
            log.from = player;
            log.arg = QString::number(player->getHp());
            room->sendLog(log);

            room->setEmotion(player, "no-question");

            continue;
        }

        QString choice = room->askForChoice(player, "guhuo", "noquestion+question");
        if(choice == "question"){
            room->setEmotion(player, "question");
            questioned << player;
        }else
            room->setEmotion(player, "no-question");

        LogMessage log;
        log.type = "#GuhuoQuery";
        log.from = player;
        log.arg = choice;

        room->sendLog(log);
    }
    
    LogMessage log;
    log.type = "$GuhuoResult";
    log.from = yuji;
    log.card_str = QString::number(subcards.first());
    room->sendLog(log);

    bool success = false;
    if(questioned.isEmpty()){
        success = true;

        foreach(ServerPlayer *player, players)
            room->setEmotion(player, ".");
        if(yuji->getPhase() == Player::Play){
            CardMoveReason reason(CardMoveReason::S_REASON_USE, yuji->objectName(), QString(), "guhuo");
            CardsMoveStruct move(used_cards, yuji, NULL, Player::PlaceTable, reason);
            moves.append(move);
            room->moveCardsAtomic(moves, true);
        }
        else if(yuji->getPhase() == Player::NotActive)
            room->moveCardTo(this, yuji, NULL, Player::DiscardPile,
                CardMoveReason(CardMoveReason::S_REASON_RESPONSE, yuji->objectName(), QString(), "guhuo"), true);

    }else{
        const Card *card = Sanguosha->getCard(subcards.first());
        bool real;
        if(user_string == "peach+analeptic")
            real = card->objectName() == yuji->tag["GuhuoSaveSelf"].toString();
        else if (user_string == "slash")
            real = card->objectName().contains("slash");
        else if (user_string == "natural_slash")
            real = card->objectName() == "slash";
        else
            real = card->match(user_string);

        success = real && card->getSuit() == Card::Heart;
        if(success && yuji->getPhase() == Player::Play){
            CardMoveReason reason(CardMoveReason::S_REASON_USE, yuji->objectName(), QString(), "guhuo");
            CardsMoveStruct move(used_cards, yuji, NULL, Player::PlaceTable, reason);
            moves.append(move);
            room->moveCardsAtomic(moves, true);
        }
        else if(success && yuji->getPhase() == Player::NotActive){
            room->moveCardTo(this, yuji, NULL, Player::DiscardPile,
                CardMoveReason(CardMoveReason::S_REASON_RESPONSE, yuji->objectName(), QString(), "guhuo"), true);
        }
        else{
            room->moveCardTo(this, yuji, NULL, Player::DiscardPile,
                CardMoveReason(CardMoveReason::S_REASON_PUT, yuji->objectName(), QString(), "guhuo"), true);
        }
        foreach(ServerPlayer *player, players){
            room->setEmotion(player, ".");

            if(questioned.contains(player)){
                if(real)
                    room->loseHp(player);
                else
                    player->drawCards(1);
            }
        }
    }
    yuji->tag.remove("GuhuoSaveSelf");
    yuji->tag.remove("GuhuoSlash");
    room->setTag("Guhuoing", false);
    room->removeTag("GuhuoType");
    if(!success)
        room->setPlayerFlag(yuji, "guhuo_failed");

    return success;
}

GuhuoDialog *GuhuoDialog::getInstance(const QString &object, bool left, bool right){
    static GuhuoDialog *instance;
    if(instance == NULL || instance->objectName() != object)
        instance = new GuhuoDialog(object, left, right);

    return instance;
}

GuhuoDialog::GuhuoDialog(const QString &object, bool left, bool right):object_name(object)
{
    setObjectName(object);
    setWindowTitle(Sanguosha->translate(object));
    group = new QButtonGroup(this);

    QHBoxLayout *layout = new QHBoxLayout;
    if(left)
        layout->addWidget(createLeft());
    if(right)
        layout->addWidget(createRight());

    setLayout(layout);

    connect(group, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(selectCard(QAbstractButton*)));
}

void GuhuoDialog::popup(){
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_PLAY)
        return;

    foreach(QAbstractButton *button, group->buttons()){
        const Card *card = map[button->objectName()];
        button->setEnabled(card->isAvailable(Self));
    }

    Self->tag.remove(object_name);
    exec();
}

void GuhuoDialog::selectCard(QAbstractButton *button){
    CardStar card = map.value(button->objectName());
    Self->tag[object_name] = QVariant::fromValue(card);
    emit onButtonClick();
    accept();
}

QGroupBox *GuhuoDialog::createLeft(){
    QGroupBox *box = new QGroupBox;
    box->setTitle(Sanguosha->translate("basic"));

    QVBoxLayout *layout = new QVBoxLayout;

    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach(const Card *card, cards){
        if(card->getTypeId() == Card::Basic && !map.contains(card->objectName())){
            Card *c = Sanguosha->cloneCard(card->objectName(), Card::NoSuit, 0);
            c->setParent(this);

            layout->addWidget(createButton(c));
        }
    }

    layout->addStretch();

    box->setLayout(layout);
    return box;
}

QGroupBox *GuhuoDialog::createRight(){
    QGroupBox *box = new QGroupBox(Sanguosha->translate("ndtrick"));
    QHBoxLayout *layout = new QHBoxLayout;

    QGroupBox *box1 = new QGroupBox(Sanguosha->translate("single_target"));
    QVBoxLayout *layout1 = new QVBoxLayout;

    QGroupBox *box2 = new QGroupBox(Sanguosha->translate("multiple_targets"));
    QVBoxLayout *layout2 = new QVBoxLayout;


    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach(const Card *card, cards){
        if(card->isNDTrick() && !map.contains(card->objectName())){
            Card *c = Sanguosha->cloneCard(card->objectName(), Card::NoSuit, 0);
            c->setSkillName(object_name);
            c->setParent(this);

            QVBoxLayout *layout = c->isKindOf("SingleTargetTrick") ? layout1 : layout2;
            layout->addWidget(createButton(c));
        }
    }

    box->setLayout(layout);
    box1->setLayout(layout1);
    box2->setLayout(layout2);

    layout1->addStretch();
    layout2->addStretch();

    layout->addWidget(box1);
    layout->addWidget(box2);
    return box;
}

QAbstractButton *GuhuoDialog::createButton(const Card *card){
    QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate(card->objectName()));
    button->setObjectName(card->objectName());
    button->setToolTip(card->getDescription());

    map.insert(card->objectName(), card);
    group->addButton(button);

    return button;
}

bool GuhuoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    CardStar card = Self->tag.value("guhuo").value<CardStar>();
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card);
}

bool GuhuoCard::targetFixed() const{
    if(Sanguosha->currentRoomState()->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_RESPONSE)
    {
        if(!ClientInstance->hasNoTargetResponsing()){
            CardStar card = Sanguosha->cloneCard(user_string, NoSuit, 0);
            Self->tag["guhuo"] = QVariant::fromValue(card);
            return card && card->targetFixed();
        }
        else return true;
    }

    CardStar card = Self->tag.value("guhuo").value<CardStar>();
    return card && card->targetFixed();
}

bool GuhuoCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    CardStar card = Self->tag.value("guhuo").value<CardStar>();
    return card && card->targetsFeasible(targets, Self);
}

const Card *GuhuoCard::validate(const CardUseStruct *card_use) const{
    Room *room = card_use->from->getRoom();

	QString to_guhuo = user_string;
    if (user_string == "slash"
        && Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        QStringList guhuo_list;
        guhuo_list << "slash";
        if (!Sanguosha->getBanPackages().contains("maneuvering"))
            guhuo_list << "natural_slash" << "thunder_slash" << "fire_slash";
        to_guhuo = room->askForChoice(card_use->from, "guhuo_slash", guhuo_list.join("+"));
        card_use->from->tag["GuhuoSlash"] = QVariant(to_guhuo);
    }
    room->broadcastSkillInvoke("guhuo");

    LogMessage log;
    log.type = card_use->to.isEmpty() ? "#GuhuoNoTarget" : "#Guhuo";
    log.from = card_use->from;
    log.to = card_use->to;
    log.arg = user_string;
    log.arg2 = "guhuo";

    room->sendLog(log);

    if(guhuo(card_use->from, log.toString())){
        const Card *card = Sanguosha->getCard(subcards.first());
		QString user_str;
		if (to_guhuo == "slash") {
            if (card->isKindOf("Slash"))
                user_str = card->objectName();
            else
                user_str = "slash";
        } else if (to_guhuo == "natural_slash")
            user_str = "slash";
        else
            user_str = to_guhuo;
        Card *use_card = Sanguosha->cloneCard(user_string, card->getSuit(), card->getNumber());
        use_card->setSkillName("guhuo");
        use_card->addSubcard(this);
        /* @todo: verify this...
        CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, card_use->from->objectName());
        room->throwCard(this, reason, NULL); */
        use_card->deleteLater();
        return use_card;
    }else
        return NULL;
}

const Card *GuhuoCard::validateInResposing(ServerPlayer *yuji, bool &continuable) const{
    continuable = true;
    Room *room = yuji->getRoom();
    room->broadcastSkillInvoke("guhuo");

    QString to_guhuo;
    if(user_string == "peach+analeptic") {
        QStringList guhuo_list;
        guhuo_list << "peach";
        if (!Sanguosha->getBanPackages().contains("maneuvering"))
            guhuo_list << "analeptic";
        if (guhuo_list.length() == 1)
            to_guhuo = guhuo_list.first();
        else
            to_guhuo = room->askForChoice(yuji, "guhuo_saveself", guhuo_list.join("+"));
        yuji->tag["GuhuoSaveSelf"] = QVariant(to_guhuo);
    }
    else if (user_string == "slash") {
        QStringList guhuo_list;
        guhuo_list << "slash";
        if (!Sanguosha->getBanPackages().contains("maneuvering"))
            guhuo_list << "natural_slash" << "thunder_slash" << "fire_slash";
        to_guhuo = room->askForChoice(yuji, "guhuo_slash", guhuo_list.join("+"));
        yuji->tag["GuhuoSlash"] = QVariant(to_guhuo);
    }
    else
        to_guhuo = user_string;

    LogMessage log;
    log.type = "#GuhuoNoTarget";
    log.from = yuji;
    log.arg = to_guhuo;
    log.arg2 = "guhuo";
    room->sendLog(log);

    if (guhuo(yuji,log.toString())){
        const Card *card = Sanguosha->getCard(subcards.first());
        QString user_str;
        if (to_guhuo == "slash") {
            if (card->isKindOf("Slash"))
                user_str = card->objectName();
            else
                user_str = "slash";
        } else if (to_guhuo == "natural_slash")
            user_str = "slash";
        else
            user_str = to_guhuo;
        Card *use_card = Sanguosha->cloneCard(user_str, card->getSuit(), card->getNumber());
        use_card->setSkillName("guhuo");
        use_card->deleteLater();
        return use_card;
    }else
        return NULL;
}

class Guhuo: public OneCardViewAsSkill{
public:
    Guhuo():OneCardViewAsSkill("guhuo"){
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return !player->isKongcheng()
                && ! pattern.startsWith("@")
                && ! pattern.startsWith(".");
    }

    virtual bool viewFilter(const Card* to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        if(Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
            GuhuoCard *card = new GuhuoCard;
            card->setUserString(Sanguosha->currentRoomState()->getCurrentCardUsePattern());
            card->addSubcard(originalCard);
            return card;
        }

        CardStar c = Self->tag.value("guhuo").value<CardStar>();
        if(c) {
            GuhuoCard *card = new GuhuoCard;
            card->setUserString(c->objectName());
            card->addSubcard(originalCard);

            return card;
        } else return NULL;
    }

    virtual QDialog *getDialog() const{
        return GuhuoDialog::getInstance("guhuo");
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const{
        if (!card->isKindOf("GuhuoCard"))
            return -2;
        else
            return -1;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        return !player->isKongcheng();
    }
};

WindPackage::WindPackage()
    :Package("wind")
{
    General *xiahouyuan, *caoren, *huangzhong, *weiyan, *zhangjiao, *zhoutai;

    xiahouyuan = new General(this, "xiahouyuan", "wei");
    xiahouyuan->addSkill(new Shensu);

    caoren = new General(this, "caoren", "wei");
    caoren->addSkill(new Jushou);

    huangzhong = new General(this, "huangzhong", "shu");
    huangzhong->addSkill(new Liegong);

    weiyan = new General(this, "weiyan", "shu");
    weiyan->addSkill(new Kuanggu);

    General *xiaoqiao = new General(this, "xiaoqiao", "wu", 3, false);
    xiaoqiao->addSkill(new Tianxiang);
    xiaoqiao->addSkill(new Hongyan);

    zhoutai = new General(this, "zhoutai", "wu");
    zhoutai->addSkill(new Buqu);
    zhoutai->addSkill(new BuquRemove);
    related_skills.insertMulti("buqu", "#buqu-remove");

    zhangjiao = new General(this, "zhangjiao$", "qun", 3);
    zhangjiao->addSkill(new Guidao);
    zhangjiao->addSkill(new Leiji);
    zhangjiao->addSkill(new Huangtian);

    General *yuji = new General(this, "yuji", "qun", 3);
    yuji->addSkill(new Guhuo);

    addMetaObject<ShensuCard>();
    addMetaObject<TianxiangCard>();
    addMetaObject<GuidaoCard>();
    addMetaObject<HuangtianCard>();
    addMetaObject<LeijiCard>();
    addMetaObject<GuhuoCard>();

    skills << new HuangtianViewAsSkill;

}

ADD_PACKAGE(Wind)
