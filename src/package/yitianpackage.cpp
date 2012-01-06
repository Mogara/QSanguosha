#include "yitianpackage.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "god.h"
#include "standard.h"

class YitianSwordSkill : public WeaponSkill{
public:
    YitianSwordSkill():WeaponSkill("yitian_sword"){
        events << DamageComplete;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        if(player->getPhase() != Player::NotActive)
           return false;

        if(player->askForSkillInvoke("yitian_sword"))
            player->getRoom()->askForUseCard(player, "slash", "@askforslash");

        return false;
    }
};

YitianSword::YitianSword(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("yitian_sword");
    skill = new YitianSwordSkill;
}

void YitianSword::onMove(const CardMoveStruct &move) const{
    if(move.from_place == Player::Equip && move.from->isAlive()){
        Room *room = move.from->getRoom();

        bool invoke = move.from->askForSkillInvoke("yitian-lost");
        if(!invoke)
            return;

        ServerPlayer *target = room->askForPlayerChosen(move.from, room->getAllPlayers(), "yitian-lost");
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

bool ChengxiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() < subcardsLength() && to_select->isWounded();
}

bool ChengxiangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() <= subcardsLength();
}

void ChengxiangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.isEmpty()){
        QList<ServerPlayer *> to;
        to << source;
        SkillCard::use(room, source, to);
    }else
        SkillCard::use(room, source, targets);
}

void ChengxiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    if(effect.to->isWounded()){
        RecoverStruct recover;
        recover.card = this;
        recover.who = effect.from;
        room->recover(effect.to, recover);
    }else
        effect.to->drawCards(2);
}

class ChengxiangViewAsSkill: public ViewAsSkill{
public:
    ChengxiangViewAsSkill():ViewAsSkill("chengxiang"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@chengxiang";
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

        if(caochong->isNude())
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

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Discard;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *) const{
        return true;
    }
};

class Zaoyao: public PhaseChangeSkill{
public:
    Zaoyao():PhaseChangeSkill("zaoyao"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *caochong) const{
        if(caochong->getPhase() == Player::Finish && caochong->getHandcardNum() > 13){
            caochong->throwAllHandCards();
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
            PlayerStar to_modify = room->askForPlayerChosen(shencc, room->getOtherPlayers(shencc), objectName());
            room->setTag("Guixin2Modify", QVariant::fromValue(to_modify));
            QString kingdom = room->askForChoice(shencc, "guixin2", "wei+shu+wu+qun");
            room->removeTag("Guixin2Modify");
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

JuejiCard::JuejiCard(){
    once = true;
}

bool JuejiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng();
}

void JuejiCard::onEffect(const CardEffectStruct &effect) const{
    bool success = effect.from->pindian(effect.to, "jueji", this);

    PlayerStar to = effect.to;
    QVariant data = QVariant::fromValue(to);

    while(success && !effect.to->isKongcheng()){
        if(effect.from->isKongcheng() || !effect.from->askForSkillInvoke("jueji", data))
            break;

        success = effect.from->pindian(effect.to, "jueji");
    }
}

class JuejiViewAsSkill: public OneCardViewAsSkill{
public:
    JuejiViewAsSkill():OneCardViewAsSkill("jueji"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        JuejiCard *card = new JuejiCard;
        card->addSubcard(card_item->getCard());
        return card;
    }
};

class Jueji: public PhaseChangeSkill{
public:
    Jueji():PhaseChangeSkill("jueji"){
        view_as_skill = new JuejiViewAsSkill;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Play){
            Room *room = target->getRoom();
            return room->askForUseCard(target, "@@jueji", "@jueji-pindian");
        }else
            return false;
    }
};

class JuejiGet: public TriggerSkill{
public:
    JuejiGet():TriggerSkill("#jueji-get"){
        events << Pindian;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        if(pindian->reason == "jueji" && pindian->isSuccess()){
            player->obtainCard(pindian->to_card);
        }

        return false;
    }
};

class LukangWeiyan: public PhaseChangeSkill{
public:
    LukangWeiyan():PhaseChangeSkill("lukang_weiyan"){
    }

