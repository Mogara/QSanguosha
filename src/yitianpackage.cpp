#include "yitianpackage.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "god.h"
#include "standard.h"
#include "standard-commons.h"

class YitianSwordSkill : public WeaponSkill{
public:
    YitianSwordSkill():WeaponSkill("yitian_sword"){
        events << CardGot;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardMoveStruct move = data.value<CardMoveStruct>();
        const Card *card = Sanguosha->getCard(move.card_id);
        Room *room = player->getRoom();
        if(room->getCurrent() != player && card->inherits("Slash") && move.to_place == Player::Hand){
            QString pattern = QString("@@yitian-%1").arg(move.card_id);
            room->askForUseCard(player, pattern, "@yitian-sword");
        }

        return false;
    }
};

class YitianSwordViewAsSkill: public ViewAsSkill{
public:
    YitianSwordViewAsSkill():ViewAsSkill("yitian_sword"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern.startsWith("@@yitian-");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(!selected.isEmpty())
            return false;

        QString pattern = ClientInstance->card_pattern;
        pattern.remove("@@yitian-");
        int card_id = pattern.toInt();

        return to_select->getCard()->getId() == card_id;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        return cards.first()->getCard();
    }
};

YitianSword::YitianSword(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("yitian_sword");
    skill = new YitianSwordSkill;
    attach_skill = true;
}

void YitianSword::onMove(const CardMoveStruct &move) const{
    if(move.from_place == Player::Equip && move.from->isAlive()){
        Room *room = move.from->getRoom();

        bool invoke = move.from->askForSkillInvoke(objectName());
        if(!invoke)
            return;

        ServerPlayer *target = room->askForPlayerChosen(move.from, room->getAllPlayers(), objectName());
        DamageStruct damage;
        damage.from = move.from;
        damage.to = target;
        damage.card = this;

        room->damage(damage);
    }
}

ChengxiangCard::ChengxiangCard()
{

}

bool ChengxiangCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.length() < subcardsLength() && to_select->isWounded();
}

bool ChengxiangCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    return targets.length() <= subcardsLength();
}

void ChengxiangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> to = targets;

    if(to.isEmpty())
        to << source;

    return SkillCard::use(room, source, to);
}

void ChengxiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    room->recover(effect.to);
}

class ChengxiangViewAsSkill: public ViewAsSkill{
public:
    ChengxiangViewAsSkill():ViewAsSkill("chengxiang"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@chengxiang";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 3)
            return false;

        int sum = 0;
        foreach(CardItem *item, selected){
            sum += item->getCard()->getNumber();
        }

        sum += to_select->getCard()->getNumber();

        return sum <= Self->getMark("chengxiang");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        int sum = 0;
        foreach(CardItem *item, cards){
            sum += item->getCard()->getNumber();
        }

        if(sum == Self->getMark("chengxiang")){
            ChengxiangCard *card = new ChengxiangCard;
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }
};

class Chengxiang: public MasochismSkill{
public:
    Chengxiang():MasochismSkill("chengxiang"){
        view_as_skill = new ChengxiangViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *caochong, const DamageStruct &damage) const{
        const Card *card = damage.card;
        if(card == NULL)
            return;

        int point = card->getNumber();
        if(point == 0)
            return;

        Room *room = caochong->getRoom();
        room->setPlayerMark(caochong, objectName(), point);

        QString prompt = QString("@chengxiang-card:::%1").arg(point);
        room->askForUseCard(caochong, "@@chengxiang", prompt);
    }
};

class Conghui: public PhaseChangeSkill{
public:
    Conghui():PhaseChangeSkill("conghui"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *caochong) const{
        if(caochong->getPhase() == Player::Discard)
            return true;
        else
            return false;
    }
};

class Zaoyao: public PhaseChangeSkill{
public:
    Zaoyao():PhaseChangeSkill("zaoyao"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *caochong) const{
        if(caochong->getPhase() == Player::Finish && caochong->getHandcardNum() > 13){
            caochong->throwAllCards();
            caochong->getRoom()->loseHp(caochong);
        }

        return false;
    }
};

class Guixin2: public PhaseChangeSkill{
public:
    Guixin2():PhaseChangeSkill("guixin2"){

    }

