#include "skill.h"
#include "engine.h"

#include <QFile>

Skill::Skill(const QString &name)
{
    static QChar lord_symbol('$');
    static QChar frequent_symbol('+');
    static QChar compulsory_symbol('!');
    static QChar toggleable_symbol('=');

    QString copy = name;

    setBooleanFlag(copy, lord_symbol, &lord_skill);
    setBooleanFlag(copy, compulsory_symbol, &compulsory);
    if(!compulsory){
        setBooleanFlag(copy, frequent_symbol, &frequent);
        setBooleanFlag(copy, toggleable_symbol, &toggleable);
    }

    setObjectName(copy);
}

bool Skill::isCompulsory() const{
    return compulsory;
}

bool Skill::isLordSkill() const{
    return lord_skill;
}

bool Skill::isFrequent() const{
    return frequent;
}

bool Skill::isToggleable() const{
    return toggleable;
}

void Skill::setBooleanFlag(QString &str, QChar symbol, bool *flag){
    if(str.contains(symbol)){
        str.remove(symbol);
        *flag = true;
    }else
        *flag = false;
}

QString Skill::getDescription() const{
    return Sanguosha->translate(":" + objectName());
}

void Skill::attachPlayer(Player *player) const{
    if(parent()->objectName() == player->getGeneral())
        player->attachSkill(this);
}

void Skill::trigger(Client *client, TriggerReason reason, const QVariant &data) const{

}

void Skill::trigger(Room *room) const{

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

ViewAsSkill::ViewAsSkill(const QString &name, int min, int max, bool include_equip, bool disable_after_use)
    :Skill(name), min(min), max(max), include_equip(include_equip), disable_after_use(disable_after_use)
{

}

void ViewAsSkill::attachPlayer(Player *player) const{
    // FIXME
}

bool ViewAsSkill::isDisableAfterUse() const{
    return disable_after_use;
}
