#include "yitianpackage.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "god.h"

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

        ServerPlayer *target = room->askForPlayerChosen(move.from, room->getAllPlayers());
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
            ServerPlayer *to_modify = room->askForPlayerChosen(shencc, room->getOtherPlayers(shencc));
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

class Plum: public DelayedTrick{
public:
    Plum(Suit suit, int number):DelayedTrick(suit, number){
        setObjectName("plum");
    }

    virtual bool isAvailable() const{
        return Self->containsTrick(objectName());
    }

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
        room->recover(source, 2);

        room->moveCardTo(this, source, Player::Judging);
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
}

bool JuejiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.length() < 3 && to_select != Self;
}

void JuejiCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->gainMark("@jueji");
}

class JuejiViewAsSkill: public ViewAsSkill{
public:
    JuejiViewAsSkill():ViewAsSkill("jueji"){

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

class JileiClear: public PhaseChangeSkill{
public:
    JileiClear():PhaseChangeSkill("#jilei-clear"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *player, players){
                if(player->hasFlag("jilei")){

                    player->setFlags("-jilei");
                    player->setFlags("-jileiB");
                    player->setFlags("-jileiE");
                    player->setFlags("-jileiT");

                    player->invoke("jilei");
                }
            }
        }

        return false;
    }
};

class Jilei: public TriggerSkill{
public:
    Jilei():TriggerSkill("jilei"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yangxiu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from == NULL)
           return false;

        Room *room = yangxiu->getRoom();
        if(room->askForSkillInvoke(yangxiu, objectName(), data)){
            QString choice = room->askForChoice(yangxiu, objectName(), "basic+equip+trick");
            room->playSkillEffect(objectName());

            QString jilei_flag = choice[0].toUpper();
            damage.from->invoke("jilei", jilei_flag);

            damage.from->setFlags("jilei");
            damage.from->setFlags("jilei" + jilei_flag);

            LogMessage log;
            log.type = "#Jilei";
            log.from = yangxiu;
            log.to << damage.from;
            log.arg = choice;
            room->sendLog(log);
        }

        return false;
    }
};

class Danlao: public TriggerSkill{
public:
    Danlao():TriggerSkill("danlao"){
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.multiple && effect.card->inherits("TrickCard")){
            Room *room = player->getRoom();
            if(room->askForSkillInvoke(player, objectName(), data)){
                room->playSkillEffect(objectName());

                LogMessage log;

                log.type = "#DanlaoAvoid";
                log.from = player;
                log.arg = effect.card->objectName();

                room->sendLog(log);

                player->drawCards(1);
                return true;
            }
        }

        return false;
    }
};

YitianPackage::YitianPackage()
    :Package("yitian")
{

    (new YitianSword)->setParent(this);

    t["#AcquireSkill"] = tr("#AcquireSkill");
    t["#ChangeKingdom"] = tr("#ChangeKingdom");
    t["#Jilei"] = tr("#Jilei");
    t["#DanlaoAvoid"] = tr("#DanlaoAvoid");

    t["yitian"] = tr("yitian");
    t["yitian_sword"] = tr("yitian_sword");
    t["jilei:basic"] = tr("basic");
    t["jilei:equip"] = tr("equip");
    t["jilei:trick"] = tr("trick");

    // generals
    General *shencc = new General(this, "shencc$", "god", 3);
    shencc->addSkill(new Guixin2);
    shencc->addSkill(new Feiying);

    General *caochong = new General(this, "caochong", "wei", 3);
    caochong->addSkill(new Chengxiang);
    caochong->addSkill(new Conghui);
    caochong->addSkill(new Zaoyao);

    General *yangxiu = new General(this, "yangxiu", "wei", 3);
    yangxiu->addSkill(new Jilei);
    yangxiu->addSkill(new JileiClear);
    yangxiu->addSkill(new Danlao);

    //General *caozhi = new General(this, "caozhi", "wei", 3);
    //caozhi->addSkill(new Ganzhen);

    General *zhangjunyi = new General(this, "zhangjunyi", "wei");
    zhangjunyi->addSkill(new Jueji);
    zhangjunyi->addSkill(new JuejiClear);

    t["shencc"] = tr("shencc");
    t["caochong"] = tr("caochong");
    t["caozhi"] = tr("caozhi");
    t["zhangjunyi"] = tr("zhangjunyi");
    t["yangxiu"] = tr("yangxiu");

    t["guixin2"] = tr("guixin2");
    t["chengxiang"] = tr("chengxiang");
    t["conghui"] = tr("conghui");
    t["zaoyao"] = tr("zaoyao");
    t["ganzhen"] = tr("ganzhen");
    t["jueji"] = tr("jueji");
    t["jilei"] = tr("jilei");
    t["danlao"] = tr("danlao");

    t[":guixin2"] = tr(":guixin2");
    t[":chengxiang"] = tr(":chengxiang");
    t[":conghui"] = tr(":conghui");
    t[":zaoyao"] = tr(":zaoyao");
    t[":ganzhen"] = tr(":ganzhen");
    t[":jueji"] = tr(":jueji");
    t[":jilei"] = tr(":jilei");
    t[":danlao"] = tr(":danlao");

    t["@jueji"] = t["jueji"];

    t["guixin2:yes"] = tr("guixin2:yes");
    t["guixin2:modify"] = tr("guixin2:modify");
    t["guixin2:obtain"] = tr("guixin2:obtain");

    t["guixin2:hujia"] = tr("guixin2:hujia");
    t["guixin2:jijiang"] = tr("guixin2:jijiang");
    t["guixin2:jiuyuan"] = tr("guixin2:jiuyuan");
    t["guixin2:huangtian"] = tr("guixin2:huangtian");
    t["guixin2:xueyi"] = tr("guixin2:xueyi");
    t["guixin2:baonue"] = tr("guixin2:baonue");
    t["guixin2:songwei"] = tr("guixin2:songwei");

    t[":yitian_sword"] = tr(":yitian_sword");
    t["yitian_sword:yes"] = tr("yitian_sword:yes");

    t["@chengxiang-card"] = tr("@chengxiang-card");

    t["$jilei"] = tr("$jilei");
    t["$danlao"] = tr("$danlao");

    skills << new YitianSwordViewAsSkill;

    addMetaObject<ChengxiangCard>();
    addMetaObject<JuejiCard>();
}

ADD_PACKAGE(Yitian);
