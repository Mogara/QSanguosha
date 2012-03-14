#include "roomthread.h"
#include "room.h"
#include "engine.h"
#include "gamerule.h"
#include "ai.h"
#include "settings.h"

#include <QTime>

LogMessage::LogMessage()
    :from(NULL)
{
}

QString LogMessage::toString() const{
    QStringList tos;
    foreach(ServerPlayer *player, to)
        tos << player->objectName();

    return QString("%1:%2->%3:%4:%5:%6")
            .arg(type)
            .arg(from ? from->objectName() : "")
            .arg(tos.join("+"))
            .arg(card_str).arg(arg).arg(arg2);
}

DamageStruct::DamageStruct()
    :from(NULL), to(NULL), card(NULL), damage(1), nature(Normal), chain(false)
{
}

CardEffectStruct::CardEffectStruct()
    :card(NULL), from(NULL), to(NULL), multiple(false)
{
}

SlashEffectStruct::SlashEffectStruct()
    :slash(NULL), jink(NULL), from(NULL), to(NULL), drank(false), nature(DamageStruct::Normal)
{
}

DyingStruct::DyingStruct()
    :who(NULL), damage(NULL)
{
}

RecoverStruct::RecoverStruct()
    :recover(1), who(NULL), card(NULL)
{

}

PindianStruct::PindianStruct()
    :from(NULL), to(NULL), from_card(NULL), to_card(NULL)
{

}

bool PindianStruct::isSuccess() const{
    return from_card->getNumber() > to_card->getNumber();
}

JudgeStructPattern::JudgeStructPattern(){
}

bool JudgeStructPattern::match(const Player *player, const Card *card) const{
    if(pattern.isEmpty())
        return false;

    if(isRegex){
        QString class_name = card->metaObject()->className();
        Card::Suit suit = card->getSuit();
        if(player->hasSkill("hongyan") && suit == Card::Spade)
            suit = Card::Heart;

        QString number = card->getNumberString();
        QString card_str = QString("%1:%2:%3").arg(class_name).arg(Card::Suit2String(suit)).arg(number);

        return QRegExp(pattern).exactMatch(card_str);
    }else{
        const CardPattern *card_pattern = Sanguosha->getPattern(pattern);
        return card_pattern && card_pattern->match(player, card);
    }
}

JudgeStructPattern &JudgeStructPattern::operator =(const QRegExp &rx){
    pattern = rx.pattern();
    isRegex = true;

    return *this;
}

JudgeStructPattern &JudgeStructPattern::operator =(const QString &str){
    pattern = str;
    isRegex = false;

    return *this;
}

JudgeStruct::JudgeStruct()
    :who(NULL), card(NULL), good(true)
{

}

bool JudgeStruct::isGood(const Card *card) const{
    if(card == NULL)
        card = this->card;

    if(good)
        return pattern.match(who, card);
    else
        return !pattern.match(who, card);
}

bool JudgeStruct::isBad() const{
    return ! isGood();
}

CardUseStruct::CardUseStruct()
    :card(NULL), from(NULL)
{
}

bool CardUseStruct::isValid() const{
    return card != NULL;
}

void CardUseStruct::parse(const QString &str, Room *room){
    QStringList words = str.split("->");
    QString card_str = words.at(0);
    QString target_str = words.at(1);

    card = Card::Parse(card_str);

    if(target_str != "."){
        QStringList target_names = target_str.split("+");
        foreach(QString target_name, target_names)
            to << room->findChild<ServerPlayer *>(target_name);
    }
}

QString EventTriplet::toString() const{
    return QString("event = %1, target = %2[%3], data = %4[%5]")
            .arg(event)
            .arg(target->objectName()).arg(target->getGeneralName())
            .arg(data->toString()).arg(data->typeName());
}

RoomThread::RoomThread(Room *room)
    :QThread(room), room(room)
{
}

void RoomThread::addPlayerSkills(ServerPlayer *player, bool invoke_game_start){
    QVariant void_data;

    foreach(const TriggerSkill *skill, player->getTriggerSkills()){
        addTriggerSkill(skill);

        if(invoke_game_start && skill->getTriggerEvents().contains(GameStart))
            skill->trigger(GameStart, player, void_data);
    }
}

void RoomThread::constructTriggerTable(const GameRule *rule){
    foreach(ServerPlayer *player, room->players){
        addPlayerSkills(player, false);
    }

    addTriggerSkill(rule);
}

static const int GameOver = 1;

void RoomThread::run3v3(){
    QList<ServerPlayer *> warm, cool;
    foreach(ServerPlayer *player, room->players){
        switch(player->getRoleEnum()){
        case Player::Lord: warm.prepend(player); break;
        case Player::Loyalist: warm.append(player); break;
        case Player::Renegade: cool.prepend(player); break;
        case Player::Rebel: cool.append(player); break;
        }
    }

    QString order = room->askForOrder(cool.first());
    QList<ServerPlayer *> *first, *second;

    if(order == "warm"){
        first = &warm;
        second = &cool;
    }else{
        first = &cool;
        second = &warm;
    }

    action3v3(first->first());

    forever{
        qSwap(first, second);

        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *player, *first){
            if(!player->hasFlag("actioned") && player->isAlive())
                targets << player;
        }

        ServerPlayer *to_action = room->askForPlayerChosen(first->first(), targets, "3v3-action");
        if(to_action){
            action3v3(to_action);

            if(to_action != first->first()){
                ServerPlayer *another;
                if(to_action == first->last())
                    another = first->at(1);
                else
                    another = first->last();

                if(!another->hasFlag("actioned") && another->isAlive())
                    action3v3(another);
            }
        }
    }
}

