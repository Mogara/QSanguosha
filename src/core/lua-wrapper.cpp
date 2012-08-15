#include "lua-wrapper.h"

LuaTriggerSkill::LuaTriggerSkill(const char *name, Frequency frequency)
    :TriggerSkill(name), on_trigger(0), can_trigger(0), priority(1)
{
    this->frequency = frequency;
}

void LuaTriggerSkill::addEvent(TriggerEvent event){
    events << event;
}

void LuaTriggerSkill::setViewAsSkill(ViewAsSkill *view_as_skill){
    this->view_as_skill = view_as_skill;
}

int LuaTriggerSkill::getPriority() const{
    return priority;
}

LuaProhibitSkill::LuaProhibitSkill(const char *name)
    :ProhibitSkill(name), is_prohibited(0)
{

}

LuaViewAsSkill::LuaViewAsSkill(const char *name)
    :ViewAsSkill(name), view_filter(0), view_as(0),
      enabled_at_play(0), enabled_at_response(0), enabled_at_nullification(0)
{

}

LuaFilterSkill::LuaFilterSkill(const char *name)
    :FilterSkill(name), view_filter(0), view_as(0)
{

}

LuaDistanceSkill::LuaDistanceSkill(const char *name)
    :DistanceSkill(name), correct_func(0)
{

}

LuaMaxCardsSkill::LuaMaxCardsSkill(const char *name)
    :MaxCardsSkill(name), extra_func(0)
{

}

static QHash<QString, const LuaSkillCard *> LuaSkillCards;

LuaSkillCard::LuaSkillCard(const char *name)
    :SkillCard(), filter(0), feasible(0), on_use(0), on_effect(0)
{
    if(name){
        LuaSkillCards.insert(name, this);
        setObjectName(name);
    }
}

LuaSkillCard *LuaSkillCard::clone() const{
    LuaSkillCard *new_card = new LuaSkillCard(NULL);

    new_card->setObjectName(objectName());

    new_card->target_fixed = target_fixed;
    new_card->will_throw = will_throw;

    new_card->filter = filter;
    new_card->feasible = feasible;
    new_card->on_use = on_use;
    new_card->on_effect = on_effect;

    return new_card;
}

void LuaSkillCard::setTargetFixed(bool target_fixed){
    this->target_fixed = target_fixed;
}

void LuaSkillCard::setWillThrow(bool will_throw){
    this->will_throw = will_throw;;
}

LuaSkillCard *LuaSkillCard::Parse(const QString &str){
    QRegExp rx("#(\\w+):(.*):(.*)");
    QRegExp e_rx("#(\\w*)\\[(\\w+):(.+)\\]:(.*):(.*)");

    static QMap<QString, Card::Suit> suit_map;
    if(suit_map.isEmpty()){
        suit_map.insert("spade", Card::Spade);
        suit_map.insert("club", Card::Club);
        suit_map.insert("heart", Card::Heart);
        suit_map.insert("diamond", Card::Diamond);
        suit_map.insert("no_suit", Card::NoSuit);
    }

    QStringList texts;
    QString name, suit, number;
    QString subcard_str;
    QString user_string;

    if(rx.exactMatch(str)){
        texts = rx.capturedTexts();
        name = texts.at(1);
        subcard_str = texts.at(2);
        user_string = texts.at(3);
    }
    else if(e_rx.exactMatch(str)){
        texts = e_rx.capturedTexts();
        name = texts.at(1);
        suit = texts.at(2);
        number = texts.at(3);
        subcard_str = texts.at(4);
        user_string = texts.at(5);
    }
    else
        return NULL;

    const LuaSkillCard *c = LuaSkillCards.value(name, NULL);
    if(c == NULL)
        return NULL;

    LuaSkillCard *new_card = c->clone();

    if(subcard_str != "."){
        foreach(QString subcard, subcard_str.split("+")){
            new_card->addSubcard(subcard.toInt());
        }
    }
    if(!suit.isEmpty())
        new_card->setSuit(suit_map.value(suit, Card::NoSuit));
    if(!number.isEmpty()){
        int num = 0;
        if(number == "A")
            num = 1;
        else if(number == "J")
            num = 11;
        else if(number == "Q")
            num = 12;
        else if(number == "K")
            num = 13;
        else
            num = number.toInt();

        new_card->setNumber(num);
    }

    new_card->setUserString(user_string);
    new_card->setSkillName(name);
    return new_card;
}

QString LuaSkillCard::toString() const{
    return QString("#%1[%2:%3]:%4:%5").arg(objectName())
            .arg(getSuitString()).arg(getNumberString())
            .arg(subcardString()).arg(user_string);
}

