#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "engine.h"
#include "nostalgia.h"
#include "settings.h"

class MoonSpearSkill: public WeaponSkill {
public:
    MoonSpearSkill(): WeaponSkill("MoonSpear") {
        events << CardUsed << CardResponded;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->getPhase() != Player::NotActive)
            return false;

        CardStar card = NULL;
        if (triggerEvent == CardUsed) {
            CardUseStruct card_use = data.value<CardUseStruct>();
            card = card_use.card;
        } else if (triggerEvent == CardResponded) {
            card = data.value<CardResponseStruct>().m_card;
        }

        if (card == NULL || !card->isBlack()
            || (card->getHandlingMethod() != Card::MethodUse && card->getHandlingMethod() != Card::MethodResponse))
            return false;

        player->setFlags("MoonspearUse");
        if (!room->askForUseCard(player, "slash", "@moon-spear-slash", -1, Card::MethodUse, false))
            player->setFlags("-MoonspearUse");

        return false;
    }
};

MoonSpear::MoonSpear(Suit suit, int number)
    : Weapon(suit, number, 3)
{
    setObjectName("MoonSpear");
}

NostalgiaPackage::NostalgiaPackage()
    : Package("nostalgia")
{
    type = CardPack;

    Card *moon_spear = new MoonSpear;
    moon_spear->setParent(this);

    skills << new MoonSpearSkill;
}


// old stantard generals

class NosJizhi: public TriggerSkill {
public:
    NosJizhi(): TriggerSkill("nosjizhi") {
        frequency = Frequent;
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *yueying, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();

        if (use.card->isNDTrick() && room->askForSkillInvoke(yueying, objectName())) {
            room->broadcastSkillInvoke("jizhi");
            yueying->drawCards(1);
        }

        return false;
    }
};

class NosQicai: public TargetModSkill {
public:
    NosQicai(): TargetModSkill("nosqicai") {
        pattern = "TrickCard";
    }

    virtual int getDistanceLimit(const Player *from, const Card *) const{
        if (from->hasSkill(objectName()))
            return 1000;
        else
            return 0;
    }
};

NosLijianCard::NosLijianCard(): LijianCard(false) {
}

class NosLijian: public OneCardViewAsSkill {
public:
    NosLijian(): OneCardViewAsSkill("noslijian") {
        filter_pattern = ".!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getAliveSiblings().length() > 1
               && player->canDiscard(player, "he") && !player->hasUsed("NosLijianCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        NosLijianCard *lijian_card = new NosLijianCard;
        lijian_card->addSubcard(originalCard->getId());
        return lijian_card;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const{
        return card->isKindOf("Duel") ? 0 : -1;
    }
};

// old wind generals

class NosLeiji: public TriggerSkill {
public:
    NosLeiji(): TriggerSkill("nosleiji") {
        events << CardResponded;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *zhangjiao, QVariant &data) const{
        CardStar card_star = data.value<CardResponseStruct>().m_card;
        if (card_star->isKindOf("Jink")) {
            ServerPlayer *target = room->askForPlayerChosen(zhangjiao, room->getAlivePlayers(), objectName(), "leiji-invoke", true, true);
            if (target) {
                room->broadcastSkillInvoke("leiji");

                JudgeStruct judge;
                judge.pattern = ".|spade";
                judge.good = false;
                judge.negative = true;
                judge.reason = objectName();
                judge.who = target;

                room->judge(judge);

                if (judge.isBad())
                    room->damage(DamageStruct(objectName(), zhangjiao, target, 2, DamageStruct::Thunder));
            }
        }
        return false;
    }
};

#include "wind.h"
class NosJushou: public Jushou {
public:
    NosJushou(): Jushou() {
        setObjectName("nosjushou");
    }

    virtual int getJushouDrawNum(ServerPlayer *) const{
        return 3;
    }
};

class NosBuquRemove: public TriggerSkill {
public:
    NosBuquRemove(): TriggerSkill("#nosbuqu-remove") {
        events << HpRecover;
    }

    static void Remove(ServerPlayer *zhoutai) {
        Room *room = zhoutai->getRoom();
        QList<int> nosbuqu(zhoutai->getPile("nosbuqu"));

        CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), "nosbuqu", QString());
        int need = 1 - zhoutai->getHp();
        if (need <= 0) {
            // clear all the buqu cards
            foreach (int card_id, nosbuqu) {
                LogMessage log;
                log.type = "$NosBuquRemove";
                log.from = zhoutai;
                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);

                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
            }
        } else {
            int to_remove = nosbuqu.length() - need;
            for (int i = 0; i < to_remove; i++) {
                room->fillAG(nosbuqu);
                int card_id = room->askForAG(zhoutai, nosbuqu, false, "nosbuqu");

                LogMessage log;
                log.type = "$NosBuquRemove";
                log.from = zhoutai;
                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);

                nosbuqu.removeOne(card_id);
                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
                room->clearAG();
            }
        }
    }

    virtual bool trigger(TriggerEvent, Room *, ServerPlayer *zhoutai, QVariant &) const{
        if (zhoutai->getPile("nosbuqu").length() > 0)
            Remove(zhoutai);

        return false;
    }
};