    virtual bool onPhaseChange(ServerPlayer *shencc) const{
        if(shencc->getPhase() != Player::Finish)
            return false;

        Room *room = shencc->getRoom();
        if(!room->askForSkillInvoke(shencc, objectName()))
            return false;

        QString choice = room->askForChoice(shencc, objectName(), "modify+obtain");

        if(choice == "modify"){
            ServerPlayer *to_modify = room->askForPlayerChosen(shencc, room->getOtherPlayers(shencc), objectName());
            QString kingdom = room->askForKingdom(shencc);
            QString old_kingdom = to_modify->getKingdom();
            room->setPlayerProperty(to_modify, "kingdom", kingdom);

            LogMessage log;
            log.type = "#ChangeKingdom";
            log.from = shencc;
            log.to << to_modify;
            log.arg = old_kingdom;
            log.arg2 = kingdom;
            room->sendLog(log);

        }else if(choice == "obtain"){
            QStringList lords = Sanguosha->getLords();
            QList<ServerPlayer *> players = room->getOtherPlayers(shencc);
            foreach(ServerPlayer *player, players){
                lords.removeOne(player->getGeneralName());
            }

            QStringList lord_skills;
            foreach(QString lord, lords){
                const General *general = Sanguosha->getGeneral(lord);
                QList<const Skill *> skills = general->findChildren<const Skill *>();
                foreach(const Skill *skill, skills){
                    if(skill->isLordSkill() && !shencc->hasSkill(skill->objectName()))
                        lord_skills << skill->objectName();
                }
            }

            if(!lord_skills.isEmpty()){
                QString skill_name = room->askForChoice(shencc, objectName(), lord_skills.join("+"));

                const Skill *skill = Sanguosha->getSkill(skill_name);
                room->acquireSkill(shencc, skill);

                if(skill->inherits("GameStartSkill")){
                    const GameStartSkill *game_start_skill = qobject_cast<const GameStartSkill *>(skill);
                    game_start_skill->onGameStart(shencc);
                }
            }
        }

        room->playSkillEffect("guixin");

        return false;
    }
};

class Ganzhen: public TriggerSkill{
public:
    Ganzhen():TriggerSkill("ganzhen"){
        events << CardUsed << CardResponsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card && card->isBlack() && card->inherits("BasicCard")){
            Room *room = player->getRoom();
            QList<ServerPlayer *> players = room->getAllPlayers();

            // find Cao Zhi
            ServerPlayer *caozhi = NULL;
            foreach(ServerPlayer *player, players){
                if(player->hasSkill(objectName())){
                    caozhi = player;
                    break;
                }
            }

            if(caozhi && room->askForSkillInvoke(caozhi, objectName(), data)){
                player->drawCards(1);
            }
        }

        return false;
    }
};

JuejiCard::JuejiCard(){
    once = true;
}

bool JuejiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.length() < 2 && to_select != Self;
}

void JuejiCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->gainMark("@jueji");
}

class JuejiViewAsSkill: public ViewAsSkill{
public:
    JuejiViewAsSkill():ViewAsSkill("jueji"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! ClientInstance->hasUsed("JuejiCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return true;
        else if(selected.length() == 1){
            const Card *first = selected.first()->getFilteredCard();
            return first->sameColorWith(to_select->getFilteredCard());
        }

        return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            JuejiCard *card = new JuejiCard;
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }
};

class Jueji: public DrawCardsSkill{
public:
    Jueji():DrawCardsSkill("jueji"){
        view_as_skill = new JuejiViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@jueji") > 0;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        Room *room = player->getRoom();
        player->loseMark("@jueji");

        // find zhanghe
        ServerPlayer *zhanghe = room->findPlayerBySkillName(objectName());
        if(zhanghe){
            zhanghe->drawCards(1);
        }

        return n - 1;
    }
};

class JuejiClear: public PhaseChangeSkill{
public:
    JuejiClear():PhaseChangeSkill("#jueji-clear"){

    }

