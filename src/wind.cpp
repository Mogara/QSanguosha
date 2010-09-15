#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"

// skill cards

GuidaoCard::GuidaoCard(){
    target_fixed = true;
}

LeijiCard::LeijiCard(){

}

HuangtianCard::HuangtianCard(){
    target_fixed = true;
}

void HuangtianCard::use(const QList<const ClientPlayer *> &targets) const{
    ClientInstance->turn_tag.insert("huangtian_used", true);
}

void HuangtianCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &) const{
    ServerPlayer *zhangjiao = room->getLord();
    if(zhangjiao->hasSkill("huangtian")){
        zhangjiao->obtainCard(this);
    }
}

GongxinCard::GongxinCard(){

}

bool GongxinCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

class Guidao:public ViewAsSkill{
public:
    Guidao():ViewAsSkill("guidao"){

    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@guidao";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && to_select->getCard()->isBlack();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        Card *card = new GuidaoCard;
        card->addSubcard(cards.first()->getCard()->getId());

        return card;
    }
};

class HuangtianViewAsSkill: public ViewAsSkill{
public:
    HuangtianViewAsSkill():ViewAsSkill("huangtian"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! ClientInstance->turn_tag.value("huangtian_used", false).toBool();
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(!selected.isEmpty())
            return false;

        if(to_select->isEnabled())
            return false;

        const Card *card = to_select->getCard();
        return card->objectName() == "jink" || card->objectName() == "lightning";
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        HuangtianCard *card = new HuangtianCard;
        card->addSubcards(cards);

        return card;
    }
};

class Huangtian: public GameStartSkill{
public:
    Huangtian():GameStartSkill("huangtian"){

    }

    virtual void onGameStart(ServerPlayer *player) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> players = room->getOtherPlayers(player);

        foreach(ServerPlayer *player, players){
            if(player->getGeneral()->getKingdom() == "qun")
                room->attachSkillToPlayer(player, "huangtian");
        }
    }
};

class LeijiViewAsSkill: public ZeroCardViewAsSkill{
public:
    LeijiViewAsSkill():ZeroCardViewAsSkill("leiji"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@leiji";
    }

    virtual const Card *viewAs() const{
        return new LeijiCard;
    }
};

class Leiji: public TriggerSkill{
public:
    Leiji():TriggerSkill("leiji"){
        events << Jinked;
        view_as_skill = new LeijiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhangjiao, QVariant &) const{
        Room *room = zhangjiao->getRoom();

        QList<ServerPlayer *> targets;
        const Card *leiji_card = room->askForCardWithTargets(zhangjiao, "@@leiji", "@leiji", targets);
        if(leiji_card){
            ServerPlayer *target = targets.first();

            int card_id = room->getJudgeCard(target);
            const Card *card = Sanguosha->getCard(card_id);
            if(card->getSuit() == Card::Spade){
                DamageStruct damage;
                damage.card = leiji_card;
                damage.damage = 2;
                damage.from = zhangjiao;
                damage.to = target;
                damage.nature = DamageStruct::Thunder;

                room->damage(damage);
            }
        }

        return false;
    }
};

ShensuCard::ShensuCard(){
}

bool ShensuCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;

    return true;
}

void ShensuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    room->playSkillEffect("shensu");

    SlashEffectStruct effect;
    effect.slash = new Slash(Card::NoSuit, 0);
    effect.from = source;
    effect.to = targets.first();
    effect.nature = DamageStruct::Normal;

    room->slashEffect(effect);
}

class Shensu1ViewAsSkill: public ZeroCardViewAsSkill{
public:
    Shensu1ViewAsSkill():ZeroCardViewAsSkill("shensu1"){
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@shensu1";
    }

    virtual const Card *viewAs() const{
        return new ShensuCard;
    }
};

class Shensu2ViewAsSkill: public ViewAsSkill{
public:
    Shensu2ViewAsSkill():ViewAsSkill("shensu2"){
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@shensu2";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        ShensuCard *card = new ShensuCard;
        card->addSubcards(cards);

        return card;
    }
};

class Shensu1: public PhaseChangeSkill{
public:
    Shensu1():PhaseChangeSkill("shensu1"){
        view_as_skill = new Shensu1ViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *xiahouyuan) const{
        Room *room = xiahouyuan->getRoom();

        if(xiahouyuan->getPhase() == Player::Start){
            QList<ServerPlayer *> targets;
            const Card *card = room->askForCardWithTargets(xiahouyuan, "@@shensu1", "@shensu1", targets);

            if(card){
                room->skip(Player::Judge);
                room->skip(Player::Draw);
                room->cardEffect(card, xiahouyuan, targets.first());
            }
        }

        return false;
    }
};

class Shensu2: public PhaseChangeSkill{
public:
    Shensu2():PhaseChangeSkill("shensu2"){
        view_as_skill = new Shensu2ViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *xiahouyuan) const{
        Room *room = xiahouyuan->getRoom();

        if(xiahouyuan->getPhase() == Player::Start){
            QList<ServerPlayer *> targets;
            const Card *card = room->askForCardWithTargets(xiahouyuan, "@@shensu2", "@shensu2", targets);

            if(card){
                room->skip(Player::Play);
                room->cardEffect(card, xiahouyuan, targets.first());
            }
        }

        return false;
    }
};

class Jushou: public PhaseChangeSkill{
public:
    Jushou():PhaseChangeSkill("jushou"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            if(room->askForSkillInvoke(target, objectName())){
                target->drawCards(3);
                target->turnOver();

                room->broadcastProperty(target, "face_up");
                room->playSkillEffect(objectName());
            }
        }

        return false;
    }
};

