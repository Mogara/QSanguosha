#include "skill.h"
#include "carditem.h"
#include "engine.h"
#include "danchuang.h"
#include "standard.h"
#include "client.h"

class V5Zhenggong:public MasochismSkill{
public:
    V5Zhenggong():MasochismSkill("v5zhenggong"){
    }

    virtual void onDamaged(ServerPlayer *shiji, const DamageStruct &damage) const{
        if(shiji->getMark("v5baijiang") > 0)
            return;
        if(damage.from && damage.from->hasEquip() && shiji->askForSkillInvoke(objectName(), QVariant::fromValue((PlayerStar)damage.from))){
            Room *room = shiji->getRoom();
            int equip = room->askForCardChosen(shiji, damage.from, "e", objectName());
            const EquipCard *equipped = qobject_cast<const EquipCard *>(Sanguosha->getCard(equip));
            QList<ServerPlayer *> targets;
            targets << shiji;
            equipped->use(room, shiji, targets);
        }
    }
};

V5QuanjiCard::V5QuanjiCard(){
    target_fixed = true;
    will_throw = false;
}

void V5QuanjiCard::use(Room *room, ServerPlayer *zhonghui, const QList<ServerPlayer *> &) const{
    PlayerStar target = room->getCurrent();
    if(!target)
        return;
    bool success = zhonghui->pindian(target, skill_name, this);
    if(success)
        room->setPlayerFlag(target, "V5quanji");
}

class V5QuanjiViewAsSkill: public OneCardViewAsSkill{
public:
    V5QuanjiViewAsSkill():OneCardViewAsSkill("v5quanji"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@v5qj";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        V5QuanjiCard *card = new V5QuanjiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class V5Quanji: public PhaseChangeSkill{
public:
    V5Quanji():PhaseChangeSkill("v5quanji"){
        view_as_skill = new V5QuanjiViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::RoundStart)
            return false;
        Room *room = player->getRoom();
        PlayerStar zh = room->findPlayerBySkillName(objectName());
        if(!zh || zh == player || player->isKongcheng())
            return false;
        if(zh->getMark("v5baijiang") > 0)
            return false;
        if(room->askForUseCard(zh, "@@v5qj", "@v5qj:" + player->objectName())){
            if(player->hasFlag("V5quanji")){
                player->skip(Player::Start);
                player->skip(Player::Judge);
            }
        }
        return false;
    }
};

class V5Baijiang: public PhaseChangeSkill{
public:
    V5Baijiang():PhaseChangeSkill("v5baijiang"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Start
                && target->hasSkill(objectName())
                && target->isAlive()
                && target->getMark(objectName()) == 0;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Start)
            return false;
        int ec = player->getEquips().count();
        if(ec >= 3){
            Room *room = player->getRoom();
            room->playSkillEffect(objectName());
            room->broadcastInvoke("animate", "lightbox:$v5baijiang");

            LogMessage log;
            log.type = "#BaijiangWake";
            log.from = player;
            log.arg = QString::number(ec);
            log.arg2 = objectName();
            room->sendLog(log);

            room->setPlayerMark(player, objectName(), 1);
            room->setPlayerProperty(player, "maxhp", player->getMaxHP() + 1);

            room->detachSkillFromPlayer(player, "v5quanji");
            room->detachSkillFromPlayer(player, "v5zhenggong");
            room->acquireSkill(player, "v5yexin");
        }
        return false;
    }
};

V5YexinCard::V5YexinCard(){
    target_fixed = true;
}

void V5YexinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    const QList<int> &quan = source->getPile("werpo");
    if(quan.isEmpty() || source->isKongcheng())
        return;

    int leng = getSubcards().length();
    DummyCard *dummy = new DummyCard;
    int card_id = -1;
    if(quan.length() == 1)
        card_id = quan.first();
    else{
        room->fillAG(quan, source);
        while(dummy->getSubcards().length() <= leng){
            int card_id = room->askForAG(source, quan, false, skill_name);
            if(card_id < 0)
                continue;
            room->takeAG(NULL, card_id);
            dummy->addSubcard(card_id);
            if(dummy->getSubcards().length() == leng)
                break;
        }
        source->invoke("clearAG");
        foreach(int x, getSubcards())
            source->addToPile("werpo", x);
        room->moveCardTo(dummy, source, Player::Hand, false);
    }
}