    virtual bool onPhaseChange(ServerPlayer *zhanghe) const{
        if(zhanghe->getPhase() == Player::Start){
            Room *room = zhanghe->getRoom();
            QList<ServerPlayer *> players = room->getOtherPlayers(zhanghe);
            foreach(ServerPlayer *player, players){
                if(player->getMark("@jueji") > 0){
                    player->loseMark("@jueji");
                }
            }
        }

        return false;
    }
};

class FanjiGet: public TriggerSkill{
public:
    FanjiGet():TriggerSkill("#fanji-get"){
        events << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        QVariant card_data = room->getTag("FanjiCard");

        if(!card_data.isValid())
            return false;

        CardStar card = card_data.value<CardStar>();
        room->setTag("FanjiCard", QVariant());

        ServerPlayer *lukang = room->findPlayerBySkillName(objectName());
        if(lukang == NULL)
            return false;

        if(!room->obtainable(card, lukang))
            return false;

        if(lukang->askForSkillInvoke("fanji", data))
            lukang->obtainCard(card);

        return false;
    }
};

class Fanji: public TriggerSkill{
public:
    Fanji():TriggerSkill("fanji"){
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *lukang, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.multiple && effect.card->isNDTrick() && effect.from != lukang){
            Room *room = effect.to->getRoom();
            room->setTag("FanjiCard", QVariant::fromValue(effect.card));
        }

        return false;
    }
};

// ---------- Lianli related skills

LianliCard::LianliCard(){

}

bool LianliCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && to_select->getGeneral()->isMale();
}

void LianliCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    LogMessage log;
    log.type = "#LianliConnection";
    log.from = effect.from;
    log.to << effect.to;
    room->sendLog(log);

    if(effect.from->getMark("@tied") == 0)
        effect.from->gainMark("@tied");

    if(effect.to->getMark("@tied") == 0){
        QList<ServerPlayer *> players = room->getOtherPlayers(effect.from);
        foreach(ServerPlayer *player, players){
            if(player->getMark("@tied") > 0){
                player->loseMark("@tied");
                break;
            }
        }

        effect.to->gainMark("@tied");
    }
}

class LianliStart: public GameStartSkill{
public:
    LianliStart():GameStartSkill("#lianli-start") {

    }

    virtual void onGameStart(ServerPlayer *player) const{
        Room *room = player->getRoom();

        QList<ServerPlayer *> players = room->getOtherPlayers(player);
        foreach(ServerPlayer *player, players){
            if(player->getGeneral()->isMale())
                room->attachSkillToPlayer(player, "lianli-slash");
        }
    }
};

LianliSlashCard::LianliSlashCard(){

}

bool LianliSlashCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && Self->canSlash(to_select);
}

void LianliSlashCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhangfei = effect.from;
    Room *room = zhangfei->getRoom();

    ServerPlayer *xiahoujuan = room->findPlayerBySkillName("lianli");
    if(xiahoujuan){
        const Card *slash = room->askForCard(xiahoujuan, "slash", "@lianli-slash");
        if(slash){
            zhangfei->invoke("increaseSlashCount");
            room->cardEffect(slash, zhangfei, effect.to);
            return;
        }
    }
}

class LianliSlashViewAsSkill:public ZeroCardViewAsSkill{
public:
    LianliSlashViewAsSkill():ZeroCardViewAsSkill("lianli-slash"){

    }

    virtual bool isEnabledAtPlay() const{
        return Self->getMark("@tied") > 0 && Slash::IsAvailable();
    }

    virtual const Card *viewAs() const{
        return new LianliSlashCard;
    }
};

class LianliSlash: public TriggerSkill{
public:
    LianliSlash():TriggerSkill("#lianli-slash"){
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@tied") > 0 && !target->hasSkill("lianli");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "slash")
            return false;

        Room *room = player->getRoom();
        if(!player->askForSkillInvoke("lianli-slash", data))
            return false;

        ServerPlayer *xiahoujuan = room->findPlayerBySkillName("lianli");
        if(xiahoujuan){
            const Card *slash = room->askForCard(xiahoujuan, "slash", "@lianli-slash");
            if(slash){
                room->provide(slash);
                return true;
            }
        }

        return false;
    }
};

