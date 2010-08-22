#include "skill.h"
#include "engine.h"
#include "player.h"
#include "room.h"

#include <QFile>

Skill::Skill(const QString &name)
{
    static QChar lord_symbol('$');

    if(name.endsWith(lord_symbol)){
        QString copy = name;
        copy.remove(lord_symbol);
        setObjectName(copy);
        lord_skill = true;
    }else{
        setObjectName(name);
        lord_skill = false;
    }
}

bool Skill::isLordSkill() const{
    return lord_skill;
}

QString Skill::getDescription() const{
    return Sanguosha->translate(":" + objectName());
}

void Skill::initMediaSource(){
    sources.clear();

    if(parent()){
        const General *general = qobject_cast<const General *>(parent());
        QString package_name = general->parent()->objectName();

        QString effect_file = QString("%1/generals/effect/%2.wav").arg(package_name).arg(objectName());
        if(QFile::exists(effect_file))
            sources << Phonon::MediaSource(effect_file);
        else{
            int i=1;
            forever{
                QString effect_file = QString("%1/generals/effect/%2%3.wav").arg(package_name).arg(objectName()).arg(i);
                if(QFile::exists(effect_file))
                    sources << Phonon::MediaSource(effect_file);
                else
                    break;
                i++;
            }
        }
    }
}

void Skill::playEffect(int index) const{
    if(!sources.isEmpty()){
        if(index == -1)
            index = qrand() % sources.length();
        else
            index--;

        Sanguosha->playEffect(sources.at(index));
    }
}

ViewAsSkill::ViewAsSkill(const QString &name, bool disable_after_use)
    :Skill(name), disable_after_use(disable_after_use)
{

}

void ViewAsSkill::attachPlayer(Player *player) const{
    if(parent()->objectName() == player->getGeneral())
        player->attachSkill(this);
}

bool ViewAsSkill::isDisableAfterUse() const{
    return disable_after_use;
}

ZeroCardViewAsSkill::ZeroCardViewAsSkill(const QString &name, bool disable_after_use)
    :ViewAsSkill(name, disable_after_use)
{

}

const Card *ZeroCardViewAsSkill::viewAs(const QList<CardItem *> &cards) const{
    if(cards.isEmpty())
        return viewAs();
    else
        return NULL;
}

bool ZeroCardViewAsSkill::viewFilter(const QList<CardItem *> &, const CardItem *) const{
    return false;
}

FilterSkill::FilterSkill(const QString &name)
    :ViewAsSkill(name, false)
{
}

PassiveSkill::PassiveSkill(const QString &name, Frequency frequency)
    :Skill(name), frequency(frequency)
{

}

int PassiveSkill::getPriority(ServerPlayer *target) const{
    // FIXME

    return 0;
}

bool PassiveSkill::triggerable(const ServerPlayer *target) const{
    return target->isAlive() && target->hasSkill(objectName());
}

PassiveSkill::Frequency PassiveSkill::getFrequency() const{
    return frequency;
}

void PassiveSkill::onOption(ServerPlayer *target, const QString &option) const{

}

void PassiveSkill::enqueueInvoke(ServerPlayer *target) const{
    Room *room = target->getRoom();

    ActiveRecord *record = new ActiveRecord;
    record->method = "@InvokeSkill";
    record->data = objectName();
    record->target = target;

    room->enqueueRecord(record);
}

MasochismSkill::MasochismSkill(const QString &name)
    :PassiveSkill(name)
{

}

void MasochismSkill::getTriggerEvents(QList<Room::TriggerEvent> &events) const{
    events << Room::Damaged;
}

bool MasochismSkill::trigger(Room::TriggerEvent, ServerPlayer *player, const QVariant &data) const{
    DamageStruct damage = data.value<DamageStruct>();

    onDamaged(player, damage);

    return false;
}

PhaseChangeSkill::PhaseChangeSkill(const QString &name)
    :PassiveSkill(name)
{
}

void PhaseChangeSkill::getTriggerEvents(QList<Room::TriggerEvent> &events) const{
    events << Room::PhaseChange;
}

bool PhaseChangeSkill::trigger(Room::TriggerEvent, ServerPlayer *player, const QVariant &) const{
    return onPhaseChange(player);
}

EnvironSkill::EnvironSkill(const QString &name)
    :PassiveSkill(name)
{
}

void EnvironSkill::getTriggerEvents(QList<Room::TriggerEvent> &events) const{
    events << Room::GameStart;
}

