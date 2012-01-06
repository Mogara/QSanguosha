#include "bgm-package.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"

class ChongZhen: public TriggerSkill{
public:
    ChongZhen(): TriggerSkill("chongzhen"){
        events << CardResponsed << SlashEffect << CardEffected << CardFinished;
    }

    virtual int getPriority() const{
        return 3;
    }

    void doChongZhen(ServerPlayer *player, const Card *card) const{
        if(card->getSkillName() != "longdan")
            return;

        Room *room = player->getRoom();

        ServerPlayer *target = player->tag["ChongZhenTarget"].value<PlayerStar>();
        if(!target || target->isKongcheng() || !room->askForSkillInvoke(player, objectName()))
            return;

        int card_id = room->askForCardChosen(player, target, "h", objectName());
        room->moveCardTo(Sanguosha->getCard(card_id), player, Player::Hand, false);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == CardFinished){
            player->tag["ChongZhenTarget"] = QVariant::fromValue(NULL);
        }
        else if(event == CardResponsed){
            CardStar card = data.value<CardStar>();
            doChongZhen(player, card);
        }
        else if(event == CardEffected){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card->inherits("Duel")
                    || effect.card->inherits("ArcheryAttack")
                    || effect.card->inherits("SavageAssault")
                    || effect.card->inherits("Slash"))
                player->tag["ChongZhenTarget"] = QVariant::fromValue(effect.from);

            doChongZhen(player, effect.card);
        }
        else{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            player->tag["ChongZhenTarget"] = QVariant::fromValue(effect.to);
            doChongZhen(player, effect.slash);
        }

        return false;
    }
};

LihunCard::LihunCard(){
}

bool LihunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!to_select->getGeneral()->isMale())
        return false;

    if(!targets.isEmpty())
        return false;

    return true;
}

void LihunCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->throwCard(this);
    effect.from->turnOver();
    foreach(const Card *cd, effect.to->getHandcards()){
        room->moveCardTo(cd, effect.from, Player::Hand, false);
    }
    room->setTag("Lihun_Target", QVariant::fromValue(effect.to));
}

class LihunSelect: public OneCardViewAsSkill{
public:
    LihunSelect():OneCardViewAsSkill("lihun"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("LihunCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new LihunCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Lihun: public TriggerSkill{
public:
    Lihun():TriggerSkill("lihun"){
        events << PhaseChange;
        view_as_skill = new LihunSelect;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasUsed("LihunCard");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *diaochan, QVariant &data) const{
        Room *room = diaochan->getRoom();

        if(event == PhaseChange && diaochan->getPhase() == Player::Discard){
            ServerPlayer *target = room->getTag("Lihun_Target").value<PlayerStar>();
            if(diaochan->getCards("he").length() <= target->getHp()){
                foreach(const Card *card, diaochan->getCards("he")){
                    room->moveCardTo(card,
                                     target,
                                     Player::Hand,
                                     room->getCardPlace(card->getEffectiveId()) == Player::Hand ? false : true);
                }
            }
            else{
                int i;
                for(i = 0; i < target->getHp(); i++){
                    if(diaochan->isNude())
                        return false;

                    int card_id = room->askForCardChosen(diaochan, diaochan, "he", objectName());
                    const Card *card = Sanguosha->getCard(card_id);
                    room->moveCardTo(card,
                                     target,
                                     Player::Hand,
                                     room->getCardPlace(card_id) == Player::Hand ? false : true);
                }
            }
        }

        return false;
    }
};

BGMPackage::BGMPackage():Package("BGM"){
    General *bgm_zhaoyun = new General(this, "bgm_zhaoyun", "qun", 3, true, true);
    bgm_zhaoyun->addSkill("longdan");
    bgm_zhaoyun->addSkill(new ChongZhen);

    General *bgm_diaochan = new General(this, "bgm_diaochan", "qun", 3, false, true);
    bgm_diaochan->addSkill(new Lihun);
    bgm_diaochan->addSkill("biyue");

    addMetaObject<LihunCard>();
}

ADD_PACKAGE(BGM)