class LianliJink: public TriggerSkill{
public:
    LianliJink():TriggerSkill("#lianli-jink"){
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@tied") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *xiahoujuan, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "jink")
            return false;

        if(!xiahoujuan->askForSkillInvoke("lianli-jink", data))
            return false;

        Room *room = xiahoujuan->getRoom();
        QList<ServerPlayer *> players = room->getOtherPlayers(xiahoujuan);
        foreach(ServerPlayer *player, players){
            if(player->getMark("@tied") > 0){
                ServerPlayer *zhangfei = player;

                const Card *jink = room->askForCard(zhangfei, "jink", "@lianli-jink");
                if(jink){
                    room->provide(jink);
                    return true;
                }

                break;
            }
        }

        return false;
    }
};

class LianliViewAsSkill: public ZeroCardViewAsSkill{
public:
    LianliViewAsSkill():ZeroCardViewAsSkill(""){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@lianli";
    }

    virtual const Card *viewAs() const{
        return new LianliCard;
    }
};

class Lianli: public PhaseChangeSkill{
public:
    Lianli():PhaseChangeSkill("lianli"){
        view_as_skill = new LianliViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Start){
            Room *room = target->getRoom();
            bool used = room->askForUseCard(target, "@lianli", "@@lianli-card");
            if(used){
                if(target->getKingdom() != "shu")
                    room->setPlayerProperty(target, "kingdom", "shu");
            }else{
                if(target->getKingdom() != "wei")
                    room->setPlayerProperty(target, "kingdom", "wei");

                QList<ServerPlayer *> players = room->getAllPlayers();
                foreach(ServerPlayer *player, players){
                    if(player->getMark("@tied") > 0)
                        player->loseMark("@tied");
                }
            }
        }

        return false;
    }
};

class Tongxin: public MasochismSkill{
public:
    Tongxin():MasochismSkill("tongxin"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@tied") > 0;
    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{
        Room *room = target->getRoom();
        ServerPlayer *xiahoujuan = room->findPlayerBySkillName(objectName());

        if(xiahoujuan && xiahoujuan->askForSkillInvoke(objectName(), QVariant::fromValue(damage))){
            room->playSkillEffect(objectName());

            ServerPlayer *zhangfei = NULL;
            if(target == xiahoujuan){
                QList<ServerPlayer *> players = room->getOtherPlayers(xiahoujuan);
                foreach(ServerPlayer *player, players){
                    if(player->getMark("@tied") > 0){
                        zhangfei = player;
                        break;
                    }
                }
            }else
                zhangfei = target;

            xiahoujuan->drawCards(damage.damage);

            if(zhangfei)
                zhangfei->drawCards(damage.damage);
        }
    }
};

class LianliClear: public TriggerSkill{
public:
    LianliClear():TriggerSkill("#lianli-clear"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> players = room->getAllPlayers();
        foreach(ServerPlayer *player, players){
            if(player->getMark("@tied") > 0)
                player->loseMark("@tied");
        }

        return false;
    }
};

// -------- end of Lianli related skills

QiaocaiCard::QiaocaiCard(){
    once = true;
}

void QiaocaiCard::onEffect(const CardEffectStruct &effect) const{
    QStack<const Card *> cards = effect.to->getJudgingArea();
    foreach(const Card *card, cards){
        effect.from->obtainCard(card);
    }
 }

class Qiaocai: public ZeroCardViewAsSkill{
public:
    Qiaocai():ZeroCardViewAsSkill("qiaocai"){

    }

    virtual bool isEnabledAtPlay() const{
        return Self->getMark("@tied") == 0 && ! ClientInstance->hasUsed("QiaocaiCard");
    }

