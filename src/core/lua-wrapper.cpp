#include "lua-wrapper.h"
#include "util.h"

LuaTriggerSkill::LuaTriggerSkill(const char *name, Frequency frequency, const char *limit_mark)
    : TriggerSkill(name), on_trigger(0), can_trigger(0), priority(2)
{
    this->frequency = frequency;
    this->limit_mark = QString(limit_mark);
}

int LuaTriggerSkill::getPriority() const{
    return priority;
}

LuaProhibitSkill::LuaProhibitSkill(const char *name)
    : ProhibitSkill(name), is_prohibited(0)
{
}

LuaViewAsSkill::LuaViewAsSkill(const char *name, const char *response_pattern)
    : ViewAsSkill(name), view_filter(0), view_as(0),
      enabled_at_play(0), enabled_at_response(0), enabled_at_nullification(0)
{
    this->response_pattern = response_pattern;
}

LuaFilterSkill::LuaFilterSkill(const char *name)
    : FilterSkill(name), view_filter(0), view_as(0)
{
}

LuaDistanceSkill::LuaDistanceSkill(const char *name)
    : DistanceSkill(name), correct_func(0)
{
}

LuaMaxCardsSkill::LuaMaxCardsSkill(const char *name)
    : MaxCardsSkill(name), extra_func(0), fixed_func(0)
{
}

LuaTargetModSkill::LuaTargetModSkill(const char *name, const char *pattern)
    : TargetModSkill(name), residue_func(0), distance_limit_func(0), extra_target_func(0)
{
    this->pattern = pattern;
}

static QHash<QString, const LuaSkillCard *> LuaSkillCards;
static QHash<QString, QString> LuaSkillCardsSkillName;

LuaSkillCard::LuaSkillCard(const char *name, const char *skillName)
    : SkillCard(), filter(0), feasible(0),
      about_to_use(0), on_use(0), on_effect(0), on_validate(0), on_validate_in_response(0)
{
    if (name) {
        LuaSkillCards.insert(name, this);
        if (skillName) {
            m_skillName = skillName;
            LuaSkillCardsSkillName.insert(name, skillName);
        }
        setObjectName(name);
    }
}

LuaSkillCard *LuaSkillCard::clone() const{
    LuaSkillCard *new_card = new LuaSkillCard(NULL, NULL);

    new_card->setObjectName(objectName());
    new_card->setSkillName(m_skillName);

    new_card->target_fixed = target_fixed;
    new_card->will_throw = will_throw;
    new_card->can_recast = can_recast;
    new_card->handling_method = handling_method;

    new_card->filter = filter;
    new_card->feasible = feasible;
    new_card->about_to_use = about_to_use;
    new_card->on_use = on_use;
    new_card->on_effect = on_effect;
    new_card->on_validate = on_validate;
    new_card->on_validate_in_response = on_validate_in_response;

    return new_card;
}

LuaSkillCard *LuaSkillCard::Parse(const QString &str) {
    QRegExp rx("#(\\w+):(.*):(.*)");
    QRegExp e_rx("#(\\w*)\\[(\\w+):(.+)\\]:(.*):(.*)");

    static QMap<QString, Card::Suit> suit_map;
    if (suit_map.isEmpty()) {
        suit_map.insert("spade", Card::Spade);
        suit_map.insert("club", Card::Club);
        suit_map.insert("heart", Card::Heart);
        suit_map.insert("diamond", Card::Diamond);
        suit_map.insert("no_suit_red", Card::NoSuitRed);
        suit_map.insert("no_suit_black", Card::NoSuitBlack);
        suit_map.insert("no_suit", Card::NoSuit);
    }

    QStringList texts;
    QString name, suit, number;
    QString subcard_str;
    QString user_string;

    if (rx.exactMatch(str)) {
        texts = rx.capturedTexts();
        name = texts.at(1);
        subcard_str = texts.at(2);
        user_string = texts.at(3);
    } else if (e_rx.exactMatch(str)) {
        texts = e_rx.capturedTexts();
        name = texts.at(1);
        suit = texts.at(2);
        number = texts.at(3);
        subcard_str = texts.at(4);
        user_string = texts.at(5);
    } else
        return NULL;

    const LuaSkillCard *c = LuaSkillCards.value(name, NULL);
    if (c == NULL)
        return NULL;

    LuaSkillCard *new_card = c->clone();

    if (subcard_str != ".")
        new_card->addSubcards(StringList2IntList(subcard_str.split("+")));

    if (!suit.isEmpty())
        new_card->setSuit(suit_map.value(suit, Card::NoSuit));
    if (!number.isEmpty()) {
        int num = 0;
        if (number == "A")
            num = 1;
        else if (number == "J")
            num = 11;
        else if (number == "Q")
            num = 12;
        else if (number == "K")
            num = 13;
        else
            num = number.toInt();

        new_card->setNumber(num);
    }

    new_card->setUserString(user_string);
    QString skillName = LuaSkillCardsSkillName.value(name, QString());
    if (skillName.isEmpty())
        skillName = name.toLower().remove("card");
    new_card->setSkillName(skillName);
    return new_card;
}