class Liegong: public SlashBuffSkill{
public:
    Liegong():SlashBuffSkill("liegong"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *huangzhong = effect.from;
        Room *room = huangzhong->getRoom();
        if(room->getCurrent() != huangzhong)
            return false;

        int num = effect.to->getHandcardNum();
        if(num >= huangzhong->getHp() || num <= huangzhong->getAttackRange()){
            if(room->askForSkillInvoke(huangzhong, "liegong")){
                SlashResultStruct result;
                result.fill(effect, true);
                room->slashResult(result);

                return true;
            }
        }

        return false;
    }
};

class Kuanggu: public TriggerSkill{
public:
    Kuanggu():TriggerSkill("kuanggu"){
        frequency = Compulsory;
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        if(data.canConvert<DamageStruct>()){
            DamageStruct damage = data.value<DamageStruct>();

            if(player->distanceTo(damage.to) <= 1){
                Room *room = player->getRoom();
                room->recover(player, damage.damage);
            }
        }

        return false;
    }
};

class Wuhun: public TriggerSkill{
public:
    Wuhun():TriggerSkill("wuhun"){
        events << Death;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        // FIXME:

        return false;
    }
};

static bool CompareBySuit(int card1, int card2){
    const Card *c1 = Sanguosha->getCard(card1);
    const Card *c2 = Sanguosha->getCard(card2);

    int a = static_cast<int>(c1->getSuit());
    int b = static_cast<int>(c2->getSuit());

    return a < b;
}

class Shelie: public PhaseChangeSkill{
public:
    Shelie():PhaseChangeSkill("shelie"){

    }

    virtual bool onPhaseChange(ServerPlayer *shenlumeng) const{
        if(shenlumeng->getPhase() != Player::Draw)
            return false;

        Room *room = shenlumeng->getRoom();
        if(!room->askForSkillInvoke(shenlumeng, objectName()))
            return false;

        QList<int> card_ids = room->getNCards(5);
        qSort(card_ids.begin(), card_ids.end(), CompareBySuit);
        QStringList card_str;
        foreach(int card_id, card_ids)
            card_str << QString::number(card_id);
        room->broadcastInvoke("fillAG", card_str.join("+"));

        while(!card_ids.isEmpty()){
            int card_id = room->askForAG(shenlumeng);
            room->takeAG(shenlumeng, card_id);

            const Card *card = Sanguosha->getCard(card_id);

            // quick-and-dirty
            shenlumeng->addCard(card, Player::Hand);
            room->setCardMapping(card_id, shenlumeng, Player::Hand);

            Card::Suit suit = card->getSuit();
            card_ids.removeOne(card_id);
            QMutableListIterator<int> itor(card_ids);
            while(itor.hasNext()){
                const Card *c = Sanguosha->getCard(itor.next());
                if(c->getSuit() == suit){
                    itor.remove();

                    room->setCardMapping(c->getId(), NULL, Player::DiscardedPile);
                    room->takeAG(NULL, c->getId());
                }
            }
        }

        room->broadcastInvoke("clearAG");

        return true;
    }
};

class Gongxin: ZeroCardViewAsSkill{
public:
    Gongxin():ZeroCardViewAsSkill("gongxin"){

    }

    virtual const Card *viewAs() const{
        return new GongxinCard;
    }
};

WindPackage::WindPackage()
    :Package("wind")
{
    // xiaoqiao, zhoutai and yuji is omitted
    General *xiahouyuan, *caoren, *huangzhong, *weiyan, *zhangjiao;

    xiahouyuan = new General(this, "xiahouyuan", "wei");
    xiahouyuan->addSkill(new Shensu1);
    xiahouyuan->addSkill(new Shensu2);

    caoren = new General(this, "caoren", "wei");
    caoren->addSkill(new Jushou);

    huangzhong = new General(this, "huangzhong", "shu");
    huangzhong->addSkill(new Liegong);

    weiyan = new General(this, "weiyan", "shu");
    weiyan->addSkill(new Kuanggu);

    zhangjiao = new General(this, "zhangjiao$", "qun", 3);
    zhangjiao->addSkill(new Guidao);
    zhangjiao->addSkill(new Leiji);
    zhangjiao->addSkill(new Huangtian);

    // two gods
    General *shenguanyu, *shenlumeng;

    shenguanyu = new General(this, "shenguanyu", "shu", 5);
    shenguanyu->addSkill(new Wuhun);

    shenlumeng = new General(this, "shenlumeng", "wu", 3);
    shenlumeng->addSkill(new Shelie);
    // shenlumeng->addSkill(new Gongxin);

    t["wind"] = tr("wind");

    t["xiahouyuan"] = tr("xiahouyuan");
    t["caoren"] = tr("caoren");
    t["huangzhong"] = tr("huangzhong");
    t["weiyan"] = tr("weiyan");
    t["zhangjiao"] = tr("zhangjiao");
    t["shenguanyu"] = tr("shenguanyu");
    t["shenlumeng"] = tr("shenlumeng");

    // skills
    t["shensu1"] = tr("shensu1");
    t["shensu2"] = tr("shensu2");
    t["jushou"] = tr("jushou");
    t["liegong"] = tr("liegong");
    t["kuanggu"] = tr("kuanggu");
    t["guidao"] = tr("guidao");
    t["leiji"] = tr("leiji");
    t["huangtian"] = tr("huangtian");
    t["wuhun"] = tr("wuhun");
    t["shelie"] = tr("shelie");
    t["gongxin"] = tr("gongxin");

    // skill prompt
    t["liegong:yes"] = tr("liegong:yes");

    addMetaObject<GuidaoCard>();
    addMetaObject<HuangtianCard>();
    addMetaObject<GongxinCard>();
    addMetaObject<LeijiCard>();    

    skills << new HuangtianViewAsSkill;
}

ADD_PACKAGE(Wind)


