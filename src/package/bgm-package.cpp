#include "bgm-package.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"

class ChongZhen: public TriggerSkill{
public:
    ChongZhen(): TriggerSkill("chongzhen"){
        events << CardResponsed << CardEffect << CardEffected << CardFinished;
    }

    virtual int getPriority() const{
        return 4;
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
        if(event == CardResponsed){
            CardStar card = data.value<CardStar>();
            doChongZhen(player, card);
        }
        else if(event == CardFinished){
            player->tag["ChongZhenTarget"] = NULL;
        }
        else{
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(event == CardEffected){
                if(effect.card->inherits("Duel")
                        || effect.card->inherits("ArcheryAttack")
                        || effect.card->inherits("SavageAssault")
                        || effect.card->inherits("Slash"))
                    player->tag["ChongZhenTarget"] = QVariant::fromValue(effect.from);
            }
            else{
                if(effect.to->hasFlag("liuli_target"))
                    return false;

                player->tag["ChongZhenTarget"] = QVariant::fromValue(effect.to);
            }

            doChongZhen(player, effect.card);
        }

        return false;
    }
};

BGMPackage::BGMPackage():Package("BGM"){
    General *sp_zhaoyun = new General(this, "sp_zhaoyun", "qun", 3, true, true);
    sp_zhaoyun->addSkill("longdan");
    sp_zhaoyun->addSkill(new ChongZhen);
}

ADD_PACKAGE(BGM)
