#include "card.h"
#include "settings.h"
#include "engine.h"
#include "client.h"
#include "room.h"
#include "carditem.h"
#include "lua-wrapper.h"
#include <QFile>

const Card::Suit Card::AllSuits[4] = {
    Card::Spade,
    Card::Club,
    Card::Heart,
    Card::Diamond
};

Card::Card(Suit suit, int number, bool target_fixed)
    :target_fixed(target_fixed), once(false), mute(false), will_throw(true), owner_discarded(false)
	, has_preact(false), suit(suit), number(number), id(-1)
{
    can_jilei = will_throw;

    if(number < 1 || number > 13)
        number = 0;
}

QString Card::getSuitString() const{
    return Suit2String(suit);
}

QString Card::Suit2String(Suit suit){
    switch(suit){
    case Spade: return "spade";
    case Heart: return "heart";
    case Club: return "club";
    case Diamond: return "diamond";
    default: return "no_suit";
    }
}


QStringList Card::IdsToStrings(const QList<int> &ids){
    QStringList strings;
    foreach(int card_id, ids)
        strings << QString::number(card_id);
    return strings;
}

QList<int> Card::StringsToIds(const QStringList &strings){
    QList<int> ids;
    foreach(QString str, strings){
        bool ok;
        ids << str.toInt(&ok);

        if(!ok)
            break;
    }

    return ids;
}

bool Card::isRed() const{
    return suit == Heart || suit == Diamond;
}

bool Card::isBlack() const{
    return suit == Spade || suit == Club;
}

int Card::getId() const{
    return id;
}

void Card::setId(int id){
    this->id = id;
}

int Card::getEffectiveId() const{
    if(isVirtualCard()){
        if(subcards.isEmpty())
            return -1;
        else
            return subcards.first();
    }else
        return id;
}

QString Card::getEffectIdString() const{
    return QString::number(getEffectiveId());
}

int Card::getNumber() const{
    return number;
}

void Card::setNumber(int number){
    this->number = number;
}

QString Card::Number2String(int number){
    if(number == 10)
        return "10";
    else{
        static const char *number_string = "-A23456789-JQK";
        return QString(number_string[number]);
    }
}

QString Card::getNumberString() const{
    return Number2String(number);
}

Card::Suit Card::getSuit() const{
    return suit;
}

void Card::setSuit(Suit suit){
    this->suit = suit;
}

bool Card::sameColorWith(const Card *other) const{
    return getColor() == other->getColor();
}

Card::Color Card::getColor() const{
    switch(suit){
    case Spade:
    case Club: return Black;
    case Heart:
    case Diamond: return Red;
    default:
        return Colorless;
    }
}

bool Card::isEquipped() const{
    return Self->hasEquip(this);
}

bool Card::match(const QString &pattern) const{
    return objectName() == pattern || getType() == pattern || getSubtype() == pattern;
}

bool Card::CompareByColor(const Card *a, const Card *b){
    if(a->suit != b->suit)
        return a->suit < b->suit;
    else
        return a->number < b->number;
}

bool Card::CompareBySuitNumber(const Card *a, const Card *b){
    static Suit new_suits[] = { Spade, Heart, Club, Diamond, NoSuit};
    Suit suit1 = new_suits[a->getSuit()];
    Suit suit2 = new_suits[b->getSuit()];

    if(suit1 != suit2)
        return suit1 < suit2;
    else
        return a->number < b->number;
}

bool Card::CompareByType(const Card *a, const Card *b){
    int order1 = a->getTypeId() * 10000 + a->id;
    int order2 = b->getTypeId() * 10000 + b->id;
    if(order1 != order2)
        return order1 < order2;
    else
        return CompareBySuitNumber(a,b);
}

QString Card::getPixmapPath() const{
    QString path = QString("image/card/%1.jpg").arg(objectName());
    return QFile::exists(path) ? path : "image/card/unknown.jpg";
}

QString Card::getIconPath() const{
    return QString("image/icon/%1.png").arg(objectName());
}

