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
            player->tag["ChongZhenTarget"] = NULL;
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
    effect.from->setFlags("lihun_used");
    effect.from->turnOver();
    QList<const Card *> handcards = effect.to->getHandcards();
    foreach(const Card *cd, handcards){
        room->moveCardTo(cd, effect.from, Player::Hand, false);
    }
    room->setTag("Lihun_Target", QVariant::fromValue(effect.to));
}

class Lihun:public OneCardViewAsSkill{
public:
    Lihun():OneCardViewAsSkill("lihun"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("LihunCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new LihunCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class LihunGiveback: public TriggerSkill{
public:
    LihunGiveback():TriggerSkill("#lihungiveback"){
        events << PhaseChange;

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName()) && target->hasFlag("lihun_used");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *diaochan, QVariant &data) const{
        Room *room = diaochan->getRoom();

        if(event == PhaseChange && diaochan->getPhase() == Player::Discard){
            ServerPlayer *target = room->getTag("Lihun_Target").value<PlayerStar>();
            int x = target->getHp();
            QList<int> allcards_id;
            QList<const Card *> equips;
            QList<const Card *> handcards;
            if(diaochan->hasEquip()){
                equips = diaochan->getEquips();
                foreach(const Card *cd, equips){
                    int id = cd->getEffectiveId();
                    allcards_id << id;
                }
            }
            if(!diaochan->isKongcheng()){
                handcards = diaochan->getHandcards();
                foreach(const Card *cd, handcards){
                    int id = cd->getEffectiveId();
                    allcards_id << id;
                }
            }
            if(allcards_id.isEmpty())
                return false;
            else if(allcards_id.length() <= x){
                foreach(int id, allcards_id){
                    room->moveCardTo(Sanguosha->getCard(id), target, Player::Hand, false);
                }
            }else if(allcards_id.length() > x){
                int i;
                for(i = 0; i < x; i++){
                    room->fillAG(allcards_id, diaochan);
                    int selected_id = room->askForAG(diaochan, allcards_id, false, "lihungiveback");
                    room->moveCardTo(Sanguosha->getCard(selected_id), target, Player::Hand, false);
                    allcards_id.removeOne(selected_id);
                    diaochan->invoke("clearAG");
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

    General *bgm_diaochan = new General(this, "bgm_diaochan", "qun", 3, true, true);
    bgm_diaochan->addSkill(new Lihun);
    bgm_diaochan->addSkill(new LihunGiveback);
    bgm_diaochan->addSkill("biyue");

    related_skills.insertMulti("lihun", "#lihungiveback");

    addMetaObject<LihunCard>();
}

ADD_PACKAGE(BGM)
