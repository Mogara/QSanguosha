#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"
#include "general.h"

// skill cards

GuidaoCard::GuidaoCard(){
    target_fixed = true;
    will_throw = false;
    can_jilei = true;
}

void GuidaoCard::use(Room *room, ServerPlayer *zhangjiao, const QList<ServerPlayer *> &targets) const{

}

LeijiCard::LeijiCard(){

}

bool LeijiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void LeijiCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhangjiao = effect.from;
    ServerPlayer *target = effect.to;

    Room *room = zhangjiao->getRoom();
    room->setEmotion(target, "bad");

    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(spade):(.*)");
    judge.good = false;
    judge.reason = "leiji";
    judge.who = target;

    room->judge(judge);

    if(judge.isBad()){
        DamageStruct damage;
        damage.card = NULL;
        damage.damage = 2;
        damage.from = zhangjiao;
        damage.to = target;
        damage.nature = DamageStruct::Thunder;

        room->damage(damage);
    }else
        room->setEmotion(zhangjiao, "bad");
}

HuangtianCard::HuangtianCard(){
}

void HuangtianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *zhangjiao = targets.first();
    if(zhangjiao->hasLordSkill("huangtian")){
        room->setPlayerFlag(zhangjiao, "HuangtianInvoked");
        zhangjiao->obtainCard(this);
        QList<int> subcards = this->getSubcards();
        foreach(int card_id, subcards)
            room->setCardFlag(card_id,"visible");
        room->setEmotion(zhangjiao, "good");
        QList<ServerPlayer *> zhangjiaos;
        QList<ServerPlayer *> players = room->getOtherPlayers(source);
        ServerPlayer *lordplayer = NULL;
        if(source->isLord())
            lordplayer = source;
        foreach(ServerPlayer *p, players){
            if(p->hasLordSkill("huangtian") && !p->hasFlag("HuangtianInvoked") && !p->loseViewasSkills()){
                zhangjiaos << p;
            }
            if(p->isLord())
                lordplayer = p;
        }
        if(zhangjiaos.empty() && lordplayer->loseViewasSkills())
            room->setPlayerFlag(source, "ForbidHuangtian");
    }
}

bool HuangtianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("huangtian")
            && to_select != Self && !to_select->hasFlag("HuangtianInvoked")
            && !to_select->loseViewasSkills();
}

class GuidaoViewAsSkill:public OneCardViewAsSkill{
public:
    GuidaoViewAsSkill():OneCardViewAsSkill(""){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@guidao";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        GuidaoCard *card = new GuidaoCard;
        card->setSuit(card_item->getFilteredCard()->getSuit());
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Guidao: public TriggerSkill{
public:
    Guidao():TriggerSkill("guidao"){
        view_as_skill = new GuidaoViewAsSkill;

        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if(!TriggerSkill::triggerable(target))
            return false;

        if(target->isKongcheng()){
            bool has_black = false;
            for(int i = 0; i < 4; i++){
                const EquipCard *equip = target->getEquip(i);
                if(equip && equip->isBlack()){
                    has_black = true;
                    break;
                }
            }

            return has_black;
        }else
            return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if (player == NULL) return false;
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@guidao-card" << judge->who->objectName()
                << objectName() << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");
        const Card *card = room->askForCard(player, "@guidao", prompt, data);

        if(card){
            // the only difference for Guicai & Guidao
            const Card* oldJudge = judge->card;
            judge->card = Sanguosha->getCard(card->getEffectiveId());

            CardsMoveStruct move1(QList<int>(), NULL, Player::DiscardPile,
                CardMoveReason(CardMoveReason::S_REASON_JUDGE, player->objectName(), this->objectName(), QString()));
            move1.card_ids.append(card->getEffectiveId());
            
            CardsMoveStruct move2(QList<int>(), player, Player::Hand,
                CardMoveReason(CardMoveReason::S_REASON_OVERRIDE, player->objectName(), this->objectName(), QString()));
            move2.card_ids.append(oldJudge->getEffectiveId());

            QList<CardsMoveStruct> moves;
            moves.append(move1);
            moves.append(move2);
            room->moveCardsAtomic(moves, true);            

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);
        }

        return false;
    }
};

class HuangtianViewAsSkill: public OneCardViewAsSkill{
public:
    HuangtianViewAsSkill():OneCardViewAsSkill("huangtianv"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        const Player *lord = NULL;
        if(player->isLord())
            lord = player;
        QList<const Player *> players = player->getSiblings();
        foreach(const Player *p, players){
            if(p->isLord())
                lord = p;
        }
        return player->getKingdom() == "qun" && !player->hasFlag("ForbidHuangtian")
                && !lord->loseViewasSkills();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->objectName() == "jink" || card->objectName() == "lightning";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        HuangtianCard *card = new HuangtianCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Huangtian: public TriggerSkill{
public:
    Huangtian():TriggerSkill("huangtian$"){
        events << GameStart << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == GameStart && player->isLord()){
            QList<ServerPlayer *> players = room->getAlivePlayers();
            foreach(ServerPlayer *p, players){
                room->attachSkillToPlayer(p, "huangtianv");
            }
        }
        else if(event == PhaseChange && player->getPhase() == Player::NotActive){
            if(player->hasFlag("ForbidHuangtian")){
                room->setPlayerFlag(player, "-ForbidHuangtian");
            }
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            foreach(ServerPlayer *p, players){
                if(p->hasFlag("HuangtianInvoked")){
                    room->setPlayerFlag(p, "-HuangtianInvoked");
                }
            }
        }
        return false;
    }
};

class LeijiViewAsSkill: public ZeroCardViewAsSkill{
public:
    LeijiViewAsSkill():ZeroCardViewAsSkill("leiji"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@leiji";
    }