    virtual const Card *viewAs() const{
        return new QiaocaiCard;
    }
};

class Jinshen: public ProhibitSkill{
public:
    Jinshen():ProhibitSkill("jinshen"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return card->inherits("Indulgence") || card->inherits("SupplyShortage");
    }
};

class WulingExEffect: public TriggerSkill{
public:
    WulingExEffect():TriggerSkill("#wuling-ex-effect"){
        events << CardEffected << Predamaged;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *xuandi = room->findPlayerBySkillName(objectName());
        if(xuandi == NULL)
            return false;

        QString wuling = xuandi->tag.value("wuling").toString();
        if(event == CardEffected && wuling == "water"){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card && effect.card->inherits("Peach")){
                room->recover(player);

                LogMessage log;
                log.type = "#WulingWater";
                log.from = player;
                room->sendLog(log);
            }
        }else if(event == Predamaged && wuling == "earth"){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature != DamageStruct::Normal && damage.damage > 1){
                damage.damage = 1;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#WulingEarth";
                log.from = player;
                room->sendLog(log);
            }
        }

        return false;
    }
};

class WulingEffect: public TriggerSkill{
public:
    WulingEffect():TriggerSkill("#wuling-effect"){
        events << Predamaged;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *xuandi = room->findPlayerBySkillName(objectName());
        if(xuandi == NULL)
            return false;

        QString wuling = xuandi->tag.value("wuling").toString();
        DamageStruct damage = data.value<DamageStruct>();

        if(wuling == "wind"){
            if(damage.nature == DamageStruct::Fire && !damage.chain){
                damage.damage ++;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#WulingWind";
                log.from = damage.to;
                log.arg = QString::number(damage.damage - 1);
                log.arg2 = QString::number(damage.damage);
                room->sendLog(log);
            }
        }else if(wuling == "thunder"){
            if(damage.nature == DamageStruct::Thunder && !damage.chain){
                damage.damage ++;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#WulingThunder";
                log.from = damage.to;
                log.arg = QString::number(damage.damage - 1);
                log.arg2 = QString::number(damage.damage);
                room->sendLog(log);
            }
        }else if(wuling == "fire"){
            if(damage.nature != DamageStruct::Fire){
                damage.nature = DamageStruct::Fire;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#WulingFire";
                log.from = damage.to;
                room->sendLog(log);
            }
        }

        return false;
    }
};

class Wuling: public PhaseChangeSkill{
public:
    Wuling():PhaseChangeSkill("wuling"){
        default_choice = "wind";
    }

    virtual bool onPhaseChange(ServerPlayer *xuandi) const{
        static QStringList effects;
        if(effects.isEmpty()){
            effects << "wind" << "thunder" << "water" << "fire" << "earth";
        }

        if(xuandi->getPhase() == Player::Start){
            QString current = xuandi->tag.value("wuling").toString();
            QStringList choices;
            foreach(QString effect, effects){
                if(effect != current)
                    choices << effect;
            }

            Room *room = xuandi->getRoom();
            QString choice = room->askForChoice(xuandi, objectName(), choices.join("+"));
            if(!current.isEmpty())
                xuandi->loseMark("@" + current);

            xuandi->gainMark("@" + choice);
            xuandi->tag["wuling"] = choice;
        }

        return false;
    }
};

GuihanCard::GuihanCard(){
    once = true;
}

void GuihanCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *caizhaoji = effect.from;
    caizhaoji->getRoom()->swapSeat(caizhaoji, effect.to);
}

class Guihan: public ViewAsSkill{
public:
    Guihan():ViewAsSkill("guihan"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! ClientInstance->hasUsed("GuihanCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(to_select->isEquipped())
            return false;

        if(selected.isEmpty())
            return to_select->getFilteredCard()->isRed();
        else if(selected.length() == 1){
            Card::Suit suit = selected.first()->getFilteredCard()->getSuit();
            return to_select->getFilteredCard()->getSuit() == suit;
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        GuihanCard *card = new GuihanCard;
        card->addSubcards(cards);
        return card;
    }
};

static QString Hujia2Callback(const Card *card, Room *){
    if(card->isRed())
        return "good";
    else
        return "bad";
}

class CaizhaojiHujia: public PhaseChangeSkill{
public:
    CaizhaojiHujia():PhaseChangeSkill("caizhaoji_hujia"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            int times = 0;
            Room *room = target->getRoom();
            while(target->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());

                times ++;
                if(times == 3){
                    target->turnOver();
                }

                CardStar card;
                if(room->judge(target, Hujia2Callback, &card) == "good"){
                    target->obtainCard(card);
                }else
                    break;
            }
        }

        return false;
    }
};