QString LuaSkillCard::toString(bool hidden) const{
    Q_UNUSED(hidden);
    return QString("#%1[%2:%3]:%4:%5").arg(objectName())
           .arg(getSuitString()).arg(getNumberString())
           .arg(subcardString()).arg(user_string);
}

LuaBasicCard::LuaBasicCard(Card::Suit suit, int number, const char *obj_name, const char *class_name, const char *subtype)
    : BasicCard(suit, number), filter(0), feasible(0), available(0), about_to_use(0), on_use(0), on_effect(0)
{
    setObjectName(obj_name);
    this->class_name = class_name;
    this->subtype = subtype;
}

LuaBasicCard *LuaBasicCard::clone(Card::Suit suit, int number) const{
    if (suit == Card::SuitToBeDecided) suit = this->getSuit();
    if (number == -1) number = this->getNumber();
    LuaBasicCard *new_card = new LuaBasicCard(suit, number, objectName().toStdString().c_str(), class_name.toStdString().c_str(), subtype.toStdString().c_str());
    new_card->subtype = subtype;

    new_card->target_fixed = target_fixed;
    new_card->can_recast = can_recast;

    new_card->filter = filter;
    new_card->feasible = feasible;
    new_card->available = available;
    new_card->about_to_use = about_to_use;
    new_card->on_use = on_use;
    new_card->on_effect = on_effect;

    return new_card;
}

LuaTrickCard::LuaTrickCard(Card::Suit suit, int number, const char *obj_name, const char *class_name, const char *subtype)
    : TrickCard(suit, number), filter(0), feasible(0), available(0), is_cancelable(0),
      about_to_use(0), on_use(0), on_effect(0), on_nullified(0)
{
    setObjectName(obj_name);
    this->class_name = class_name;
    this->subtype = subtype;
}

LuaTrickCard *LuaTrickCard::clone(Card::Suit suit, int number) const{
    if (suit == Card::SuitToBeDecided) suit = this->getSuit();
    if (number == -1) number = this->getNumber();
    LuaTrickCard *new_card = new LuaTrickCard(suit, number, objectName().toStdString().c_str(), class_name.toStdString().c_str(), subtype.toStdString().c_str());
    new_card->subclass = subclass;
    new_card->subtype = subtype;

    new_card->target_fixed = target_fixed;
    new_card->can_recast = can_recast;

    new_card->filter = filter;
    new_card->feasible = feasible;
    new_card->available = available;
    new_card->is_cancelable = is_cancelable;
    new_card->about_to_use = about_to_use;
    new_card->on_use = on_use;
    new_card->on_effect = on_effect;
    new_card->on_nullified = on_nullified;

    return new_card;
}

LuaWeapon::LuaWeapon(Card::Suit suit, int number, int range, const char *obj_name, const char *class_name)
    : Weapon(suit, number, range)
{
    setObjectName(obj_name);
    this->class_name = class_name;
}

LuaWeapon *LuaWeapon::clone(Card::Suit suit, int number) const{
    if (suit == Card::SuitToBeDecided) suit = this->getSuit();
    if (number == -1) number = this->getNumber();
    LuaWeapon *new_card = new LuaWeapon(suit, number, this->getRange(), objectName().toStdString().c_str(), class_name.toStdString().c_str());

    new_card->on_install = on_install;
    new_card->on_uninstall = on_uninstall;

    return new_card;
}

LuaArmor::LuaArmor(Card::Suit suit, int number, const char *obj_name, const char *class_name)
    : Armor(suit, number)
{
    setObjectName(obj_name);
    this->class_name = class_name;
}

LuaArmor *LuaArmor::clone(Card::Suit suit, int number) const{
    if (suit == Card::SuitToBeDecided) suit = this->getSuit();
    if (number == -1) number = this->getNumber();
    LuaArmor *new_card = new LuaArmor(suit, number, objectName().toStdString().c_str(), class_name.toStdString().c_str());

    new_card->on_install = on_install;
    new_card->on_uninstall = on_uninstall;

    return new_card;
}