    virtual const Card *viewAs() const{
        return new LeijiCard;
    }
};

class Leiji: public TriggerSkill{
public:
    Leiji():TriggerSkill("leiji"){
        events << CardResponsed;
        view_as_skill = new LeijiViewAsSkill;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *zhangjiao, QVariant &data) const{
        if (zhangjiao == NULL) return false;
        CardStar card_star = data.value<CardStar>();
        if(!card_star->inherits("Jink"))
            return false;       
        room->askForUseCard(zhangjiao, "@@leiji", "@leiji");

        return false;
    }
};

ShensuCard::ShensuCard(){
    mute = true;
}

bool ShensuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void ShensuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this, source);

    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("shensu");
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to = targets;

    room->useCard(use);
}

class ShensuViewAsSkill: public ViewAsSkill{
public:
    ShensuViewAsSkill():ViewAsSkill("shensu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(ClientInstance->getPattern().endsWith("1"))
            return false;
        else
            return selected.isEmpty() && to_select->getCard()->inherits("EquipCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern.startsWith("@@shensu");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(ClientInstance->getPattern().endsWith("1")){
            if(cards.isEmpty())
                return new ShensuCard;
            else
                return NULL;
        }else{
            if(cards.length() != 1)
                return NULL;

            ShensuCard *card = new ShensuCard;
            card->addSubcards(cards);

            return card;
        }
    }
};

