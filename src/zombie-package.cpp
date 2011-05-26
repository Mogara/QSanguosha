#include "zombie-package.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"
#include "thicket.h"
#include "standard-commons.h"

class Zaibian: public TriggerSkill{
public:
    Zaibian():TriggerSkill("zaibian"){
        events << DrawNCards << PhaseChange;
        frequency = Compulsory;
    }

    int getNumDiff(ServerPlayer *zombie) const{

        int size=0;
        Room *room = zombie->getRoom();
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            if(p->getRoleEnum()==Player::Rebel)size--;
            else size++;
        }
        size++;
        if(size<0)size=0;
        return size;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zombie, QVariant &data) const{
        if(event == PhaseChange && zombie->getPhase() == Player::Play){
            int x = getNumDiff(zombie);
            data = data.toInt() + x;

            Room *room = zombie->getRoom();
            LogMessage log;
            log.type = "#ZaibianGood";
            log.from = zombie;
            log.arg = QString::number(x);
            room->sendLog(log);

            zombie->drawCards(x);

        }/*else if(event == PhaseChange && zombie->getPhase() == Player::Discard){
            int x = getNumDiff(zombie);
            int total = zombie->getEquips().length() + zombie->getHandcardNum();
            Room *room = zombie->getRoom();

            if(total <= x){
                zombie->throwAllHandCards();
                zombie->throwAllEquips();

                LogMessage log;
                log.type = "#ZaibianWorst";
                log.from = zombie;
                log.arg = QString::number(total);
                room->sendLog(log);

            }else{

                room->askForDiscard(zombie, "zaibian", x, false, true);

                LogMessage log;
                log.type = "#ZaibianBad";
                log.from = zombie;
                log.arg = QString::number(x);
                room->sendLog(log);
            }
        }*/

        return false;
    }
};

class Xunmeng: public TriggerSkill{
public:
    Xunmeng():TriggerSkill("xunmeng"){
        events << Predamage;

        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zombie, QVariant &data) const{

            DamageStruct damage = data.value<DamageStruct>();

            const Card *reason = damage.card;
            if(reason == NULL)
                return false;

            if(reason->inherits("Slash")){
                LogMessage log;
                log.type = "#Xunmeng";
                log.from = zombie;
                log.to << damage.to;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(damage.damage + 1);
                zombie->getRoom()->sendLog(log);

                if(zombie->getHp()>2)zombie->getRoom()->loseHp(zombie);
                damage.damage ++;
                data = QVariant::fromValue(damage);
            }


        return false;
    }
};

PeachingCard::PeachingCard()
    :QingnangCard()
{

}

bool PeachingCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(targets.length() > 0)return false;
    return to_select->isWounded() && (Self->distanceTo(to_select) <= 1);
}

class Peaching: public OneCardViewAsSkill{
public:
    Peaching():OneCardViewAsSkill("peaching"){

    }

    virtual bool isEnabledAtPlay() const{
        return true;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Peach");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        PeachingCard *qingnang_card = new PeachingCard;
        qingnang_card->addSubcard(card_item->getCard()->getId());

        return qingnang_card;
    }
};

GanranEquip::GanranEquip(Card::Suit suit, int number)
    :IronChain(suit, number)
{

}

bool GanranEquip::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return false;
}

class Ganran: public FilterSkill{
public:
    Ganran():FilterSkill("ganran"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getTypeId() == Card::Equip;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        GanranEquip *ironchain = new GanranEquip(card->getSuit(), card->getNumber());
        ironchain->addSubcard(card_item->getCard()->getId());
        ironchain->setSkillName(objectName());

        return ironchain;
    }
};

ZombiePackage::ZombiePackage()
    :Package("zombies")
{
    General *zombie = new General(this,"zombie","zombies",3);
    zombie->addSkill(new Xunmeng);
    zombie->addSkill(new Skill("yicong", Skill::Compulsory));
    zombie->addSkill(new Skill("paoxiao"));
    zombie->addSkill(new Ganran);
    zombie->addSkill(new Zaibian);
    zombie->addSkill(new Skill("wansha", Skill::Compulsory));

    addMetaObject<PeachingCard>();
}

ADD_PACKAGE(Zombie);