class NosBuqu: public TriggerSkill {
public:
    NosBuqu(): TriggerSkill("nosbuqu") {
        events << PostHpReduced << AskForPeachesDone;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *zhoutai, QVariant &data) const{
        if (triggerEvent == PostHpReduced && zhoutai->getHp() < 1) {
            if (room->askForSkillInvoke(zhoutai, objectName(), data)) {
                room->setTag("NosBuqu", zhoutai->objectName());
                room->broadcastSkillInvoke("buqu");
                const QList<int> &nosbuqu = zhoutai->getPile("nosbuqu");

                int need = 1 - zhoutai->getHp(); // the buqu cards that should be turned over
                int n = need - nosbuqu.length();
                if (n > 0) {
                    QList<int> card_ids = room->getNCards(n, false);
                    zhoutai->addToPile("nosbuqu", card_ids);
                }
                const QList<int> &nosbuqunew = zhoutai->getPile("nosbuqu");
                QList<int> duplicate_numbers;

                QSet<int> numbers;
                foreach (int card_id, nosbuqunew) {
                    const Card *card = Sanguosha->getCard(card_id);
                    int number = card->getNumber();

                    if (numbers.contains(number))
                        duplicate_numbers << number;
                    else
                        numbers << number;
                }

                if (duplicate_numbers.isEmpty()) {
                    room->setTag("NosBuqu", QVariant());
                    return true;
                }
            }
        } else if (triggerEvent == AskForPeachesDone) {
            const QList<int> &nosbuqu = zhoutai->getPile("nosbuqu");

            if (zhoutai->getHp() > 0)
                return false;
            if (room->getTag("NosBuqu").toString() != zhoutai->objectName())
                return false;
            room->setTag("NosBuqu", QVariant());

            QList<int> duplicate_numbers;
            QSet<int> numbers;
            foreach (int card_id, nosbuqu) {
                const Card *card = Sanguosha->getCard(card_id);
                int number = card->getNumber();

                if (numbers.contains(number) && !duplicate_numbers.contains(number))
                    duplicate_numbers << number;
                else
                    numbers << number;
            }

            if (duplicate_numbers.isEmpty()) {
                room->broadcastSkillInvoke("buqu");
                room->setPlayerFlag(zhoutai, "-Global_Dying");
                return true;
            } else {
                LogMessage log;
                log.type = "#NosBuquDuplicate";
                log.from = zhoutai;
                log.arg = QString::number(duplicate_numbers.length());
                room->sendLog(log);

                for (int i = 0; i < duplicate_numbers.length(); i++) {
                    int number = duplicate_numbers.at(i);

                    LogMessage log;
                    log.type = "#NosBuquDuplicateGroup";
                    log.from = zhoutai;
                    log.arg = QString::number(i + 1);
                    if (number == 10)
                        log.arg2 = "10";
                    else {
                        const char *number_string = "-A23456789-JQK";
                        log.arg2 = QString(number_string[number]);
                    }
                    room->sendLog(log);

                    foreach (int card_id, nosbuqu) {
                        const Card *card = Sanguosha->getCard(card_id);
                        if (card->getNumber() == number) {
                            LogMessage log;
                            log.type = "$NosBuquDuplicateItem";
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

class NosBuquClear: public DetachEffectSkill {
public:
    NosBuquClear(): DetachEffectSkill("nosbuqu") {
    }

    virtual void onSkillDetached(Room *room, ServerPlayer *player) const{
        if (player->getHp() <= 0)
            room->enterDying(player, NULL);
    }
};

NosGuhuoCard::NosGuhuoCard() {
    mute = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool NosGuhuoCard::nosguhuo(ServerPlayer *yuji) const{
    Room *room = yuji->getRoom();
    QList<ServerPlayer *> players = room->getOtherPlayers(yuji);
    QSet<ServerPlayer *> questioned;

    QList<int> used_cards;
    QList<CardsMoveStruct> moves;
    foreach (int card_id, getSubcards())
        used_cards << card_id;
    room->setTag("NosGuhuoType", user_string);

    foreach (ServerPlayer *player, players) {
        if (player->getHp() <= 0) {
            LogMessage log;
            log.type = "#GuhuoCannotQuestion";
            log.from = player;
            log.arg = QString::number(player->getHp());
            room->sendLog(log);

            room->setEmotion(player, "no-question");
            continue;
        }

        QString choice = room->askForChoice(player, "nosguhuo", "noquestion+question");
        if (choice == "question") {
            room->setEmotion(player, "question");
            questioned << player;
        } else
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
    if (questioned.isEmpty()) {
        success = true;
        foreach (ServerPlayer *player, players)
            room->setEmotion(player, ".");

        CardMoveReason reason(CardMoveReason::S_REASON_USE, yuji->objectName(), QString(), "nosguhuo");
        CardsMoveStruct move(used_cards, yuji, NULL, Player::PlaceUnknown, Player::PlaceTable, reason);
        moves.append(move);
        room->moveCardsAtomic(moves, true);
    } else {
        const Card *card = Sanguosha->getCard(subcards.first());
        bool real;
        if (user_string == "peach+analeptic")
            real = card->objectName() == yuji->tag["NosGuhuoSaveSelf"].toString();
        else if (user_string == "slash")
            real = card->objectName().contains("slash");
        else if (user_string == "normal_slash")
            real = card->objectName() == "slash";
        else
            real = card->match(user_string);

        success = real && card->getSuit() == Card::Heart;
        if (success) {
            CardMoveReason reason(CardMoveReason::S_REASON_USE, yuji->objectName(), QString(), "nosguhuo");
            CardsMoveStruct move(used_cards, yuji, NULL, Player::PlaceUnknown, Player::PlaceTable, reason);
            moves.append(move);
            room->moveCardsAtomic(moves, true);
        } else {
            room->moveCardTo(this, yuji, NULL, Player::DiscardPile,
                             CardMoveReason(CardMoveReason::S_REASON_PUT, yuji->objectName(), QString(), "nosguhuo"), true);
        }
        foreach (ServerPlayer *player, players) {
            room->setEmotion(player, ".");
            if (questioned.contains(player)) {
                if (real)
                    room->loseHp(player);
                else
                    player->drawCards(1);
            }
        }
    }
    yuji->tag.remove("NosGuhuoSaveSelf");
    yuji->tag.remove("NosGuhuoSlash");
    return success;
}

bool NosGuhuoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
    } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return false;
    }

    CardStar card = Self->tag.value("nosguhuo").value<CardStar>();
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
}

bool NosGuhuoCard::targetFixed() const{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFixed();
    } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return true;
    }

    CardStar card = Self->tag.value("nosguhuo").value<CardStar>();
    return card && card->targetFixed();
}

bool NosGuhuoCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetsFeasible(targets, Self);
    } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return true;
    }

    CardStar card = Self->tag.value("nosguhuo").value<CardStar>();
    return card && card->targetsFeasible(targets, Self);
}

const Card *NosGuhuoCard::validate(CardUseStruct &card_use) const{
    ServerPlayer *yuji = card_use.from;
    Room *room = yuji->getRoom();

    QString to_nosguhuo = user_string;
    if (user_string == "slash"
        && Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        QStringList nosguhuo_list;
        nosguhuo_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            nosguhuo_list << "normal_slash" << "thunder_slash" << "fire_slash";
        to_nosguhuo = room->askForChoice(yuji, "nosguhuo_slash", nosguhuo_list.join("+"));
        yuji->tag["NosGuhuoSlash"] = QVariant(to_nosguhuo);
    }
    room->broadcastSkillInvoke("nosguhuo");

    LogMessage log;
    log.type = card_use.to.isEmpty() ? "#GuhuoNoTarget" : "#Guhuo";
    log.from = yuji;
    log.to = card_use.to;
    log.arg = to_nosguhuo;
    log.arg2 = "nosguhuo";

    room->sendLog(log);

    if (nosguhuo(card_use.from)) {
        const Card *card = Sanguosha->getCard(subcards.first());
        QString user_str;
        if (to_nosguhuo == "slash") {
            if (card->isKindOf("Slash"))
                user_str = card->objectName();
            else
                user_str = "slash";
        } else if (to_nosguhuo == "normal_slash")
            user_str = "slash";
        else
            user_str = to_nosguhuo;
        Card *use_card = Sanguosha->cloneCard(user_str, card->getSuit(), card->getNumber());
        use_card->setSkillName("nosguhuo");
        use_card->addSubcard(subcards.first());
        use_card->deleteLater();
        return use_card;
    } else
        return NULL;
}

const Card *NosGuhuoCard::validateInResponse(ServerPlayer *yuji) const{
    Room *room = yuji->getRoom();
    room->broadcastSkillInvoke("nosguhuo");

    QString to_nosguhuo;
    if (user_string == "peach+analeptic") {
        QStringList nosguhuo_list;
        nosguhuo_list << "peach";
        if (!Config.BanPackages.contains("maneuvering"))
            nosguhuo_list << "analeptic";
        to_nosguhuo = room->askForChoice(yuji, "nosguhuo_saveself", nosguhuo_list.join("+"));
        yuji->tag["NosGuhuoSaveSelf"] = QVariant(to_nosguhuo);
    } else if (user_string == "slash") {
        QStringList nosguhuo_list;
        nosguhuo_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            nosguhuo_list << "normal_slash" << "thunder_slash" << "fire_slash";
        to_nosguhuo = room->askForChoice(yuji, "nosguhuo_slash", nosguhuo_list.join("+"));
        yuji->tag["NosGuhuoSlash"] = QVariant(to_nosguhuo);
    }
    else
        to_nosguhuo = user_string;

    LogMessage log;
    log.type = "#GuhuoNoTarget";
    log.from = yuji;
    log.arg = to_nosguhuo;
    log.arg2 = "nosguhuo";
    room->sendLog(log);

    if (nosguhuo(yuji)) {
        const Card *card = Sanguosha->getCard(subcards.first());
        QString user_str;
        if (to_nosguhuo == "slash") {
            if (card->isKindOf("Slash"))
                user_str = card->objectName();
            else
                user_str = "slash";
        } else if (to_nosguhuo == "normal_slash")
            user_str = "slash";
        else
            user_str = to_nosguhuo;
        Card *use_card = Sanguosha->cloneCard(user_str, card->getSuit(), card->getNumber());
        use_card->setSkillName("nosguhuo");
        use_card->addSubcard(subcards.first());
        use_card->deleteLater();
        return use_card;
    } else
        return NULL;
}

class NosGuhuo: public OneCardViewAsSkill {
public:
    NosGuhuo(): OneCardViewAsSkill("nosguhuo") {
        filter_pattern = ".|.|.|hand";
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if (player->isKongcheng() || pattern.startsWith(".") || pattern.startsWith("@")) return false;
        if (pattern == "peach" && player->hasFlag("Global_PreventPeach")) return false;
        for (int i = 0; i < pattern.length(); i++) {
            QChar ch = pattern[i];
            if (ch.isUpper() || ch.isDigit()) return false; // This is an extremely dirty hack!! For we need to prevent patterns like 'BasicCard'
        }
        return true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE
            || Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
            NosGuhuoCard *card = new NosGuhuoCard;
            card->setUserString(Sanguosha->currentRoomState()->getCurrentCardUsePattern());
            card->addSubcard(originalCard);
            return card;
        }