class Shenjun: public TriggerSkill{
public:
    Shenjun():TriggerSkill("shenjun"){
        events << GameStart << PhaseChange << Predamaged;
        frequency = Compulsory;
    }   

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == GameStart){
            QString gender = room->askForChoice(player, objectName(), "male+female");
            if(gender == "female"){
                room->transfigure(player, "luboyanf", false);
            }

            LogMessage log;
            log.type = "#ShenjunChoose";
            log.from = player;
            log.arg = gender;
            room->sendLog(log);

        }else if(event == PhaseChange){
            if(player->getPhase() == Player::Start){
                LogMessage log;
                log.from = player;
                log.type = "#ShenjunFlip";
                room->sendLog(log);

                QString new_general = "luboyan";
                if(player->getGeneral()->isMale())
                    new_general.append("f");
                room->transfigure(player, new_general, false);
            }
        }else if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature != DamageStruct::Thunder && damage.from &&
               damage.from->getGeneral()->isMale() != player->getGeneral()->isMale()){

                LogMessage log;
                log.type = "#ShenjunProtect";
                log.to << player;
                log.from = damage.from;
                room->sendLog(log);

                return true;
            }
        }

        return false;
    }
};

class Zonghuo: public TriggerSkill{
public:
    Zonghuo():TriggerSkill("zonghuo"){
        frequency = Compulsory;
        events << SlashEffect;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.nature != DamageStruct::Fire){
            effect.nature = DamageStruct::Fire;

            data = QVariant::fromValue(effect);

            LogMessage log;
            log.type = "#Zonghuo";
            log.from = player;
            player->getRoom()->sendLog(log);
        }

        return false;
    }
};

static QString ShaoyingCallback(const Card *card, Room *){
    if(card->isRed())
        return "good";
    else
        return "bad";
}

class Shaoying: public TriggerSkill{
public:
    Shaoying():TriggerSkill("shaoying"){
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from == NULL)
            return false;

        if(!damage.from->hasSkill(objectName()))
            return false;

        ServerPlayer *luboyan = damage.from;
        if(!damage.to->isChained() && damage.nature == DamageStruct::Fire
           && luboyan->askForSkillInvoke(objectName(), data))
        {
            Room *room = luboyan->getRoom();
            if(room->judge(luboyan, ShaoyingCallback) == "good"){
                DamageStruct shaoying_damage;
                shaoying_damage.nature = DamageStruct::Fire;
                shaoying_damage.from = luboyan;
                shaoying_damage.to = room->getOtherPlayers(damage.to).first();

                room->damage(shaoying_damage);
            }
        }

        return false;
    }
};

class Gongmou: public PhaseChangeSkill{
public:
    Gongmou():PhaseChangeSkill("gongmou"){

    }

    virtual bool onPhaseChange(ServerPlayer *zhongshiji) const{
        switch(zhongshiji->getPhase()){
        case Player::Finish:{
                if(zhongshiji->askForSkillInvoke(objectName())){
                    Room *room = zhongshiji->getRoom();
                    QList<ServerPlayer *> players = room->getOtherPlayers(zhongshiji);
                    ServerPlayer *target = room->askForPlayerChosen(zhongshiji, players, "gongmou");
                    target->gainMark("@conspiracy");
                }
                break;
            }

        case Player::Start:{
                Room *room = zhongshiji->getRoom();
                QList<ServerPlayer *> players = room->getOtherPlayers(zhongshiji);
                foreach(ServerPlayer *player, players){
                    if(player->getMark("@conspiracy") > 0)
                        player->loseMark("@conspiracy");
                }
            }

        default:
            break;
        }


        return false;
    }
};

