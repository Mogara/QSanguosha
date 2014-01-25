#include "settings.h"
#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "engine.h"
#include "ai.h"
#include "general.h"

class Jushou: public PhaseChangeSkill {
public:
    Jushou(): PhaseChangeSkill("jushou"){

    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /* = NULL */) const{
        return TriggerSkill::triggerable(triggerEvent, room, player, data, ask_who) && player->getPhase() == Player::Finish;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->askForSkillInvoke(objectName())){
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }
    
    virtual bool onPhaseChange(ServerPlayer *target) const{
        target->drawCards(3);
        target->turnOver();

        return false;
    }
};

class KuangguGlobal: public TriggerSkill{
public:
    KuangguGlobal(): TriggerSkill("KuangguGlobal"){
        global = true;
        events << PreDamageDone;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /* = NULL */) const{
        return player != NULL;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        return true;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *weiyan = damage.from;
        weiyan->tag["InvokeKuanggu"] = weiyan->distanceTo(damage.to) <= 1;

        return false;
    }
};

class Kuanggu: public TriggerSkill {
public:
    Kuanggu(): TriggerSkill("kuanggu") {
        frequency = Compulsory;
        events << Damage;
    }

    virtual bool triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who /* = NULL */) const{
        if (TriggerSkill::triggerable(triggerEvent, room, player, data, ask_who)){
            bool invoke = player->tag.value("InvokeKuanggu", false).toBool();
            player->tag["InvokeKuanggu"] = false;
            return invoke && player->isWounded();
        }
        return false;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        return player->hasShownSkill(this) ? true : room->askForSkillInvoke(player, objectName());
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        room->broadcastSkillInvoke(objectName());

        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);
        room->notifySkillInvoked(player, objectName());

        RecoverStruct recover;
        recover.who = player;
        recover.recover = damage.damage;
        room->recover(player, recover);

        return false;
    }
};

class NosBuquRemove: public TriggerSkill {
public:
    NosBuquRemove(): TriggerSkill("#nosbuqu-remove") {
        events << HpRecover;
    }

    static void Remove(ServerPlayer *zhoutai) {
        Room *room = zhoutai->getRoom();
        QList<int> nosbuqu(zhoutai->getPile("nosbuqu"));

        CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), "nosbuqu", QString());
        int need = 1 - zhoutai->getHp();
        if (need <= 0) {
            // clear all the buqu cards
            foreach (int card_id, nosbuqu) {
                LogMessage log;
                log.type = "$NosBuquRemove";
                log.from = zhoutai;
                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);

                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
            }
        } else {
            int to_remove = nosbuqu.length() - need;
            for (int i = 0; i < to_remove; i++) {
                room->fillAG(nosbuqu);
                int card_id = room->askForAG(zhoutai, nosbuqu, false, "nosbuqu");

                LogMessage log;
                log.type = "$NosBuquRemove";
                log.from = zhoutai;
                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);

                nosbuqu.removeOne(card_id);
                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
                room->clearAG();
            }
        }
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *zhoutai, QVariant &) const{
        if (zhoutai->getPile("nosbuqu").length() > 0)
            Remove(zhoutai);

        return false;
    }
};