        CardStar c = Self->tag.value("nosguhuo").value<CardStar>();
        if (c) {
            NosGuhuoCard *card = new NosGuhuoCard;
            if (!c->objectName().contains("slash"))
                card->setUserString(c->objectName());
            else
                card->setUserString(Self->tag["NosGuhuoSlash"].toString());
            card->addSubcard(originalCard);
            return card;
        } else
            return NULL;
    }

    virtual QDialog *getDialog() const{
        return GuhuoDialog::getInstance("nosguhuo");
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const{
        if (!card->isKindOf("NosGuhuoCard"))
            return -2;
        else
            return -1;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        return !player->isKongcheng();
    }
};

NostalStandardPackage::NostalStandardPackage()
    : Package("nostal_standard")
{
    General *huangyueying = new General(this, "nos_huangyueying", "shu", 3, false, true, true);
    huangyueying->addSkill(new NosJizhi);
    huangyueying->addSkill(new NosQicai);

    General *nos_diaochan = new General(this, "nos_diaochan", "qun", 3, false, true, true);
    nos_diaochan->addSkill(new NosLijian);
    nos_diaochan->addSkill("biyue");

    addMetaObject<NosLijianCard>();
}

NostalWindPackage::NostalWindPackage()
    : Package("nostal_wind")
{
    General *nos_caoren = new General(this, "nos_caoren", "wei");
    nos_caoren->addSkill(new NosJushou);

    General *nos_zhoutai = new General(this, "nos_zhoutai", "wu");
    nos_zhoutai->addSkill(new NosBuqu);
    nos_zhoutai->addSkill(new NosBuquRemove);
    nos_zhoutai->addSkill(new NosBuquClear);
    related_skills.insertMulti("nosbuqu", "#nosbuqu-remove");
    related_skills.insertMulti("nosbuqu", "#nosbuqu-clear");

    General *nos_zhangjiao = new General(this, "nos_zhangjiao$", "qun", 3);
    nos_zhangjiao->addSkill(new NosLeiji);
    nos_zhangjiao->addSkill("guidao");
    nos_zhangjiao->addSkill("huangtian");

    General *nos_yuji = new General(this, "nos_yuji", "qun", 3);
    nos_yuji->addSkill(new NosGuhuo);

    addMetaObject<NosGuhuoCard>();
}


ADD_PACKAGE(Nostalgia)
ADD_PACKAGE(NostalStandard)
ADD_PACKAGE(NostalWind)