QString Card::getPackage() const{
    if(parent())
        return parent()->objectName();
    else
        return "";
}

QString Card::getEffectPath(bool is_male) const{
    QString gender = is_male ? "male" : "female";
    return QString("audio/card/%1/%2.ogg").arg(gender).arg(objectName());
}

bool Card::isNDTrick() const{
    return getTypeId() == Trick && !inherits("DelayedTrick");
}

QString Card::getEffectPath() const{
    return QString("audio/card/common/%1.ogg").arg(objectName());
}

QIcon Card::getSuitIcon() const{
    return QIcon(QString("image/system/suit/%1.png").arg(getSuitString()));
}

QString Card::getFullName(bool include_suit) const{
    QString name = getName();
    if(include_suit){
        QString suit_name = Sanguosha->translate(getSuitString());
        return QString("%1%2 %3").arg(suit_name).arg(getNumberString()).arg(name);
    }else
        return QString("%1 %2").arg(getNumberString()).arg(name);
}

QString Card::getLogName() const{
    QString suit_char;
    QString number_string;
    QString tmp_string = "no_suit";
    if(getId() > -1)
        tmp_string = getSuitString();
    else if(!getSubcards().empty())
       tmp_string = Sanguosha->getCard(this->getSubcards().first())->getSuitString();
    if(suit != Card::NoSuit)
        suit_char = QString("<img src='image/system/log/%1.png' height = 12/>").arg(tmp_string);
    else
        suit_char = tr("NoSuit");

    if(number != 0)
        number_string = getNumberString();

    return QString("%1[%2%3]").arg(getName()).arg(suit_char).arg(number_string);
}

QString Card::getName() const{
    return Sanguosha->translate(objectName());
}

QString Card::getSkillName() const{
    return skill_name;
}

void Card::setSkillName(const QString &name){
    this->skill_name = name;
}

QString Card::getDescription() const{
    QString desc = Sanguosha->translate(":" + objectName());
    desc.replace("\n", "<br/>");
    return tr("<b>[%1]</b> %2").arg(getName()).arg(desc);
}

QString Card::toString() const{
    if(!isVirtualCard())
        return QString::number(id);
    else
        return QString("%1:%2[%3:%4]=%5")
                .arg(objectName()).arg(skill_name)
                .arg(getSuitString()).arg(getNumberString()).arg(subcardString());
}

QString Card::subcardString() const{
    if(subcards.isEmpty())
        return ".";

    QStringList str;
    foreach(int subcard, subcards)
        str << QString::number(subcard);

    return str.join("+");
}

void Card::addSubcards(const QList<CardItem *> &card_items){
    foreach(CardItem *card_item, card_items)
        subcards << card_item->getCard()->getId();
}

int Card::subcardsLength() const{
    return subcards.length();
}

bool Card::isVirtualCard() const{
    return id < 0;
}