class Shensu: public PhaseChangeSkill{
public:
    Shensu():PhaseChangeSkill("shensu"){
        view_as_skill = new ShensuViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *xiahouyuan) const{
        Room *room = xiahouyuan->getRoom();

        if(xiahouyuan->getPhase() == Player::Judge){
            if(room->askForUseCard(xiahouyuan, "@@shensu1", "@shensu1")){
                xiahouyuan->skip(Player::Draw);
                return true;
            }
        }else if(xiahouyuan->getPhase() == Player::Play){
            if(room->askForUseCard(xiahouyuan, "@@shensu2", "@shensu2")){
                return true;
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
        if(huangzhong->getPhase() != Player::Play)
            return false;

        int num = effect.to->getHandcardNum();
        if(num >= huangzhong->getHp() || num <= huangzhong->getAttackRange()){
            if(huangzhong->askForSkillInvoke(objectName(), QVariant::fromValue(effect))){
                room->playSkillEffect(objectName());
                room->slashResult(effect, NULL);

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
        events << Damage << DamageDone;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *weiyan = room->findPlayerBySkillName(objectName());
        if(weiyan && weiyan->loseTriggerSkills())
            return false;
        if(event == DamageDone && damage.from && damage.from->hasSkill("kuanggu") && damage.from->isAlive()){
            ServerPlayer *weiyan = damage.from;
            weiyan->tag["InvokeKuanggu"] = weiyan->distanceTo(damage.to) <= 1;
        }else if(event == Damage && player->hasSkill("kuanggu") && player->isAlive()){
            bool invoke = player->tag.value("InvokeKuanggu", false).toBool();
            if(invoke){
                Room *room = player->getRoom();

                room->playSkillEffect(objectName());

                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);

                RecoverStruct recover;
                recover.who = player;
                recover.recover = damage.damage;
                room->recover(player, recover);
            }
        }

        return false;
    }
};

class BuquRemove: public TriggerSkill{
public:
    BuquRemove():TriggerSkill("#buqu-remove"){
        events << HpRecover;
    }

    virtual int getPriority() const{
        return -1;
    }

    static void Remove(ServerPlayer *zhoutai){
        Room *room = zhoutai->getRoom();
        const QList<int> buqu(zhoutai->getPile("buqu"));

        CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, zhoutai->objectName(), "buqu", QString());
        int need = 1 - zhoutai->getHp();
        if(need <= 0){
            // clear all the buqu cards
            foreach(int card_id, buqu) {                
                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
            }
        }else{
            int to_remove = buqu.length() - need;

            room->fillAG(buqu);
                        
            for(int i = 0; i < to_remove; i++){
                int card_id = room->askForAG(zhoutai, buqu, false, "buqu");                
                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
            }

            room->broadcastInvoke("clearAG");
        }
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *zhoutai, QVariant &) const{
        if(zhoutai->getHp() < 1)
            Remove(zhoutai);

        return false;
    }
};

class Buqu: public TriggerSkill{
public:
    Buqu():TriggerSkill("buqu"){
        events << Dying << AskForPeachesDone;
        default_choice = "alive";
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *zhoutai, QVariant &) const{
        if(event == Dying){
            QString choice = room->askForChoice(zhoutai, objectName(), "alive+dead");
            if(choice == "alive"){
                room->setTag("Buqu", zhoutai->objectName());
                room->playSkillEffect(objectName());
                const QList<int> &buqu = zhoutai->getPile("buqu");

                int need = 1 - zhoutai->getHp(); // the buqu cards that should be turned over
                int n = need - buqu.length();
                if(n > 0){
                    QList<int> card_ids = room->getNCards(n);
                    foreach(int card_id, card_ids){
                        zhoutai->addToPile("buqu", card_id);
                    }
                }
                const QList<int> &buqunew = zhoutai->getPile("buqu");
                QList<int> duplicate_numbers;

                QSet<int> numbers;
                foreach(int card_id, buqunew){
                    const Card *card = Sanguosha->getCard(card_id);
                    int number = card->getNumber();

                    if(numbers.contains(number)){
                        duplicate_numbers << number;
                    }else
                        numbers << number;
                }

                if(duplicate_numbers.isEmpty()){
                    room->setTag("Buqu", QVariant());
                    zhoutai->setFlags("-dying");
                    return true;
                }
            }
        }else if(event == AskForPeachesDone){
            const QList<int> &buqu = zhoutai->getPile("buqu");

            if(zhoutai->getHp() > 0)
                return false;
            if(room->getTag("Buqu").toString() != zhoutai->objectName())
                return false;
            room->setTag("Buqu", QVariant());
            QList<int> duplicate_numbers;

            QSet<int> numbers;
            foreach(int card_id, buqu){
                const Card *card = Sanguosha->getCard(card_id);
                int number = card->getNumber();

                if(numbers.contains(number)){
                    duplicate_numbers << number;
                }else
                    numbers << number;
            }

            if(duplicate_numbers.isEmpty()){
                room->playSkillEffect(objectName());
                zhoutai->setFlags("-dying");
                return true;
            }else{
                LogMessage log;
                log.type = "#BuquDuplicate";
                log.from = zhoutai;
                log.arg = QString::number(duplicate_numbers.length());
                room->sendLog(log);

                for(int i=0; i<duplicate_numbers.length(); i++){
                    int number = duplicate_numbers.at(i);

                    LogMessage log;
                    log.type = "#BuquDuplicateGroup";
                    log.from = zhoutai;
                    log.arg = QString::number(i+1);
                    log.arg2 = Card::Number2String(number);
                    room->sendLog(log);

                    foreach(int card_id, buqu){
                        const Card *card = Sanguosha->getCard(card_id);
                        if(card->getNumber() == number){
                            LogMessage log;
                            log.type = "$BuquDuplicateItem";
                            log.from = zhoutai;
                            log.card_str = QString::number(card_id);
                            room->sendLog(log);
                        }
                    }
                }
            }
        }

        return false;
    }
};

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCommandLinkButton>

class Hongyan: public FilterSkill{
public:
    Hongyan():FilterSkill("hongyan"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *new_card = Card::Clone(card);
        if(new_card) {
            new_card->setSuit(Card::Heart);
            new_card->setSkillName(objectName());
            return new_card;
        }else
            return card;
    }
};

class HongyanRetrial: public TriggerSkill{
public:
    HongyanRetrial():TriggerSkill("#hongyan-retrial"){
        frequency = Compulsory;

        events << FinishJudge;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        if(judge->card->getSuit() == Card::Spade){
            LogMessage log;
            log.type = "#HongyanJudge";
            log.from = player;

            Card *new_card = Card::Clone(judge->card);
            new_card->setSuit(Card::Heart);
            new_card->setSkillName("hongyan");
            judge->card = new_card;

            player->getRoom()->sendLog(log);
        }

        return false;
    }
};

TianxiangCard::TianxiangCard()
{
    owner_discarded = true;
}

void TianxiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    DamageStruct damage = effect.from->tag["TianxiangDamage"].value<DamageStruct>();
    damage.to = effect.to;
    damage.chain = true;
    room->damage(damage);

    if(damage.to->isAlive())
        damage.to->drawCards(damage.to->getLostHp());
}

class TianxiangViewAsSkill: public OneCardViewAsSkill{
public:
    TianxiangViewAsSkill():OneCardViewAsSkill("tianxiang"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@tianxiang";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        TianxiangCard *card = new TianxiangCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Tianxiang: public TriggerSkill{
public:
    Tianxiang():TriggerSkill("tianxiang"){
        events << Predamaged;

        view_as_skill = new TianxiangViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *xiaoqiao, QVariant &data) const{
        if(!xiaoqiao->isKongcheng()){
            DamageStruct damage = data.value<DamageStruct>();

            xiaoqiao->tag["TianxiangDamage"] = QVariant::fromValue(damage);
            if(room->askForUseCard(xiaoqiao, "@@tianxiang", "@tianxiang-card"))
                return true;
        }

        return false;
    }
};

GuhuoCard::GuhuoCard(){
    mute = true;
}

bool GuhuoCard::guhuo(ServerPlayer* yuji, const QString& message) const{
    Room *room = yuji->getRoom();
    room->setTag("Guhuoing", true);
    room->setTag("GuhuoType", this->user_string);

    // yuji->addToPile("#guhuo_pile", this->getEffectiveId(), false);
    room->moveCardTo(this, yuji, Player::PlaceTakeoff, 
        CardMoveReason(CardMoveReason::S_REASON_UNKNOWN, yuji->objectName(), "guhuo", user_string), false);

    QList<ServerPlayer *> players = room->getOtherPlayers(yuji);
    QSet<ServerPlayer *> questioned;

    foreach(ServerPlayer *player, players){
        if(player->getHp() <= 0){
            LogMessage log;
            log.type = "#GuhuoCannotQuestion";
            log.from = player;
            log.arg = QString::number(player->getHp());
            room->sendLog(log);

            room->setEmotion(player, "no-question");

            continue;
        }

        if(player->getState()=="online")
            player->invoke("log", message);
        QString choice = room->askForChoice(player, "guhuo", "question+noquestion");
        if(choice == "question"){
            room->setEmotion(player, "question");
            questioned << player;
        }else
            room->setEmotion(player, "no-question");

        LogMessage log;
        log.type = "#GuhuoQuery";
        log.from = player;
        log.arg = choice;

        room->sendLog(log);
    }

    room->moveCardTo(this, NULL, Player::DiscardPile,
        CardMoveReason(CardMoveReason::S_REASON_SHOW, yuji->objectName(), "guhuo", user_string), true, false);
    
    LogMessage log;
    log.type = "$GuhuoResult";
    log.from = yuji;
    log.card_str = QString::number(subcards.first());
    room->sendLog(log);

    bool success = false;
    if(questioned.isEmpty()){
        success = true;

        foreach(ServerPlayer *player, players)
            room->setEmotion(player, ".");

    }else{
        const Card *card = Sanguosha->getCard(subcards.first());
        bool real;
        if(user_string == "peach+analeptic")
            real = card->objectName() == "peach" || card->objectName() == "analeptic";
        else
            real = card->match(user_string);

        success = real && card->getSuit() == Card::Heart;

        foreach(ServerPlayer *player, players){
            room->setEmotion(player, ".");

            if(questioned.contains(player)){
                if(real)
                    room->loseHp(player);
                else
                    player->drawCards(1);
            }
        }
    }
    
    room->setTag("Guhuoing", false);
    room->removeTag("GuhuoType");

    return success;
}

GuhuoDialog *GuhuoDialog::GetInstance(const QString &object, bool left, bool right){
    static GuhuoDialog *instance;
    if(instance == NULL)
        instance = new GuhuoDialog(object, left, right);

    return instance;
}

GuhuoDialog::GuhuoDialog(const QString &object, bool left, bool right):object_name(object)
{
    setWindowTitle(Sanguosha->translate(object));
    group = new QButtonGroup(this);

    QHBoxLayout *layout = new QHBoxLayout;
    if(left)
        layout->addWidget(createLeft());
    if(right)
        layout->addWidget(createRight());

    setLayout(layout);

    connect(group, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(selectCard(QAbstractButton*)));
}

void GuhuoDialog::popup(){
    if(ClientInstance->getStatus() != Client::Playing)
        return;

    foreach(QAbstractButton *button, group->buttons()){
        const Card *card = map[button->objectName()];
        button->setEnabled(card->isAvailable(Self));
    }

    Self->tag.remove(object_name);
    exec();
}

void GuhuoDialog::selectCard(QAbstractButton *button){
    CardStar card = map.value(button->objectName());
    Self->tag[object_name] = QVariant::fromValue(card);
    accept();
}

QGroupBox *GuhuoDialog::createLeft(){
    QGroupBox *box = new QGroupBox;
    box->setTitle(Sanguosha->translate("basic"));

    QVBoxLayout *layout = new QVBoxLayout;

    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach(const Card *card, cards){
        if(card->getTypeId() == Card::Basic && !map.contains(card->objectName())){
            Card *c = Sanguosha->cloneCard(card->objectName(), Card::NoSuit, 0);
            c->setParent(this);

            layout->addWidget(createButton(c));
        }
    }

    layout->addStretch();

    box->setLayout(layout);
    return box;
}

QGroupBox *GuhuoDialog::createRight(){
    QGroupBox *box = new QGroupBox(Sanguosha->translate("ndtrick"));
    QHBoxLayout *layout = new QHBoxLayout;

    QGroupBox *box1 = new QGroupBox(Sanguosha->translate("single_target"));
    QVBoxLayout *layout1 = new QVBoxLayout;

    QGroupBox *box2 = new QGroupBox(Sanguosha->translate("multiple_targets"));
    QVBoxLayout *layout2 = new QVBoxLayout;


    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach(const Card *card, cards){
        if(card->isNDTrick() && !map.contains(card->objectName())){
            Card *c = Sanguosha->cloneCard(card->objectName(), Card::NoSuit, 0);
            c->setSkillName(object_name);
            c->setParent(this);

            QVBoxLayout *layout = c->inherits("SingleTargetTrick") ? layout1 : layout2;
            layout->addWidget(createButton(c));
        }
    }

    box->setLayout(layout);
    box1->setLayout(layout1);
    box2->setLayout(layout2);

    layout1->addStretch();
    layout2->addStretch();

    layout->addWidget(box1);
    layout->addWidget(box2);
    return box;
}

QAbstractButton *GuhuoDialog::createButton(const Card *card){
    QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate(card->objectName()));
    button->setObjectName(card->objectName());
    button->setToolTip(card->getDescription());

