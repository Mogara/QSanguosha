#include "bgm-package.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"

class ChongZheng: public TriggerSkill{
public:
    ChongZheng(): TriggerSkill("chongzheng"){
        events << CardResponsed << CardEffect << CardEffected;
    }

    virtual int getPriority() const{
        return 4;
    }

    void doChongZheng(ServerPlayer *player, const Card *card) const{
        if(card->getSkillName() != "longdan")
            return;

        Room *room = player->getRoom();

        ServerPlayer *target = player->tag["ChongZhengTarget"].value<PlayerStar>();
        if(!target || target->isKongcheng() || !room->askForSkillInvoke(player, objectName()))
            return;

        int card_id = room->askForCardChosen(player, target, "h", objectName());
        room->moveCardTo(Sanguosha->getCard(card_id), player, Player::Hand, false);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == CardResponsed){
            CardStar card = data.value<CardStar>();
            doChongZheng(player, card);
        }
        else{
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(event == CardEffected)
                player->tag["ChongZhengTarget"] = QVariant::fromValue(effect.from);
            else
                player->tag["ChongZhengTarget"] = QVariant::fromValue(effect.to);

            doChongZheng(player, effect.card);
        }

        return false;
    }
};

BGMPackage::BGMPackage():Package("BGM"){
    General *sp_zhaoyun = new General(this, "sp_zhaoyun", "qun", 3, true, true);
    sp_zhaoyun->addSkill("longdan");
    sp_zhaoyun->addSkill(new ChongZheng);
}

ADD_PACKAGE(BGM)