    virtual int getPriority() const{
        return 4; // very high priority
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        switch(target->getPhase()){
        case Player::Draw: {
                if(target->askForSkillInvoke("lukang_weiyan", "draw2play")){
                    target->getRoom()->setPlayerProperty(target, "phase", "play");
                }

                break;
            }

        case Player::Play:{
                if(target->askForSkillInvoke("lukang_weiyan", "play2draw")){
                    target->getRoom()->setPlayerProperty(target, "phase", "draw");
                }

                break;
            }

        default:
            return false;
        }

        return false;
    }
};

class Kegou: public PhaseChangeSkill{
public:
    Kegou():PhaseChangeSkill("kegou"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("kegou") == 0
                && target->getKingdom() == "wu"
                && !target->isLord();
    }

    virtual bool onPhaseChange(ServerPlayer *lukang) const{
        foreach(const Player *player, lukang->getSiblings()){
            if(player->isAlive() && player->getKingdom() == "wu"
               && !player->isLord() && player != lukang){
                return false;
            }
        }

        lukang->setMark("kegou", 1);


        Room *room = lukang->getRoom();

        LogMessage log;
        log.type = "#KegouWake";
        log.from = lukang;
        room->sendLog(log);

        room->loseMaxHp(lukang);
        room->acquireSkill(lukang, "lianying");

        return false;
    }
};

// ---------- Lianli related skills

LianliCard::LianliCard(){

}

bool LianliCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
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

bool LianliSlashCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
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

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@tied") > 0 && Slash::IsAvailable(player);
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

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@lianli";
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

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
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
    QList<const Card *> cards = effect.to->getJudgingArea();
    foreach(const Card *card, cards){
        effect.from->obtainCard(card);
    }
 }