class NosBuqu: public TriggerSkill {
public:
    NosBuqu(): TriggerSkill("nosbuqu") {
        events << PostHpReduced << AskForPeachesDone;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *zhoutai, QVariant &data) const{
        if (triggerEvent == PostHpReduced && zhoutai->getHp() < 1) {
            if (room->askForSkillInvoke(zhoutai, objectName(), data)) {
                room->setTag("NosBuqu", zhoutai->objectName());
                room->broadcastSkillInvoke("buqu");
                const QList<int> &nosbuqu = zhoutai->getPile("nosbuqu");

                int need = 1 - zhoutai->getHp(); // the buqu cards that should be turned over
                int n = need - nosbuqu.length();
                if (n > 0) {
                    QList<int> card_ids = room->getNCards(n, false);
                    zhoutai->addToPile("nosbuqu", card_ids);
                }
                const QList<int> &nosbuqunew = zhoutai->getPile("nosbuqu");
                QList<int> duplicate_numbers;

                QSet<int> numbers;
                foreach (int card_id, nosbuqunew) {
                    const Card *card = Sanguosha->getCard(card_id);
                    int number = card->getNumber();

                    if (numbers.contains(number))
                        duplicate_numbers << number;
                    else
                        numbers << number;
                }

                if (duplicate_numbers.isEmpty()) {
                    room->setTag("NosBuqu", QVariant());
                    return true;
                }
            }
        } else if (triggerEvent == AskForPeachesDone) {
            const QList<int> &nosbuqu = zhoutai->getPile("nosbuqu");

            if (zhoutai->getHp() > 0)
                return false;
            if (room->getTag("NosBuqu").toString() != zhoutai->objectName())
                return false;
            room->setTag("NosBuqu", QVariant());

            QList<int> duplicate_numbers;
            QSet<int> numbers;
            foreach (int card_id, nosbuqu) {
                const Card *card = Sanguosha->getCard(card_id);
                int number = card->getNumber();

                if (numbers.contains(number) && !duplicate_numbers.contains(number))
                    duplicate_numbers << number;
                else
                    numbers << number;
            }

            if (duplicate_numbers.isEmpty()) {
                room->broadcastSkillInvoke("buqu");
                room->setPlayerFlag(zhoutai, "-Global_Dying");
                return true;
            } else {
                LogMessage log;
                log.type = "#NosBuquDuplicate";
                log.from = zhoutai;
                log.arg = QString::number(duplicate_numbers.length());
                room->sendLog(log);

                for (int i = 0; i < duplicate_numbers.length(); i++) {
                    int number = duplicate_numbers.at(i);

                    LogMessage log;
                    log.type = "#NosBuquDuplicateGroup";
                    log.from = zhoutai;
                    log.arg = QString::number(i + 1);
                    if (number == 10)
                        log.arg2 = "10";
                    else {
                        const char *number_string = "-A23456789-JQK";
                        log.arg2 = QString(number_string[number]);
                    }
                    room->sendLog(log);

                    foreach (int card_id, nosbuqu) {
                        const Card *card = Sanguosha->getCard(card_id);
                        if (card->getNumber() == number) {
                            LogMessage log;
                            log.type = "$NosBuquDuplicateItem";
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

class NosBuquClear: public DetachEffectSkill {
public:
    NosBuquClear(): DetachEffectSkill("nosbuqu") {
    }

    virtual void onSkillDetached(Room *room, ServerPlayer *player) const{
        if (player->getHp() <= 0)
            room->enterDying(player, NULL);
    }
};


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCommandLinkButton>

GuhuoDialog *GuhuoDialog::getInstance(const QString &object, bool left, bool right) {
    static GuhuoDialog *instance;
    if (instance == NULL || instance->objectName() != object)
        instance = new GuhuoDialog(object, left, right);

    return instance;
}

GuhuoDialog::GuhuoDialog(const QString &object, bool left, bool right): object_name(object) {
    setObjectName(object);
    setWindowTitle(Sanguosha->translate(object));
    group = new QButtonGroup(this);

    QHBoxLayout *layout = new QHBoxLayout;
    if (left) layout->addWidget(createLeft());
    if (right) layout->addWidget(createRight());
    setLayout(layout);

    connect(group, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(selectCard(QAbstractButton *)));
}

void GuhuoDialog::popup() {
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_PLAY)
        return;

    foreach (QAbstractButton *button, group->buttons()) {
        const Card *card = map[button->objectName()];
        bool enabled = !Self->isCardLimited(card, Card::MethodUse, true) && card->isAvailable(Self);
        button->setEnabled(enabled);
    }

    Self->tag.remove(object_name);
    exec();
}

void GuhuoDialog::selectCard(QAbstractButton *button) {
    CardStar card = map.value(button->objectName());
    Self->tag[object_name] = QVariant::fromValue(card);
    if (button->objectName().contains("slash")) {
        if (objectName() == "guhuo")
            Self->tag["GuhuoSlash"] = button->objectName();
        else if (objectName() == "nosguhuo")
            Self->tag["NosGuhuoSlash"] = button->objectName();
    }
    emit onButtonClick();
    accept();
}

QGroupBox *GuhuoDialog::createLeft() {
    QGroupBox *box = new QGroupBox;
    box->setTitle(Sanguosha->translate("basic"));

    QVBoxLayout *layout = new QVBoxLayout;

    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach (const Card *card, cards) {
        if (card->getTypeId() == Card::TypeBasic && !map.contains(card->objectName())
            && !Config.BanPackages.contains(card->getPackage())) {
            Card *c = Sanguosha->cloneCard(card->objectName());
            c->setParent(this);
            layout->addWidget(createButton(c));

            if (card->objectName() == "slash" && !Config.BanPackages.contains("maneuvering") && objectName() != "gudan") {
                Card *c2 = Sanguosha->cloneCard(card->objectName());
                c2->setParent(this);
                layout->addWidget(createButton(c2));
            }
        }
    }

    layout->addStretch();
    box->setLayout(layout);
    return box;
}

QGroupBox *GuhuoDialog::createRight() {
    QGroupBox *box = new QGroupBox(Sanguosha->translate("ndtrick"));
    QHBoxLayout *layout = new QHBoxLayout;

    QGroupBox *box1 = new QGroupBox(Sanguosha->translate("single_target_trick"));
    QVBoxLayout *layout1 = new QVBoxLayout;

    QGroupBox *box2 = new QGroupBox(Sanguosha->translate("multiple_target_trick"));
    QVBoxLayout *layout2 = new QVBoxLayout;


    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach (const Card *card, cards) {
        if (card->isNDTrick() && !map.contains(card->objectName())
            && !Config.BanPackages.contains(card->getPackage())) {
            Card *c = Sanguosha->cloneCard(card->objectName());
            c->setSkillName(object_name);
            c->setParent(this);

            QVBoxLayout *layout = c->isKindOf("SingleTargetTrick") ? layout1 : layout2;
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

QAbstractButton *GuhuoDialog::createButton(const Card *card) {
    if (card->objectName() == "slash" && map.contains(card->objectName()) 
        && !map.contains("normal_slash") && objectName() != "gudan") {
        QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate("normal_slash"));
        button->setObjectName("normal_slash");
        button->setToolTip(card->getDescription());

        map.insert("normal_slash", card);
        group->addButton(button);

        return button;
    } else {
        QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate(card->objectName()));
        button->setObjectName(card->objectName());
        button->setToolTip(card->getDescription());

        map.insert(card->objectName(), card);
        group->addButton(button);

        return button;
    }
}

WindPackage::WindPackage()
    :Package("wind")
{
    General *caoren = new General(this, "caoren", "wei"); // WEI 011
    caoren->addSkill(new Jushou);

    General *weiyan = new General(this, "weiyan", "shu"); // SHU 009
    weiyan->addSkill(new Kuanggu);

    General *nos_zhoutai = new General(this, "nos_zhoutai", "wu");
    nos_zhoutai->addSkill(new NosBuqu);
    nos_zhoutai->addSkill(new NosBuquRemove);
    nos_zhoutai->addSkill(new NosBuquClear);
    related_skills.insertMulti("nosbuqu", "#nosbuqu-remove");
    related_skills.insertMulti("nosbuqu", "#nosbuqu-clear");


    skills << new KuangguGlobal;
}

ADD_PACKAGE(Wind)