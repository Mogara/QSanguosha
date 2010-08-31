#include "roomthread.h"
#include "room.h"
#include "gamerule.h"

bool PassiveSkillSorter::operator ()(const PassiveSkill *a, const PassiveSkill *b){
    int x = a->getPriority(target);
    int y = b->getPriority(target);

    return x > y;
}

void PassiveSkillSorter::sort(QList<const PassiveSkill *> &skills){
    qSort(skills.begin(), skills.end(), *this);
}

DamageStruct::DamageStruct()
    :from(NULL), to(NULL), card(NULL), damage(0), nature(Normal)
{
}

RoomThread::RoomThread(Room *room)
    :QThread(room), room(room)
{
    foreach(ServerPlayer *player, room->players){
        const General *general = player->getGeneral();

        QList<const PassiveSkill *> skills = general->findChildren<const PassiveSkill *>();
        foreach(const PassiveSkill *skill, skills){
            passive_skills.insert(skill->objectName(), skill);
        }
    }
    GameRule *game_rule = new GameRule;
    passive_skills.insert(game_rule->objectName(), game_rule);

    // construct trigger_table
    foreach(const PassiveSkill *skill, passive_skills){
        QList<TriggerEvent> events;
        skill->getTriggerEvents(events);
        foreach(TriggerEvent event, events){
            trigger_table[event] << skill;            
        }
    }
}

void RoomThread::run(){
    // start game, draw initial cards
    foreach(ServerPlayer *player, room->players){
        invokePassiveSkills(GameStart, player);
    }

    room->changePhase(room->players.first());
}

bool RoomThread::invokePassiveSkills(TriggerEvent event, ServerPlayer *target, const QVariant &data){
    Q_ASSERT(QThread::currentThread() == this);

    QList<const PassiveSkill *> skills = trigger_table[event];    
    QMutableListIterator<const PassiveSkill *> itor(skills);
    while(itor.hasNext()){
        const PassiveSkill *skill = itor.next();
        if(!skill->triggerable(target))
            itor.remove();
    }

    static PassiveSkillSorter sorter;

    sorter.target = target;
    sorter.sort(skills);

    foreach(const PassiveSkill *skill, skills){
        if(skill->trigger(event, target, data))
            return true;
    }

    return false;
}
