#include "roomthread.h"
#include "room.h"
#include "engine.h"
#include "gamerule.h"
#include "ai.h"

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

bool TriggerSkillSorter::operator ()(const TriggerSkill *a, const TriggerSkill *b){
    int x = a->getPriority();
    int y = b->getPriority();

    return x > y;
}

void TriggerSkillSorter::sort(QList<const TriggerSkill *> &skills){
    qStableSort(skills.begin(), skills.end(), *this);
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
    :slash(NULL), from(NULL), to(NULL), drank(false), nature(DamageStruct::Normal)
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

JudgeStruct::JudgeStruct()
    :who(NULL), card(NULL), good(true)
{

}

bool JudgeStruct::isGood(const Card *card) const{
    if(card == NULL)
        card = this->card;

    QString class_name = card->metaObject()->className();
    Card::Suit suit = card->getSuit();
    if(who->hasSkill("hongyan") && suit == Card::Spade)
        suit = Card::Heart;

    QString number = card->getNumberString();
    QString card_str = QString("%1:%2:%3").arg(class_name).arg(Card::Suit2String(suit)).arg(number);

    if(good)
        return pattern.exactMatch(card_str);
    else
        return !pattern.exactMatch(card_str);
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

RoomThread::RoomThread(Room *room)
    :QThread(room), room(room)
{
}

void RoomThread::addPlayerSkills(ServerPlayer *player, bool invoke_game_start){
    QVariant void_data;

    foreach(const TriggerSkill *skill, player->getTriggerSkills()){
        if(skill->isLordSkill()){
            if(!player->isLord() || room->mode == "06_3v3")
                continue;
        }

        addTriggerSkill(skill);

        if(invoke_game_start && skill->getTriggerEvents().contains(GameStart))
            skill->trigger(GameStart, player, void_data);
    }
}

void RoomThread::removePlayerSkills(ServerPlayer *player){
    foreach(const TriggerSkill *skill, player->getTriggerSkills()){
        if(skill->isLordSkill()){
            if(!player->isLord() || room->mode == "06_3v3")
                continue;
        }

        removeTriggerSkill(skill);
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
    // start game, draw initial 4 cards
    foreach(ServerPlayer *player, room->players){
        trigger(GameStart, player);
    }

    if(setjmp(env) == GameOver)
        return;

    if(room->mode == "06_3v3"){
        run3v3();
    }else{
        if(room->getMode() == "02_1v1")
            room->setCurrent(room->players.at(1));

        forever{
            trigger(TurnStart, room->getCurrent());
            room->setCurrent(room->getCurrent()->getNextAlive());
        }
    }
}

bool RoomThread::trigger(TriggerEvent event, ServerPlayer *target, QVariant &data){
    Q_ASSERT(QThread::currentThread() == this);

    QList<const TriggerSkill *> skills = skill_table[event];
    QMutableListIterator<const TriggerSkill *> itor(skills);
    while(itor.hasNext()){
        const TriggerSkill *skill = itor.next();
        if(!skill->triggerable(target))
            itor.remove();
    }

    TriggerSkillSorter sorter;
    sorter.sort(skills);

    bool broken = false;
    foreach(const TriggerSkill *skill, skills){
        if(skill->trigger(event, target, data)){
            broken = true;
            break;
        }
    }

    foreach(AI *ai, room->ais)
        ai->filterEvent(event, target, data);

    return broken;
}

bool RoomThread::trigger(TriggerEvent event, ServerPlayer *target){
    QVariant data;
    return trigger(event, target, data);
}

void RoomThread::addTriggerSkill(const TriggerSkill *skill){
    int count = refcount.value(skill, 0);
    if(count != 0){
        refcount[skill] ++;
        return;
    }else
        refcount[skill] = 1;

    QList<TriggerEvent> events = skill->getTriggerEvents();
    foreach(TriggerEvent event, events){
        skill_table[event] << skill;
    }
}

void RoomThread::removeTriggerSkill(const QString &skill_name){
    const TriggerSkill *skill = Sanguosha->getTriggerSkill(skill_name);
    if(skill)
        removeTriggerSkill(skill);
}

void RoomThread::removeTriggerSkill(const TriggerSkill *skill){
    int count = refcount.value(skill, 0);
    if(count > 1){
        refcount[skill] --;
        return;
    }else
        refcount.remove(skill);

    QList<TriggerEvent> events = skill->getTriggerEvents();
    foreach(TriggerEvent event, events){
        skill_table[event].removeOne(skill);
    }
}

void RoomThread::delay(unsigned long secs){
    msleep(secs);
}

void RoomThread::end(){
    longjmp(env, GameOver);
}