const Card *Card::Parse(const QString &str){
    static QMap<QString, Card::Suit> suit_map;
    if(suit_map.isEmpty()){
        suit_map.insert("spade", Card::Spade);
        suit_map.insert("club", Card::Club);
        suit_map.insert("heart", Card::Heart);
        suit_map.insert("diamond", Card::Diamond);
        suit_map.insert("no_suit", Card::NoSuit);
    }

    if(str.startsWith(QChar('@'))){
        // skill card
        QRegExp pattern("@(\\w+)=([^:]+)(:.+)?");
        QRegExp ex_pattern("@(\\w*)\\[(\\w+):(.+)\\]=([^:]+)(:.+)?");

        QStringList texts;
        QString card_name, card_suit, card_number;
        QStringList subcard_ids;
        QString subcard_str;
        QString user_string;

        if(pattern.exactMatch(str)){
            texts = pattern.capturedTexts();
            card_name = texts.at(1);
            subcard_str = texts.at(2);
            user_string = texts.at(3);
        }
        else if(ex_pattern.exactMatch(str)){
            texts = ex_pattern.capturedTexts();
            card_name = texts.at(1);
            card_suit = texts.at(2);
            card_number = texts.at(3);
            subcard_str = texts.at(4);
            user_string = texts.at(5);
        }
        else
            return NULL;

        if(subcard_str != ".")
           subcard_ids = subcard_str.split("+");

        SkillCard *card = Sanguosha->cloneSkillCard(card_name);

        if(card == NULL)
            return NULL;

        foreach(QString subcard_id, subcard_ids)
            card->addSubcard(subcard_id.toInt());

        // skill name
        QString skill_name = card_name.remove("Card").toLower();
        card->setSkillName(skill_name);
        if(!card_suit.isEmpty())
            card->setSuit(suit_map.value(card_suit, Card::NoSuit));
        if(!card_number.isEmpty()){
            int number = 0;
            if(card_number == "A")
                number = 1;
            else if(card_number == "J")
                number = 11;
            else if(card_number == "Q")
                number = 12;
            else if(card_number == "K")
                number = 13;
            else
                number = card_number.toInt();

            card->setNumber(number);
        }

        if(!user_string.isEmpty()){
            user_string.remove(0, 1);
            card->setUserString(user_string);
        }

        return card;
    }else if(str.startsWith(QChar('$'))){
        QString copy = str;
        copy.remove(QChar('$'));
        QStringList card_strs = copy.split("+");
        DummyCard *dummy = new DummyCard;
        foreach(QString card_str, card_strs){
            dummy->addSubcard(card_str.toInt());
        }

        return dummy;
    }else if(str.startsWith(QChar('#'))){
        LuaSkillCard *new_card =  LuaSkillCard::Parse(str);
        return new_card;
    }if(str.contains(QChar('='))){
        QRegExp pattern("(\\w+):(\\w*)\\[(\\w+):(.+)\\]=(.+)");
        if(!pattern.exactMatch(str))
            return NULL;

        QStringList texts = pattern.capturedTexts();
        QString name = texts.at(1);
        QString skill_name = texts.at(2);
        QString suit_string = texts.at(3);
        QString number_string = texts.at(4);
        QString subcard_str = texts.at(5);
        QStringList subcard_ids;
        if(subcard_str != ".")
            subcard_ids = subcard_str.split("+");

        Suit suit = suit_map.value(suit_string, Card::NoSuit);

        int number = 0;
        if(number_string == "A")
            number = 1;
        else if(number_string == "J")
            number = 11;
        else if(number_string == "Q")
            number = 12;
        else if(number_string == "K")
            number = 13;
        else
            number = number_string.toInt();

        Card *card = Sanguosha->cloneCard(name, suit, number);
        if(card == NULL)
            return NULL;

        foreach(QString subcard_id, subcard_ids)
            card->addSubcard(subcard_id.toInt());

        card->setSkillName(skill_name);
        return card;
    }else{
        bool ok;
        int card_id = str.toInt(&ok);
        if(ok)
            return Sanguosha->getCard(card_id);
        else
            return NULL;
    }
}

Card *Card::Clone(const Card *card){
    const QMetaObject *meta = card->metaObject();
    Card::Suit suit = card->getSuit();
    int number = card->getNumber();

    QObject *card_obj = meta->newInstance(Q_ARG(Card::Suit, suit), Q_ARG(int, number));
    if(card_obj){
        Card *new_card = qobject_cast<Card *>(card_obj);
        new_card->setObjectName(card->objectName());
        new_card->addSubcard(card->getId());
        return new_card;
    }else
        return NULL;
}

bool Card::targetFixed() const{
    return target_fixed;
}

bool Card::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    if(target_fixed)
        return true;
    else
        return !targets.isEmpty();
}

bool Card::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

int Card::targetFilterMultiple(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targetFilter(targets,to_select,Self);
}

void Card::doPreAction(Room *, const CardUseStruct &) const{

}

