#include "skill.h"
#include "engine.h"
#include "player.h"

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

void Skill::playEffect() const{
    if(!sources.isEmpty()){
        int r = qrand() % sources.length();
        Sanguosha->playEffect(sources.at(r));
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

PassiveSkill::PassiveSkill(const QString &name)
    :Skill(name)
{

}

ActiveRecord *PassiveSkill::onGameStart(ServerPlayer *target) const{
    return NULL;
}

ActiveRecord *PassiveSkill::onPhaseChange(ServerPlayer *target) const{
    return NULL;
}

FrequentPassiveSkill::FrequentPassiveSkill(const QString &name)
    :PassiveSkill(name)
{

}

EnvironSkill::EnvironSkill(const QString &name)
    :Skill(name)
{

}
