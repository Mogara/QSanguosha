#include "roomthread.h"
#include "room.h"
#include "engine.h"
#include "gamerule.h"

bool TriggerSkillSorter::operator ()(const TriggerSkill *a, const TriggerSkill *b){
    int x = a->getPriority(target);
    int y = b->getPriority(target);

    return x > y;
}

void TriggerSkillSorter::sort(QList<const TriggerSkill *> &skills){
    qSort(skills.begin(), skills.end(), *this);
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

SlashResultStruct::SlashResultStruct()
    :slash(NULL), from(NULL), to(NULL), nature(DamageStruct::Normal),
    drank(false), success(false)
{
}

void SlashResultStruct::fill(const SlashEffectStruct &effect, bool success)
{
    slash = effect.slash;
    from = effect.from;
    to = effect.to;
    nature = effect.nature;
    drank = effect.drank;
    this->success = success;
}

DyingStruct::DyingStruct()
    :damage(NULL), peaches(0)
{

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

void RoomThread::addPlayerSkills(ServerPlayer *player){
    const General *general = player->getGeneral();

    Q_ASSERT(general);

    QList<const TriggerSkill *> skills = general->findChildren<const TriggerSkill *>();
    foreach(const TriggerSkill *skill, skills){
        if(skill->isLordSkill() && !player->isLord())
            continue;

        addTriggerSkill(skill);
    }
}

void RoomThread::removePlayerSkills(ServerPlayer *player){
    const General *general = player->getGeneral();

    Q_ASSERT(general);

    QList<const TriggerSkill *> skills = general->findChildren<const TriggerSkill *>();
    foreach(const TriggerSkill *skill, skills){
        if(skill->isLordSkill() && !player->isLord())
            continue;

        removeTriggerSkill(skill);
    }
}

void RoomThread::constructTriggerTable(const GameRule *rule){
    foreach(ServerPlayer *player, room->players){
        addPlayerSkills(player);
    }   

    addTriggerSkill(rule);

    foreach(ServerPlayer *player, room->players){
        const General *general2 = player->getGeneral2();
        if(general2){
            QList<const Skill *> skills = general2->findChildren<const Skill *>();

            foreach(const Skill *skill, skills){
                if(!player->hasSkill(skill->objectName())){
                    if(player->getRoleEnum() != Player::Lord && skill->isLordSkill())
                        continue;

                    room->acquireSkill(player, skill);
                }
            }
        }
    }
}

static const int GameOver = 1;

void RoomThread::run(){
    // start game, draw initial 4 cards
    foreach(ServerPlayer *player, room->players){
        trigger(GameStart, player);
    }

    QList<Player::Phase> phases;
    phases << Player::Start << Player::Judge << Player::Draw
            << Player::Play << Player::Discard << Player::Finish
            << Player::NotActive;

    if(setjmp(env) == GameOver)
        return;

    forever{
        ServerPlayer *player = room->getCurrent();
        room->resetSkipSet();

        foreach(Player::Phase phase, phases){
            if(!room->isSkipped(phase)){
                player->setPhase(phase);
                room->broadcastProperty(player, "phase");

                trigger(PhaseChange, player);

                if(!player->isAlive())
                    break;
            }
        }

        room->nextPlayer();
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

    static TriggerSkillSorter sorter;

    sorter.target = target;
    sorter.sort(skills);

    foreach(const TriggerSkill *skill, skills){
        if(skill->trigger(event, target, data))
            return true;
    }

    return false;
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
