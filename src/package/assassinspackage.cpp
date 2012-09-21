#include "assassinspackage.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"

class MouKui: public TriggerSkill{
public:
    MouKui(): TriggerSkill("moukui"){
        events << TargetConfirmed;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        return false;
    }
};

class TianMing: public TriggerSkill{
public:
    TianMing(): TriggerSkill("tianming"){
        events << TargetConfirming;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card && use.card->isKindOf("Slash") && player->askForSkillInvoke(objectName())){
            if(!player->isNude()){
                int total = 0;
                QSet<const Card *> jilei_cards;
                QList<const Card *> handcards = player->getHandcards();
                foreach(const Card *card, handcards){
                    if(player->isJilei(card))
                        jilei_cards << card;
                }
                total = handcards.size() - jilei_cards.size() + player->getEquips().length();

                if(total <= 2)
                    player->throwAllHandCardsAndEquips();
                else
                    room->askForDiscard(player, objectName(), 2, 2, false, true);
            }

            player->drawCards(2);

            int max = 0;
            foreach(ServerPlayer *p, room->getAlivePlayers())
                if(p->getHp() > max)
                    max = p->getHp();

            QList<ServerPlayer *> maxs;
            foreach(ServerPlayer *p, room->getAlivePlayers()){
                if(p->getHp() == max)
                    maxs << p;
                if(maxs.size() != 1)
                    return false;
                else if(maxs.first() == player)
                    return false;
                else{
                    ServerPlayer *target = maxs.first();
                    QSet<const Card *> jilei_cards;
                    QList<const Card *> handcards = target->getHandcards();
                    foreach(const Card *card, handcards){
                        if(target->isJilei(card))
                            jilei_cards << card;
                    }
                    int total = handcards.size() - jilei_cards.size() + target->getEquips().length();

                    if(total <= 2)
                        target->throwAllHandCardsAndEquips();
                    else
                        room->askForDiscard(target, objectName(), 2, 2, false, true);
                }
            }
        }

        return false;
    }
};

MizhaoCard::MizhaoCard(){
	will_throw=false;
}

bool MizhaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return(to_select != Self);
}

void MizhaoCard::onEffect(const CardEffectStruct &effect) const{
	effect.to->obtainCard(effect.card, false);
	Room* room=effect.from->getRoom();

	ServerPlayer *target;
	QList<ServerPlayer *> targets;
	foreach(ServerPlayer *p, room->getOtherPlayers(effect.to))
		if(!p->isKongcheng())
			targets << p;

	if(!effect.to->isKongcheng())
		target = room->askForPlayerChosen(effect.from, targets, "mizhao");
	
	if(target){
		bool win = effect.to->pindian(target, "mizhao", NULL);
		Slash *slash = new Slash(Card::NoSuit, 0);
		slash->setSkillName("mizhao");
		if(win){
			CardUseStruct use;
			use.card = slash;
			use.from = effect.to;
			use.to << target;
			room->useCard(use);
		}else{
			CardUseStruct use;
			use.card = slash;
			use.from = target;
			use.to << effect.to;
			room->useCard(use);
		}
	}
}

class MiZhao: public ViewAsSkill{
public:
    MiZhao():ViewAsSkill("mizhao"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return(!player->isKongcheng() && !player->hasUsed("MizhaoCard"));
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() < Self->getHandcardNum())
            return NULL;

		MizhaoCard *card = new MizhaoCard;
		card->addSubcards(cards);
        return card;

        return NULL;
    }
};

class JieYuan: public TriggerSkill{
public:
    JieYuan(): TriggerSkill("jieyuan"){
        events << DamageCaused << DamageInflicted;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(triggerEvent == DamageCaused){
            if(damage.to->getHp() >= player->getHp())
                if(player->askForSkillInvoke(objectName()) && room->askForCard(player, ".black", "@JieYuanIncrease", QVariant(), CardDiscarded)){
                    damage.damage ++;
                    data = QVariant::fromValue(damage);
                }
        }else if(triggerEvent == DamageInflicted){
            if(damage.from->getHp() >= player->getHp())
                if(player->askForSkillInvoke(objectName()) && room->askForCard(player, ".red", "@JieYuanDecrease", QVariant(), CardDiscarded)){
                    damage.damage --;
                    data = QVariant::fromValue(damage);
                }
        }

        return false;
    }
};

class FenXin: public TriggerSkill{
public:
    FenXin(): TriggerSkill("fenxin"){
        events << Death;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        return false;
    }
};

AssassinsPackage::AssassinsPackage():Package("assassins"){
    General *wujiangjia = new General(this, "wujiangjia", "qun", 4);
    wujiangjia->addSkill(new MouKui);

    General *wujiangyi = new General(this, "wujiangyi", "qun", 3);
    wujiangyi->addSkill(new TianMing);
    wujiangyi->addSkill(new MiZhao);

    General *wujiangbing = new General(this, "wujiangbing", "qun", 3, false);
    wujiangbing->addSkill(new JieYuan);
    wujiangbing->addSkill(new FenXin);

    addMetaObject<MizhaoCard>();
}

ADD_PACKAGE(Assassins)