class GongmouExchange:public PhaseChangeSkill{
public:
    GongmouExchange():PhaseChangeSkill("#gongmou-exchange"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@conspiracy") > 0;
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Draw)
            return false;

        player->loseMark("@conspiracy");

        Room *room = player->getRoom();
        ServerPlayer *zhongshiji = room->findPlayerBySkillName("gongmou");
        if(zhongshiji){
            int x = qMin(zhongshiji->getHandcardNum(), player->getHandcardNum());
            if(x == 0)
                return false;

            const Card *to_exchange = NULL;
            if(player->getHandcardNum() == x)
                to_exchange = player->wholeHandCards();
            else
                to_exchange = room->askForExchange(player, "gongmou", x);

            room->moveCardTo(to_exchange, zhongshiji, Player::Hand, false);

            delete to_exchange;

            to_exchange = room->askForExchange(zhongshiji, "gongmou", x);
            room->moveCardTo(to_exchange, player, Player::Hand, false);

            delete to_exchange;

            LogMessage log;
            log.type = "#GongmouExchange";
            log.from = zhongshiji;
            log.to << player;
            log.arg = QString::number(x);
            room->sendLog(log);
        }

        return false;
    }
};

LexueCard::LexueCard(){
    once = true;
}

void LexueCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    const Card *card = room->askForCardShow(effect.to, effect.from);
    int card_id = card->getEffectiveId();
    room->showCard(effect.to, card_id);

    if(card->inherits("BasicCard") || card->isNDTrick()){
        room->setPlayerMark(effect.from, "lexue", card_id);
        room->setPlayerFlag(effect.from, "lexue");
    }else{
        effect.from->obtainCard(card);
        room->setPlayerFlag(effect.from, "-lexue");
    }
}

class Lexue: public ViewAsSkill{
public:
    Lexue():ViewAsSkill("lexue"){

    }

    virtual bool isEnabledAtPlay() const{
        if(ClientInstance->hasUsed("LexueCard") && Self->hasFlag("lexue")){
            int card_id = Self->getMark("lexue");
            const Card *card = Sanguosha->getCard(card_id);
            return card->isAvailable();
        }else
            return true;
    }

    virtual bool isEnabledAtResponse() const{
        if(!Self->hasFlag("lexue"))
            return false;

        if(ClientInstance->hasUsed("LexueCard")){
            int card_id = Self->getMark("lexue");
            const Card *card = Sanguosha->getCard(card_id);
            return ClientInstance->card_pattern.contains(card->objectName());
        }else
            return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(ClientInstance->hasUsed("LexueCard") && selected.isEmpty() && Self->hasFlag("lexue")){
            int card_id = Self->getMark("lexue");
            const Card *card = Sanguosha->getCard(card_id);
            return to_select->getFilteredCard()->getSuit() == card->getSuit();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(ClientInstance->hasUsed("LexueCard")){
            if(!Self->hasFlag("lexue"))
                return false;

            if(cards.length() != 1)
                return NULL;

            int card_id = Self->getMark("lexue");
            const Card *card = Sanguosha->getCard(card_id);
            const Card *first = cards.first()->getFilteredCard();

            Card *new_card = Sanguosha->cloneCard(card->objectName(), first->getSuit(), first->getNumber());
            new_card->setSkillName(objectName());
            new_card->addSubcards(cards);
            return new_card;
        }else{
            return new LexueCard;
        }
    }
};

XunzhiCard::XunzhiCard(){
    target_fixed = true;
}

void XunzhiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> players = room->getAlivePlayers();
    QStringList all_generals = Sanguosha->getLimitedGeneralNames();
    QStringList shu_generals;
    foreach(QString name, all_generals){
        const General *general = Sanguosha->getGeneral(name);
        if(general->getKingdom() != "shu")
            continue;

        bool duplicated = false;
        foreach(ServerPlayer *player, players){
            if(player->getGeneralName() == name){
                duplicated = true;
                break;
            }
        }

        if(!duplicated)
            shu_generals << name;
    }

    QString general = room->askForGeneral(source, shu_generals);

    room->transfigure(source, general, false);
    room->acquireSkill(source, "xunzhi", false);
}