class Qiaocai: public ZeroCardViewAsSkill{
public:
    Qiaocai():ZeroCardViewAsSkill("qiaocai"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@tied") == 0 && ! player->hasUsed("QiaocaiCard");
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
                RecoverStruct recover;
                recover.card = effect.card;
                recover.who = effect.from;
                room->recover(player, recover);

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

            room->playSkillEffect(objectName(), effects.indexOf(choice) + 1);
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

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("GuihanCard");
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

class CaizhaojiHujia: public TriggerSkill{
public:
    CaizhaojiHujia():TriggerSkill("caizhaoji_hujia"){
        events << PhaseChange << FinishJudge;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *caizhaoji, QVariant &data) const{
        if(event == PhaseChange && caizhaoji->getPhase() == Player::Finish){
            int times = 0;
            Room *room = caizhaoji->getRoom();
            while(caizhaoji->askForSkillInvoke(objectName())){
                caizhaoji->setFlags("caizhaoji_hujia");

                room->playSkillEffect(objectName());

                times ++;
                if(times == 3){
                    caizhaoji->turnOver();
                }

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = caizhaoji;

                room->judge(judge);

                if(judge.isBad())
                    break;
            }

            caizhaoji->setFlags("-caizhaoji_hujia");
        }else if(event == FinishJudge){
            if(caizhaoji->hasFlag("caizhaoji_hujia")){
                JudgeStar judge = data.value<JudgeStar>();
                if(judge->card->isRed()){
                    caizhaoji->obtainCard(judge->card);
                    return true;
                }
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

    virtual QString getDefaultChoice(ServerPlayer *player) const{
        int males = 0;
        foreach(ServerPlayer *player, player->getRoom()->getAlivePlayers()){
            if(player->getGender() == General::Male)
                males ++;
        }

        if(males > (player->aliveCount() - males))
            return "female";
        else
            return "male";
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == GameStart){
            if(player->getGeneral2Name().startsWith("luboyan")){
                room->setPlayerProperty(player, "general2", player->getGeneralName());
                room->setPlayerProperty(player, "general", "luboyan");
            }

            QString gender = room->askForChoice(player, objectName(), "male+female");
            bool is_male = player->getGeneral()->isMale();
            if(gender == "female"){
                if(is_male)
                    room->transfigure(player, "luboyanf", false, false, "luboyan");
            }else if(gender == "male"){
                if(!is_male)
                    room->transfigure(player, "luboyan", false, false, "luboyanf");
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
                QString old_general = new_general.endsWith("f")?"luboyan":"luboyanf";
                room->transfigure(player, new_general, false, false, old_general);
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

class Shaoying: public TriggerSkill{
public:
    Shaoying():TriggerSkill("shaoying"){
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
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

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = luboyan;

            room->judge(judge);

            if(judge.isGood()){
                DamageStruct shaoying_damage;
                shaoying_damage.nature = DamageStruct::Fire;
                shaoying_damage.from = luboyan;
                shaoying_damage.to = player->getNextAlive();

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
    mute = true;
}

bool LexueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void LexueCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    room->playSkillEffect("lexue", 1);
    const Card *card = room->askForCardShow(effect.to, effect.from, "lexue");
    int card_id = card->getEffectiveId();
    room->showCard(effect.to, card_id);

    if(card->getTypeId() == Card::Basic || card->isNDTrick()){
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

    virtual int getEffectIndex(ServerPlayer *, const Card *card) const{
        if(card->getTypeId() == Card::Basic)
            return 2;
        else
            return 3;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->hasUsed("LexueCard") && player->hasFlag("lexue")){
            int card_id = player->getMark("lexue");
            const Card *card = Sanguosha->getCard(card_id);
            return card->isAvailable(player);
        }else
            return true;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if(player->getPhase() == Player::NotActive)
            return false;

        if(!player->hasFlag("lexue"))
            return false;

        if(player->hasUsed("LexueCard")){
            int card_id = player->getMark("lexue");
            const Card *card = Sanguosha->getCard(card_id);
            return  pattern.contains(card->objectName());
        }else
            return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(Self->hasUsed("LexueCard") && selected.isEmpty() && Self->hasFlag("lexue")){
            int card_id = Self->getMark("lexue");
            const Card *card = Sanguosha->getCard(card_id);
            return to_select->getFilteredCard()->getSuit() == card->getSuit();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(Self->hasUsed("LexueCard")){
            if(!Self->hasFlag("lexue"))
                return false;

            if(cards.length() != 1)
                return NULL;

            int card_id = Self->getMark("lexue");
            const Card *card = Sanguosha->getCard(card_id);
            const Card *first = cards.first()->getFilteredCard();

            Card *new_card = Sanguosha->cloneCard(card->objectName(), first->getSuit(), first->getNumber());
            new_card->addSubcards(cards);
            new_card->setSkillName(objectName());
            return new_card;
        }else{
            return new LexueCard;
        }
    }
};

XunzhiCard::XunzhiCard(){
    target_fixed = true;
}

void XunzhiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->drawCards(3);

    QList<ServerPlayer *> players = room->getAlivePlayers();
    QSet<QString> general_names;
    foreach(ServerPlayer *player, players)
        general_names << player->getGeneralName();

    QStringList all_generals = Sanguosha->getLimitedGeneralNames();
    QStringList shu_generals;
    foreach(QString name, all_generals){
        const General *general = Sanguosha->getGeneral(name);
        if(general->getKingdom() == "shu" && !general_names.contains(name))
            shu_generals << name;
    }

    QString general = room->askForGeneral(source, shu_generals);
    source->tag["newgeneral"] = general;
    room->transfigure(source, general, false, false, "jiangboyue");
    room->acquireSkill(source, "xunzhi", false);
    source->setFlags("xunzhi");
}

class XunzhiViewAsSkill: public ZeroCardViewAsSkill{
public:
    XunzhiViewAsSkill():ZeroCardViewAsSkill("#xunzhi"){

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
        if(target->getPhase() == Player::NotActive &&
           target->hasFlag("xunzhi"))
        {
            Room *room = target->getRoom();
            room->transfigure(target, parent()->objectName(), false, false, target->tag.value("newgeneral", "").toString());
            room->killPlayer(target);
        }

        return false;
    }
};

class Dongcha: public PhaseChangeSkill{
public:
    Dongcha():PhaseChangeSkill("dongcha"){

    }

    virtual bool onPhaseChange(ServerPlayer *jiawenhe) const{
        switch(jiawenhe->getPhase()){
        case Player::Start:{
                if(jiawenhe->askForSkillInvoke(objectName())){
                    Room *room = jiawenhe->getRoom();
                    QList<ServerPlayer *> players = room->getOtherPlayers(jiawenhe);
                    ServerPlayer *dongchaee = room->askForPlayerChosen(jiawenhe, players, objectName());
                    room->setPlayerFlag(dongchaee, "dongchaee");
                    room->setTag("Dongchaee", dongchaee->objectName());
                    room->setTag("Dongchaer", jiawenhe->objectName());

                    room->showAllCards(dongchaee, jiawenhe);
                }

                break;
            }

        case Player::Finish:{
                Room *room = jiawenhe->getRoom();
                QString dongchaee_name = room->getTag("Dongchaee").toString();
                if(!dongchaee_name.isEmpty()){
                    ServerPlayer *dongchaee = room->findChild<ServerPlayer *>(dongchaee_name);
                    room->setPlayerFlag(dongchaee, "-dongchaee");

                    room->setTag("Dongchaee", QVariant());
                    room->setTag("Dongchaer", QVariant());
                }

                break;
            }

        default:
            break;
        }

        return false;
    }
};

class Dushi: public TriggerSkill{
public:
    Dushi():TriggerSkill("dushi"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;

        if(killer){
            Room *room = player->getRoom();
            if(killer != player && !killer->hasSkill("benghuai")){
                killer->gainMark("@collapse");
                room->acquireSkill(killer, "benghuai");
            }
        }

        return false;
    }
};

class Sizhan: public TriggerSkill{
public:
    Sizhan():TriggerSkill("sizhan"){
        events << Predamaged << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *elai, QVariant &data) const{
        if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();

            LogMessage log;
            log.type = "#SizhanPrevent";
            log.from = elai;
            log.arg = QString::number(damage.damage);
            elai->getRoom()->sendLog(log);

            elai->gainMark("@struggle", damage.damage);

            return true;
        }else if(event == PhaseChange && elai->getPhase() == Player::Finish){
            int x = elai->getMark("@struggle");
            if(x > 0){
                elai->loseMark("@struggle", x);

                LogMessage log;
                log.type = "#SizhanLoseHP";
                log.from = elai;
                log.arg = QString::number(x);

                Room *room = elai->getRoom();
                room->sendLog(log);
                room->loseHp(elai, x);
            }

            elai->setFlags("-shenli");
        }

        return false;
    }
};

class Shenli: public TriggerSkill{
public:
    Shenli():TriggerSkill("shenli"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *elai, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") &&
           elai->getPhase() == Player::Play && !elai->hasFlag("shenli"))
        {
            elai->setFlags("shenli");

            int x = elai->getMark("@struggle");
            if(x > 0){
                x = qMin(3, x);
                damage.damage += x;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#ShenliBuff";
                log.from = elai;
                log.arg = QString::number(x);
                log.arg2 = QString::number(damage.damage);
                elai->getRoom()->sendLog(log);
            }
        }

        return false;
    }
};

class Zhenggong: public TriggerSkill{
public:
    Zhenggong():TriggerSkill("zhenggong"){
        events << TurnStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return ! target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        ServerPlayer *dengshizai = room->findPlayerBySkillName(objectName());

        if(dengshizai && dengshizai->faceUp() && dengshizai->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());

            dengshizai->turnOver();

            PlayerStar zhenggong = room->getTag("Zhenggong").value<PlayerStar>();
            if(zhenggong == NULL){
                PlayerStar p = player;
                room->setTag("Zhenggong", QVariant::fromValue(p));
                player->gainMark("@zhenggong");
            }

            room->setCurrent(dengshizai);
            dengshizai->play();

            return true;

        }else{
            PlayerStar p = room->getTag("Zhenggong").value<PlayerStar>();
            if(p){
                p->loseMark("@zhenggong");
                room->setCurrent(p);
                room->setTag("Zhenggong", QVariant());
            }
        }

        return false;
    }
};

class Toudu: public MasochismSkill{
public:
    Toudu():MasochismSkill("toudu"){

    }

    virtual void onDamaged(ServerPlayer *dengshizai, const DamageStruct &) const{
        if(dengshizai->faceUp())
            return;

        if(dengshizai->isKongcheng())
            return;

        if(!dengshizai->askForSkillInvoke("toudu"))
            return;

        Room *room = dengshizai->getRoom();

        if(!room->askForDiscard(dengshizai, "toudu", 1, false, false))
            return;

        dengshizai->turnOver();

        QList<ServerPlayer *> players = room->getOtherPlayers(dengshizai), targets;
        foreach(ServerPlayer *player, players){
            if(dengshizai->canSlash(player, false)){
                targets << player;
            }
        }

        if(!targets.isEmpty()){
            ServerPlayer *target = room->askForPlayerChosen(dengshizai, targets, "toudu");

            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName("toudu");

            CardUseStruct use;
            use.card = slash;
            use.from = dengshizai;
            use.to << target;
            room->useCard(use);
        }
    }
};

YisheCard::YisheCard(){
    target_fixed = true;
    will_throw = false;
}

void YisheCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    const QList<int> rice = source->getPile("rice");

    if(subcards.isEmpty()){
        foreach(int card_id, rice){
            room->obtainCard(source, card_id);
        }
    }else{
        foreach(int card_id, subcards){
            source->addToPile("rice", card_id);
        }
    }
}

class YisheViewAsSkill: public ViewAsSkill{
public:
    YisheViewAsSkill():ViewAsSkill(""){
        card = new YisheCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->getPile("rice").isEmpty())
            return !player->isKongcheng();
        else
            return true;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        int n = Self->getPile("rice").length();
        if(selected.length() + n >= 5)
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(Self->getPile("rice").isEmpty() && cards.isEmpty())
            return NULL;

        card->clearSubcards();
        card->addSubcards(cards);
        return card;
    }

private:
    YisheCard *card;
};

YisheAskCard::YisheAskCard(){
    target_fixed = true;
}

void YisheAskCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    ServerPlayer *zhanglu = room->findPlayerBySkillName("yishe");
    if(zhanglu == NULL)
        return;

    const QList<int> &yishe = zhanglu->getPile("rice");
    if(yishe.isEmpty())
        return;

    int card_id;
    if(yishe.length() == 1)
        card_id = yishe.first();
    else{
        room->fillAG(yishe, source);
        card_id = room->askForAG(source, yishe, false, "yisheask");
        source->invoke("clearAG");
    }

    source->obtainCard(Sanguosha->getCard(card_id));
    room->showCard(source, card_id);

    if(room->askForChoice(zhanglu, "yisheask", "allow+disallow") == "disallow"){
        zhanglu->addToPile("rice", card_id);
    }
}

class YisheAsk: public ZeroCardViewAsSkill{
public:
    YisheAsk():ZeroCardViewAsSkill("yisheask"){
        default_choice = "disallow";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->hasSkill("yishe"))
            return false;

        if(player->usedTimes("YisheAskCard") >= 2)
            return false;

        Player *zhanglu = NULL;
        foreach(Player *p, player->parent()->findChildren<Player *>()){
            if(p->isAlive() && p->hasSkill("yishe")){
                zhanglu = p;
                break;
            }
        }

        return zhanglu && !zhanglu->getPile("rice").isEmpty();
    }

    virtual const Card *viewAs() const{
        return new YisheAskCard;
    }
};

class Yishe: public GameStartSkill{
public:
    Yishe():GameStartSkill("yishe"){
        view_as_skill = new YisheViewAsSkill;
    }

    virtual void onGameStart(ServerPlayer *player) const{
        Room *room = player->getRoom();
        foreach(ServerPlayer *p, room->getOtherPlayers(player))
            room->attachSkillToPlayer(p, "yisheask");
    }
};

class Xiliang: public TriggerSkill{
public:
    Xiliang():TriggerSkill("xiliang"){
        events << CardDiscarded;

        default_choice = "obtain";

        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(player->getPhase() != Player::Discard)
            return false;

        ServerPlayer *zhanglu = room->findPlayerBySkillName(objectName());

        if(zhanglu == NULL)
            return false;

        CardStar card = data.value<CardStar>();
        QList<const Card *> red_cards;
        foreach(int card_id, card->getSubcards()){
            const Card *c = Sanguosha->getCard(card_id);
            if(c->isRed())
                red_cards << c;
        }

        if(red_cards.isEmpty())
            return false;

        if(!zhanglu->askForSkillInvoke(objectName(), data))
            return false;

        bool can_put = 5 - zhanglu->getPile("rice").length() >= red_cards.length();
        if(can_put && room->askForChoice(zhanglu, objectName(), "put+obtain") == "put"){
            foreach(const Card *card, red_cards){
                zhanglu->addToPile("rice", card->getEffectiveId());
            }
        }else{
            foreach(const Card *card, red_cards){
                zhanglu->obtainCard(card);
            }
        }

        return false;
    }
};

class Zhenwei: public TriggerSkill{
public:
    Zhenwei():TriggerSkill("zhenwei"){
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(player->getRoom()->obtainable(effect.jink, player) && player->askForSkillInvoke(objectName(), data))
            player->obtainCard(effect.jink);

        return false;
    }
};

class Yitian: public TriggerSkill{
public:
    Yitian():TriggerSkill("yitian"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->isCaoCao() && player->askForSkillInvoke(objectName(), data)){
            LogMessage log;
            log.type = "#YitianSolace";
            log.from = player;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage - 1);
            player->getRoom()->sendLog(log);

            damage.damage --;
            data.setValue(damage);
        }

        return false;
    }
};