class V5YexinViewAsSkill: public ViewAsSkill{
public:
    V5YexinViewAsSkill(): ViewAsSkill("v5yexin"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player && !player->isKongcheng() && !player->getPile("werpo").isEmpty();
    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *to) const{
        return !to->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty() || cards.length() > Self->getPile("werpo").length())
            return NULL;
        V5YexinCard *card = new V5YexinCard;
        card->addSubcards(cards);
        return card;
    }
};

class V5Yexin: public TriggerSkill{
public:
    V5Yexin():TriggerSkill("v5yexin"){
        events << Damage << Damaged;
        view_as_skill = new V5YexinViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());
            int up = room->drawCard();
            player->addToPile("werpo", up);
        }
        return false;
    }
};

class V5Zili: public PhaseChangeSkill{
public:
    V5Zili():PhaseChangeSkill("v5zili"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Start
                && target->hasSkill(objectName())
                && target->isAlive()
                && target->getMark(objectName()) == 0;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Start)
            return false;
        int qn = player->getPile("werpo").length();
        if(qn >= 4){
            Room *room = player->getRoom();
            LogMessage log;
            log.type = "#ZiLiWake";
            log.from = player;
            log.arg = QString::number(qn);
            log.arg2 = objectName();
            room->sendLog(log);

            room->playSkillEffect(objectName());
            room->broadcastInvoke("animate", "lightbox:$v5zili:2500");
            room->getThread()->delay(2500);

            room->loseMaxHp(player);

            room->acquireSkill(player, "v5paiyi");
            room->setPlayerMark(player, objectName(), 1);
        }
        return false;
    }
};

class V5Paiyi: public PhaseChangeSkill{
public:
    V5Paiyi():PhaseChangeSkill("v5paiyi"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Finish)
            return false;
        Room *room = player->getRoom();
        while(player){
            const QList<int> &quan = player->getPile("werpo");
            if(quan.isEmpty() || !player->askForSkillInvoke(objectName()))
                break;
            room->fillAG(quan, player);
            int card_id = room->askForAG(player, quan, false, objectName());
            if(card_id < 0)
                card_id = quan.first();
            const Card *card = Sanguosha->getCard(card_id);

            Player::Place place = Player::Hand;
            int equip_index = -1;
            const DelayedTrick *trick = NULL;
            if(card->isKindOf("EquipCard")){
                const EquipCard *equip = qobject_cast<const EquipCard *>(card);
                equip_index = static_cast<int>(equip->location());
                place = Player::Equip;
            }else if(card->isKindOf("TrickCard")){
                trick = DelayedTrick::CastFrom(card);
                place = Player::Judging;
            }
            else
                trick = DelayedTrick::CastFrom(card);

            QList<ServerPlayer *> tos;
            foreach(ServerPlayer *p, room->getAlivePlayers()){
                switch(place){
                case Player::Judging:{
                    if(!player->isProhibited(p, trick) && !p->containsTrick(trick->objectName()))
                        tos << p;
                    break;
                }
                case Player::Equip:{
                    if(p->getEquip(equip_index) == NULL)
                        tos << p;
                    break;
                }
                default: tos << p; break;
                }
            }
            if(trick && trick->isVirtualCard())
                delete trick;

            ServerPlayer *to = room->askForPlayerChosen(player, tos, objectName());
            if(trick && place == Player::Hand){
                QString ch = room->askForChoice(player, objectName(), "judge+hand+werpo");
                if(ch == "judge")
                    place = Player::Judging;
                else if(ch == "werpo")
                    place = Player::Special;
            }
            room->moveCardTo(card, to, place);

            player->invoke("clearAG");
        }
        return false;
    }
};

DanchuangPackage::DanchuangPackage()
    :Package("danchuang")
{
    General *v5zhonghui = new General(this, "v5zhonghui", "wei", 3);
    v5zhonghui->addSkill(new V5Zhenggong);
    v5zhonghui->addSkill(new V5Quanji);
    v5zhonghui->addSkill(new V5Baijiang);
    skills << new V5Yexin;
    v5zhonghui->addRelateSkill("v5yexin");
    skills << new V5Paiyi;
    v5zhonghui->addSkill(new V5Zili);
    v5zhonghui->addRelateSkill("v5paiyi");

    addMetaObject<V5QuanjiCard>();
    addMetaObject<V5YexinCard>();
}

ADD_PACKAGE(Danchuang)