class XunzhiViewAsSkill: public ZeroCardViewAsSkill{
public:
    XunzhiViewAsSkill():ZeroCardViewAsSkill("#xunzhi"){

    }

    virtual bool isEnabledAtPlay() const{
        return Self->getGeneral()->hasSkill("xunzhi");
    }

    virtual const Card *viewAs() const{
        return new XunzhiCard;
    }
};

class Xunzhi: public PhaseChangeSkill{
public:
    Xunzhi():PhaseChangeSkill("xunzhi"){
        view_as_skill = new XunzhiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish &&
           !target->getGeneral()->hasSkill(objectName()))
        {
            Room *room = target->getRoom();
            room->transfigure(target, parent()->objectName(), false);
            room->killPlayer(target);
        }

        return false;
    }
};

YitianPackage::YitianPackage()
    :Package("yitian")
{

    (new YitianSword)->setParent(this);

    // generals
    General *shencc = new General(this, "shencc$", "god", 3);
    shencc->addSkill(new Guixin2);
    shencc->addSkill(new Feiying);

    General *caochong = new General(this, "caochong", "wei", 3);
    caochong->addSkill(new Chengxiang);
    caochong->addSkill(new Conghui);
    caochong->addSkill(new Zaoyao);

    //General *caozhi = new General(this, "caozhi", "wei", 3);
    //caozhi->addSkill(new Ganzhen);

    General *zhangjunyi = new General(this, "zhangjunyi", "wei");
    zhangjunyi->addSkill(new Jueji);
    zhangjunyi->addSkill(new JuejiClear);

    General *lukang = new General(this, "lukang", "wu", 3);
    lukang->addSkill(new Qianxun);
    lukang->addSkill(new Fanji);
    lukang->addSkill(new FanjiGet);

    General *jinxuandi = new General(this, "jinxuandi", "god");
    jinxuandi->addSkill(new Wuling);
    jinxuandi->addSkill(new WulingEffect);
    jinxuandi->addSkill(new WulingExEffect);

    General *xiahoujuan = new General(this, "xiahoujuan", "wei", 3, false);
    xiahoujuan->addSkill(new LianliStart);
    xiahoujuan->addSkill(new Lianli);
    xiahoujuan->addSkill(new LianliSlash);
    xiahoujuan->addSkill(new LianliJink);
    xiahoujuan->addSkill(new LianliClear);
    xiahoujuan->addSkill(new Tongxin);
    xiahoujuan->addSkill(new Skill("liqian", Skill::Compulsory));
    xiahoujuan->addSkill(new Qiaocai);

    General *caizhaoji = new General(this, "caizhaoji", "qun", 3, false);
    caizhaoji->addSkill(new Guihan);
    caizhaoji->addSkill(new CaizhaojiHujia);

    General *luboyan = new General(this, "luboyan", "wu", 3);
    luboyan->addSkill(new Shenjun);
    luboyan->addSkill(new Shaoying);
    luboyan->addSkill(new Zonghuo);

    General *luboyanf = new General(this, "luboyanf", "wu", 3, false, true);
    luboyanf->addSkill(new Shenjun);
    luboyanf->addSkill(new Shaoying);
    luboyanf->addSkill(new Zonghuo);

    General *zhongshiji = new General(this, "zhongshiji", "wei");
    zhongshiji->addSkill(new Gongmou);
    zhongshiji->addSkill(new GongmouExchange);

    General *jiangboyue = new General(this, "jiangboyue", "shu");
    jiangboyue->addSkill(new Lexue);
    jiangboyue->addSkill(new Xunzhi);

    skills << new YitianSwordViewAsSkill << new LianliSlashViewAsSkill;

    addMetaObject<ChengxiangCard>();
    addMetaObject<JuejiCard>();
    addMetaObject<LianliCard>();
    addMetaObject<QiaocaiCard>();
    addMetaObject<LianliSlashCard>();
    addMetaObject<GuihanCard>();
    addMetaObject<LexueCard>();
    addMetaObject<XunzhiCard>();
}

ADD_PACKAGE(Yitian);