TaichenCard::TaichenCard(){
}

bool TaichenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty() || to_select->isAllNude())
        return false;

    if(!subcards.isEmpty() && Sanguosha->getCard(subcards.first()) == Self->getWeapon() && !Self->hasSkill("zhengfeng"))
        return Self->distanceTo(to_select) == 1;
    else
        return Self->inMyAttackRange(to_select);
}

void TaichenCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    if(subcards.isEmpty())
        room->loseHp(effect.from);
    else
        room->throwCard(this);

    int i;
    for(i=0; i<2; i++){
        if(!effect.to->isAllNude())
            room->throwCard(room->askForCardChosen(effect.from, effect.to, "hej", "taichen"));
    }
}

class Taichen: public ViewAsSkill{
public:
    Taichen():ViewAsSkill("taichen"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && to_select->getFilteredCard()->inherits("Weapon");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() <= 1){
            TaichenCard *taichen_card = new TaichenCard;
            taichen_card->addSubcards(cards);
            return taichen_card;
        }else
            return NULL;
    }
};

YitianCardPackage::YitianCardPackage()
    :Package("yitian_cards")
{
    (new YitianSword)->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(YitianCard)

YitianPackage::YitianPackage()
    :Package("yitian")
{
    // generals
    General *shencc = new General(this, "shencc", "god", 3);
    shencc->addSkill(new Guixin2);
    shencc->addSkill("feiying");

    General *caochong = new General(this, "caochong", "wei", 3);
    caochong->addSkill(new Chengxiang);
    caochong->addSkill(new Conghui);
    caochong->addSkill(new Zaoyao);

    General *zhangjunyi = new General(this, "zhangjunyi", "qun");
    zhangjunyi->addSkill(new Jueji);
    zhangjunyi->addSkill(new JuejiGet);

    related_skills.insertMulti("jueji", "#jueji-get");

    General *lukang = new General(this, "lukang", "wu", 4);
    lukang->addSkill(new LukangWeiyan);
    lukang->addSkill(new Kegou);

    General *jinxuandi = new General(this, "jinxuandi", "god");
    jinxuandi->addSkill(new Wuling);
    jinxuandi->addSkill(new WulingEffect);
    jinxuandi->addSkill(new WulingExEffect);

    related_skills.insertMulti("wuling", "#wuling-effect");
    related_skills.insertMulti("wuling", "#wuling-ex-effect");

    General *xiahoujuan = new General(this, "xiahoujuan", "wei", 3, false);
    xiahoujuan->addSkill(new LianliStart);
    xiahoujuan->addSkill(new Lianli);
    xiahoujuan->addSkill(new LianliSlash);
    xiahoujuan->addSkill(new LianliJink);
    xiahoujuan->addSkill(new LianliClear);
    xiahoujuan->addSkill(new Tongxin);
    xiahoujuan->addSkill(new Skill("liqian", Skill::Compulsory));
    xiahoujuan->addSkill(new Qiaocai);

    related_skills.insertMulti("lianli", "#lianli-start");
    related_skills.insertMulti("lianli", "#lianli-slash");
    related_skills.insertMulti("lianli", "#lianli-jink");
    related_skills.insertMulti("lianli", "#lianli-clear");

    General *caizhaoji = new General(this, "caizhaoji", "qun", 3, false);
    caizhaoji->addSkill(new Guihan);
    caizhaoji->addSkill(new CaizhaojiHujia);

    General *luboyan = new General(this, "luboyan", "wu", 3);
    luboyan->addSkill(new Shenjun);
    luboyan->addSkill(new Shaoying);
    luboyan->addSkill(new Zonghuo);

    General *luboyanf = new General(this, "luboyanf", "wu", 3, false, true);
    luboyanf->addSkill("shenjun");
    luboyanf->addSkill("shaoying");
    luboyanf->addSkill("zonghuo");

    General *zhongshiji = new General(this, "zhongshiji", "wei");
    zhongshiji->addSkill(new Gongmou);
    zhongshiji->addSkill(new GongmouExchange);

    related_skills.insertMulti("gongmou", "#gongmou-exchange");

    General *jiangboyue = new General(this, "jiangboyue", "shu");
    jiangboyue->addSkill(new Lexue);
    jiangboyue->addSkill(new Xunzhi);

    General *jiawenhe = new General(this, "jiawenhe", "qun");
    jiawenhe->addSkill(new Dongcha);
    jiawenhe->addSkill(new Dushi);

    General *elai = new General(this, "guzhielai", "wei");
    elai->addSkill(new Sizhan);
    elai->addSkill(new Shenli);

    General *dengshizai = new General(this, "dengshizai", "wei", 3);
    dengshizai->addSkill(new Zhenggong);
    dengshizai->addSkill(new Toudu);

    General *zhanggongqi = new General(this, "zhanggongqi", "qun", 3);
    zhanggongqi->addSkill(new Yishe);
    zhanggongqi->addSkill(new Xiliang);

    General *yitianjian = new General(this, "yitianjian", "wei");
    yitianjian->addSkill(new Skill("zhengfeng", Skill::Compulsory));
    yitianjian->addSkill(new Zhenwei);
    yitianjian->addSkill(new Yitian);

    General *sp_pangde = new General(this, "sp_pangde", "wei");
    sp_pangde->addSkill(new Taichen);

    skills << new LianliSlashViewAsSkill << new YisheAsk;

    addMetaObject<ChengxiangCard>();
    addMetaObject<JuejiCard>();
    addMetaObject<LianliCard>();
    addMetaObject<QiaocaiCard>();
    addMetaObject<LianliSlashCard>();
    addMetaObject<GuihanCard>();
    addMetaObject<LexueCard>();
    addMetaObject<XunzhiCard>();
    addMetaObject<YisheAskCard>();
    addMetaObject<YisheCard>();
    addMetaObject<TaichenCard>();
}

ADD_PACKAGE(Yitian);