    map.insert(card->objectName(), card);
    group->addButton(button);

    return button;
}

bool GuhuoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    CardStar card = Self->tag.value("guhuo").value<CardStar>();
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card);
}

bool GuhuoCard::targetFixed() const{
    if(ClientInstance->getStatus() == Client::Responsing)
        return true;

    CardStar card = Self->tag.value("guhuo").value<CardStar>();
    return card && card->targetFixed();
}

bool GuhuoCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    CardStar card = Self->tag.value("guhuo").value<CardStar>();
    return card && card->targetsFeasible(targets, Self);
}

const Card *GuhuoCard::validate(const CardUseStruct *card_use) const{
    Room *room = card_use->from->getRoom();
    room->playSkillEffect("guhuo");

    LogMessage log;
    log.type = card_use->to.isEmpty() ? "#GuhuoNoTarget" : "#Guhuo";
    log.from = card_use->from;
    log.to = card_use->to;
    log.arg = user_string;
    log.arg2 = "guhuo";

    room->sendLog(log);

    if(guhuo(card_use->from, log.toString())){
        const Card *card = Sanguosha->getCard(subcards.first());
        Card *use_card = Sanguosha->cloneCard(user_string, card->getSuit(), card->getNumber());
        use_card->setSkillName("guhuo");
        use_card->addSubcard(this);
        // @todo: verify this...
        CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, card_use->from->objectName());
        room->throwCard(this, reason, NULL);

        return use_card;
    }else
        return NULL;
}

