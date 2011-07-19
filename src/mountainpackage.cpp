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
        if(damage.card == NULL || !damage.card->inherits("Slash"))
            return false;

        Room *room = player->getRoom();
        ServerPlayer *caiwenji = room->findPlayerBySkillName(objectName());
        if(caiwenji && !caiwenji->isKongcheng() && caiwenji->askForSkillInvoke(objectName(), data)){
            room->askForDiscard(caiwenji, "beige", 1);

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(.*):(.*)");
            judge.good = true;
            judge.who = player;

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
                    if(damage.from){
                        int to_discard = qMin(2, damage.from->getCardCount(true));
                        if(to_discard != 0)
                            room->askForDiscard(damage.from, "beige", to_discard, false, true);
                    }

                    break;
                }

            case Card::Spade:{
                    if(damage.from)
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

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();

        if(damage && damage->from){
            Room *room = player->getRoom();

            int max_hp = damage->from->getMaxHP();
            int hp = damage->from->getHp();
            QString to_transfigure = damage->from->getGeneral()->isMale() ? "sujiang" : "sujiangf";
            room->transfigure(damage->from, to_transfigure, false, false);
            if(damage->from->getGeneral2())
                room->setPlayerProperty(damage->from, "general2", to_transfigure);

            if(max_hp != damage->from->getMaxHP())
                room->setPlayerProperty(damage->from, "maxhp", max_hp);

            if(hp != damage->from->getHp())
                room->setPlayerProperty(damage->from, "hp", hp);
        }

        return false;
    }
};

MountainPackage::MountainPackage()
    :Package("mountain")
{
    General *zhanghe = new General(this, "zhanghe", "wei");
    zhanghe->addSkill(new Qiaobian);

    General *caiwenji = new General(this, "caiwenji", "qun", 3, false);
    caiwenji->addSkill(new Beige);
    caiwenji->addSkill(new Duanchang);

    addMetaObject<QiaobianCard>();
}

ADD_PACKAGE(Mountain);