void RoomThread::action3v3(ServerPlayer *player){
    room->setCurrent(player);
    trigger(TurnStart, room->getCurrent());
    room->setPlayerFlag(player, "actioned");

    bool all_actioned = true;
    foreach(ServerPlayer *player, room->alive_players){
        if(!player->hasFlag("actioned")){
            all_actioned = false;
            break;
        }
    }

    if(all_actioned){
        foreach(ServerPlayer *player, room->alive_players){
            room->setPlayerFlag(player, "-actioned");
        }
    }
}

void RoomThread::run(){
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    if(setjmp(env) == GameOver){
        quit();
        return;
    }

    // start game, draw initial 4 cards
    foreach(ServerPlayer *player, room->players){
        trigger(GameStart, player);
    }

    if(room->mode == "06_3v3"){
        run3v3();
    }else if(room->getMode() == "04_1v3"){
        ServerPlayer *shenlvbu = room->getLord();
        if(shenlvbu->getGeneralName() == "shenlvbu1"){
            QList<ServerPlayer *> league = room->players;
            league.removeOne(shenlvbu);

            forever{
                foreach(ServerPlayer *player, league){
                    if(player->hasFlag("actioned"))
                        room->setPlayerFlag(player, "-actioned");
                }

                foreach(ServerPlayer *player, league){
                    room->setCurrent(player);
                    trigger(TurnStart, room->getCurrent());

                    if(!player->hasFlag("actioned"))
                        room->setPlayerFlag(player, "actioned");

                    if(shenlvbu->getGeneralName() == "shenlvbu2")
                        goto second_phase;

                    if(player->isAlive()){
                        room->setCurrent(shenlvbu);
                        trigger(TurnStart, room->getCurrent());

                        if(shenlvbu->getGeneralName() == "shenlvbu2")
                            goto second_phase;
                    }
                }
            }

        }else{
            second_phase:

            foreach(ServerPlayer *player, room->players){
                if(player != shenlvbu){
                    if(player->hasFlag("actioned"))
                        room->setPlayerFlag(player, "-actioned");

                    if(player->getPhase() != Player::NotActive){
                        room->setPlayerProperty(player, "phase", "not_active");
                        trigger(PhaseChange, player);
                    }
                }
            }

            room->setCurrent(shenlvbu);

            forever{
                trigger(TurnStart, room->getCurrent());
                room->setCurrent(room->getCurrent()->getNext());
            }
        }


    }else{
        if(room->getMode() == "02_1v1")
            room->setCurrent(room->players.at(1));

        forever{
            trigger(TurnStart, room->getCurrent());
            room->setCurrent(room->getCurrent()->getNextAlive());
        }
    }
}

static bool CompareByPriority(const TriggerSkill *a, const TriggerSkill *b){
    return a->getPriority() > b->getPriority();
}

bool RoomThread::trigger(TriggerEvent event, ServerPlayer *target, QVariant &data){
    Q_ASSERT(QThread::currentThread() == this);

    // push it to event stack
    EventTriplet triplet(event, target, &data);
    event_stack.push_back(triplet);

    bool broken = false;
    foreach(const TriggerSkill *skill, skill_table[event]){
        if(skill->triggerable(target)){
            broken = skill->trigger(event, target, data);
            if(broken)
                break;
        }
    }

    if(target){
        foreach(AI *ai, room->ais){
            ai->filterEvent(event, target, data);
        }
    }

    delay(1);
    // pop event stack
    event_stack.pop_back();

    return broken;
}

const QList<EventTriplet> *RoomThread::getEventStack() const{
    return &event_stack;
}

bool RoomThread::trigger(TriggerEvent event, ServerPlayer *target){
    QVariant data;
    return trigger(event, target, data);
}

void RoomThread::addTriggerSkill(const TriggerSkill *skill){
    if(skillSet.contains(skill))
        return;

    skillSet << skill;

    QList<TriggerEvent> events = skill->getTriggerEvents();
    foreach(TriggerEvent event, events){
        QList<const TriggerSkill *> &table = skill_table[event];

        table << skill;
        qStableSort(table.begin(), table.end(), CompareByPriority);
    }

    if(skill->isVisible()){
        foreach(const Skill *skill, Sanguosha->getRelatedSkills(skill->objectName())){
            const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
            addTriggerSkill(trigger_skill);
        }
    }
}

void RoomThread::delay(unsigned long secs){
    if(room->property("to_test").toString().isEmpty()&& Config.AIDelay>0)
        msleep(secs);
}

void RoomThread::end(){
    longjmp(env, GameOver);
}