const Card *GuhuoCard::validateInResposing(ServerPlayer *yuji, bool *continuable) const{
    *continuable = true;

    Room *room = yuji->getRoom();
    room->playSkillEffect("guhuo");

    QString to_guhuo;
    if(user_string == "peach+analeptic")
        to_guhuo = room->askForChoice(yuji, "guhuo-saveself", user_string);
    else
        to_guhuo = user_string;

    LogMessage log;
    log.type = "#GuhuoNoTarget";
    log.from = yuji;
    log.arg = to_guhuo;
    log.arg2 = "guhuo";
    room->sendLog(log);

    if (guhuo(yuji,log.toString())){
        const Card *card = Sanguosha->getCard(subcards.first());
        Card *use_card = Sanguosha->cloneCard(to_guhuo, card->getSuit(), card->getNumber());
        use_card->setSkillName("guhuo");
        return use_card;
    }else
        return NULL;
}

class Guhuo: public OneCardViewAsSkill{
public:
    Guhuo():OneCardViewAsSkill("guhuo"){
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return !player->isKongcheng()
                && ! pattern.startsWith("@")
                && ! pattern.startsWith(".");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        if(ClientInstance->getStatus() == Client::Responsing){
            GuhuoCard *card = new GuhuoCard;
            card->setUserString(ClientInstance->getPattern());
            card->addSubcard(card_item->getFilteredCard());
            return card;
        }

        CardStar c = Self->tag.value("guhuo").value<CardStar>();
        if(c){
            GuhuoCard *card = new GuhuoCard;
            card->setUserString(c->objectName());
            card->addSubcard(card_item->getFilteredCard());

            return card;
        }else
            return NULL;
    }