void Card::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *player = card_use.from;

    LogMessage log;
    log.from = player;
    log.to = card_use.to;
    log.type = "#UseCard";
    log.card_str = toString();
    room->sendLog(log);

    QVariant data = QVariant::fromValue(card_use);
    RoomThread *thread = room->getThread();
 
    if(will_throw){
        room->throwCard(this, owner_discarded ? card_use.from : NULL);
    }
    thread->trigger(CardUsed, room, player, data);

    thread->trigger(CardFinished, room, player, data);
}

void Card::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.length() == 1){
        room->cardEffect(this, source, targets.first());
    }else{
        QList<ServerPlayer *> players = targets;
        qSort(players.begin(), players.end(), ServerPlayer::CompareByActionOrder);

        if(room->getMode() == "06_3v3"){
           if(inherits("AOE") || inherits("GlobalEffect"))
               room->reverseFor3v3(this, source, players);
        }

        foreach(ServerPlayer *target, players){
            CardEffectStruct effect;
            effect.card = this;
            effect.from = source;
            effect.to = target;
            effect.multiple = true;

            room->cardEffect(effect);
        }
    }
    room->removeTag("Huoshou");
}

void Card::onEffect(const CardEffectStruct &) const{

}

bool Card::isCancelable(const CardEffectStruct &) const{
    return false;
}

void Card::onMove(const CardMoveStruct &) const{
    // usually dummy
}

void Card::addSubcard(int card_id){
    if(card_id < 0)
        qWarning("%s", qPrintable(tr("Subcard must not be virtual card!")));
    else
        subcards << card_id;
}

void Card::addSubcard(const Card *card){
    addSubcard(card->getEffectiveId());
}

QList<int> Card::getSubcards() const{
    return subcards;
}

void Card::clearSubcards(){
    subcards.clear();
}

bool Card::isAvailable(const Player *player) const{
    return !player->isJilei(this) && !player->isLocked(this);
}

const Card *Card::validate(const CardUseStruct *) const{
    return this;
}

const Card *Card::validateInResposing(ServerPlayer *, bool *continuable) const{
    *continuable = false;
    return this;
}

bool Card::isOnce() const{
    return once;
}

bool Card::isMute() const{
    return mute;
}


bool Card::willThrow() const{
    return will_throw;
}

bool Card::canJilei() const{
    return can_jilei;
}

bool Card::isOwnerDiscarded() const{
    return owner_discarded;
}

bool Card::hasPreAction() const{
    return has_preact;
}

void Card::setFlags(const QString &flag) const{
    static char symbol_c = '-';

    if(flag.isEmpty())
        return;
    else if(flag == ".")
        flags.clear();
    else if(flag.startsWith(symbol_c)){
        QString copy = flag;
        copy.remove(symbol_c);
        flags.removeOne(copy);
    }
    else
        flags << flag;
}

bool Card::hasFlag(const QString &flag) const{
    return flags.contains(flag);
}

void Card::clearFlags() const{
    flags.clear();
}

// ---------   Skill card     ------------------

SkillCard::SkillCard()
    :Card(NoSuit, 0)
{
}

void SkillCard::setUserString(const QString &user_string){
    this->user_string = user_string;
}

QString SkillCard::getType() const{
    return "skill_card";
}

QString SkillCard::getSubtype() const{
    return "skill_card";
}

Card::CardType SkillCard::getTypeId() const{
    return Card::Skill;
}

QString SkillCard::toString() const{
    QString str = QString("@%1[%2:%3]=%4")
            .arg(metaObject()->className()).arg(getSuitString())
            .arg(getNumberString()).arg(subcardString());

    if(!user_string.isEmpty())
        return QString("%1:%2").arg(str).arg(user_string);
    else
        return str;
}

// ---------- Dummy card      -------------------

DummyCard::DummyCard():SkillCard()
{
    target_fixed = true;
    setObjectName("dummy");
}

QString DummyCard::getType() const{
    return "dummy_card";
}

QString DummyCard::getSubtype() const{
    return "dummy_card";
}

QString DummyCard::toString() const{
    return "$" + subcardString();
}
