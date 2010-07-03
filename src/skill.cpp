#include "skill.h"
#include "engine.h"

Skill::Skill(const QString &name, const QScriptValue &value, QObject *parent)
    :QObject(parent), value(value)
{
    static QChar lord_symbol('$');
    static QChar frequent_symbol('+');
    static QChar compulsory_symbol('!');

    QString copy = name;

    setBooleanFlag(copy, lord_symbol, &lord_skill);
    setBooleanFlag(copy, compulsory_symbol, &compulsory);
    if(!compulsory)
        setBooleanFlag(copy, frequent_symbol, &frequent);

    setObjectName(copy);
}

void Skill::doCallback(Room *room){
    QScriptValue this_object = Sanguosha->newQObject(room);
    QScriptValueList arguments;
    value.call(this_object, arguments);
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

void Skill::setBooleanFlag(QString &str, QChar symbol, bool *flag){
    if(str.contains(symbol)){
        str.remove(symbol);
        *flag = true;
    }else
        *flag = false;
}