    virtual QDialog *getDialog() const{
        return GuhuoDialog::GetInstance("guhuo");
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 0;
    }
};

WindPackage::WindPackage()
    :Package("wind")
{
    General *xiahouyuan, *caoren, *huangzhong, *weiyan, *zhangjiao, *zhoutai;

    xiahouyuan = new General(this, "xiahouyuan", "wei");
    xiahouyuan->addSkill(new Shensu);

    caoren = new General(this, "caoren", "wei");
    caoren->addSkill(new Jushou);

    huangzhong = new General(this, "huangzhong", "shu");
    huangzhong->addSkill(new Liegong);

    weiyan = new General(this, "weiyan", "shu");
    weiyan->addSkill(new Kuanggu);

    General *xiaoqiao = new General(this, "xiaoqiao", "wu", 3, false);
    xiaoqiao->addSkill(new Hongyan);
    xiaoqiao->addSkill(new HongyanRetrial);
    xiaoqiao->addSkill(new Tianxiang);
    related_skills.insertMulti("hongyan", "#hongyan-retrial");

    zhoutai = new General(this, "zhoutai", "wu");
    zhoutai->addSkill(new Buqu);
    zhoutai->addSkill(new BuquRemove);
    related_skills.insertMulti("buqu", "#buqu-remove");

    zhangjiao = new General(this, "zhangjiao$", "qun", 3);
    zhangjiao->addSkill(new Guidao);
    zhangjiao->addSkill(new Leiji);
    zhangjiao->addSkill(new Huangtian);

    General *yuji = new General(this, "yuji", "qun", 3);
    yuji->addSkill(new Guhuo);

    addMetaObject<ShensuCard>();
    addMetaObject<TianxiangCard>();
    addMetaObject<GuidaoCard>();
    addMetaObject<HuangtianCard>();
    addMetaObject<LeijiCard>();
    addMetaObject<GuhuoCard>();

    skills << new HuangtianViewAsSkill;

}

ADD_PACKAGE(Wind)


