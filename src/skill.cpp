#include "skill.h"
#include "engine.h"
#include "player.h"
#include "room.h"
#include "client.h"

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

ViewAsSkill::ViewAsSkill(const QString &name)
    :Skill(name)
{

}

bool ViewAsSkill::isAvailable() const{
    switch(ClientInstance->getStatus()){
    case Client::Playing: return isEnabledAtPlay();
    case Client::Responsing: return isEnabledAtResponse();
    default:
        return false;
    }
}

bool ViewAsSkill::isEnabledAtPlay() const{
    return true;
}

bool ViewAsSkill::isEnabledAtResponse() const{
    return false;
}

ZeroCardViewAsSkill::ZeroCardViewAsSkill(const QString &name)
    :ViewAsSkill(name)
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
    :ViewAsSkill(name)
{
}

PassiveSkill::PassiveSkill(const QString &name, Frequency frequency)
    :Skill(name), frequency(frequency)
{

}

int PassiveSkill::getPriority(ServerPlayer *target) const{    
    return 1;
}

bool PassiveSkill::triggerable(const ServerPlayer *target) const{
    return target->isAlive() && target->hasSkill(objectName());
}

PassiveSkill::Frequency PassiveSkill::getFrequency() const{
    return frequency;
}

MasochismSkill::MasochismSkill(const QString &name)
    :PassiveSkill(name)
{

}

void MasochismSkill::getTriggerEvents(QList<TriggerEvent> &events) const{
    events << Damaged;
}

bool MasochismSkill::trigger(TriggerEvent, ServerPlayer *player, const QVariant &data) const{
    DamageStruct damage = data.value<DamageStruct>();

    onDamaged(player, damage);

    return false;
}

PhaseChangeSkill::PhaseChangeSkill(const QString &name)
    :PassiveSkill(name)
{
}

void PhaseChangeSkill::getTriggerEvents(QList<TriggerEvent> &events) const{
    events << PhaseChange;
}

bool PhaseChangeSkill::trigger(TriggerEvent, ServerPlayer *player, const QVariant &) const{
    return onPhaseChange(player);
}

EnvironSkill::EnvironSkill(const QString &name)
    :PassiveSkill(name)
{
}

void EnvironSkill::getTriggerEvents(QList<TriggerEvent> &events) const{
    events << GameStart;
}

