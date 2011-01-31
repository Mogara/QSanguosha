#include "joypackage.h"
#include "engine.h"

Shit::Shit(Suit suit, int number):BasicCard(suit, number){
    setObjectName("shit");

    target_fixed = true;
}

QString Shit::getSubtype() const{
    return "disgusting_card";
}

void Shit::onMove(const CardMoveStruct &move) const{
    ServerPlayer *from = move.from;
    if(from && move.from_place == Player::Hand &&
       from->getRoom()->getCurrent() == move.from
       && (move.to_place == Player::DiscardedPile || move.to_place == Player::Special)
       && move.to == NULL
       && from->isAlive()){

        DamageStruct damage;
        damage.from = damage.to = from;
        damage.card = this;

        switch(getSuit()){
        case Club: damage.nature = DamageStruct::Thunder; break;
        case Heart: damage.nature = DamageStruct::Fire; break;
        default:
            damage.nature = DamageStruct::Normal;
        }

        from->getRoom()->damage(damage);
    }
}

bool Shit::HasShit(const Card *card){
    if(card->isVirtualCard()){
        QList<int> card_ids = card->getSubcards();
        foreach(int card_id, card_ids){
            const Card *c = Sanguosha->getCard(card_id);
            if(c->objectName() == "shit")
                return true;
        }

        return false;
    }else
        return card->objectName() == "shit";
}

// -----------  Deluge -----------------

static QString DelugeCallback(const Card *card, Room *){
    int number = card->getNumber();
    if(number == 1 || number == 13)
        return "bad";
    else
        return "good";
}

Deluge::Deluge(Card::Suit suit, int number)
    :DelayedTrick(suit, number, true)
{
    callback = DelugeCallback;
    setObjectName("deluge");
    target_fixed = true;
}

void Deluge::takeEffect(ServerPlayer *target) const{
    QList<const Card *> cards = target->getCards("he");

    Room *room = target->getRoom();
    int n = qMin(cards.length(), target->aliveCount());
    if(n == 0)
        return;

    qShuffle(cards);
    cards = cards.mid(0, n);

    QList<int> card_ids;
    foreach(const Card *card, cards){
        card_ids << card->getEffectiveId();
        room->throwCard(card);
    }

    room->fillAG(card_ids);

    QList<ServerPlayer *> players = room->getOtherPlayers(target);
    players << target;
    players = players.mid(0, n);
    foreach(ServerPlayer *player, players){
        if(player->isAlive()){
            int card_id = room->askForAG(player, card_ids);
            card_ids.removeOne(card_id);

            room->takeAG(player, card_id);
        }
    }

    foreach(int card_id, card_ids)
        room->takeAG(NULL, card_id);

    room->broadcastInvoke("clearAG");
}

// -----------  Typhoon -----------------

static QString TyphoonCallback(const Card *card, Room *)
{
    int number = card->getNumber();
    if(card->getSuit() == Card::Diamond && number >= 2 && number <= 9)
        return "bad";
    else
        return "good";
}

Typhoon::Typhoon(Card::Suit suit, int number)
    :DelayedTrick(suit, number, true)
{
    callback = TyphoonCallback;
    setObjectName("typhoon");
    target_fixed = true;
}

void Typhoon::takeEffect(ServerPlayer *target) const{
    Room *room = target->getRoom();

    QList<ServerPlayer *> players = room->getOtherPlayers(target);
    foreach(ServerPlayer *player, players){
        if(target->distanceTo(player) == 1){
            int discard_num = qMin(6, player->getHandcardNum());
            if(discard_num == 0)
                room->setEmotion(player, Room::Good);
            else{
                room->setEmotion(player, Room::Bad);
                room->broadcastInvoke("animate", "typhoon:" + player->objectName());
                room->broadcastInvoke("playAudio", "typhoon");

                room->askForDiscard(player, objectName(), discard_num);
            }

            room->getThread()->delay();
        }
    }
}

// -----------  Earthquake -----------------

static QString EarthquakeCallback(const Card *card, Room *)
{
    int number = card->getNumber();
    if(card->getSuit() == Card::Club && number >= 2 && number <= 9)
        return "bad";
    else
        return "good";
}

Earthquake::Earthquake(Card::Suit suit, int number)
    :DelayedTrick(suit, number, true)
{
    callback = EarthquakeCallback;
    setObjectName("earthquake");
    target_fixed = true;
}

void Earthquake::takeEffect(ServerPlayer *target) const{
    Room *room = target->getRoom();
    QList<ServerPlayer *> players = room->getAllPlayers();
    foreach(ServerPlayer *player, players){
        if(target->distanceTo(player) <= 1){
            if(player->getEquips().isEmpty()){
                room->setEmotion(player, Room::Good);
            }else{
                room->setEmotion(player, Room::Bad);
                room->broadcastInvoke("playAudio", "earthquake");
                player->throwAllEquips();
            }

            room->getThread()->delay();
        }
    }
}

// -----------  Volcano -----------------

static QString VolcanoCallback(const Card *card, Room *room)
{
    int number = card->getNumber();
    if(card->getSuit() == Card::Heart && number >= 2 && number <= 9){
        QString victim = number <= 5 ? "Previous" : "Next";
        room->setTag("VolcanoVictim", victim);
        return "bad";
    }else
        return "good";
}

Volcano::Volcano(Card::Suit suit, int number)
    :DelayedTrick(suit, number, true)
{
    callback = VolcanoCallback;
    setObjectName("volcano");
    target_fixed = true;
}

void Volcano::takeEffect(ServerPlayer *target) const{
    Room *room = target->getRoom();

    QList<ServerPlayer *> players = room->getAlivePlayers();
    int index = players.indexOf(target);

    QString victim = room->getTag("VolcanoVictim").toString();
    if(victim == "Previous"){
        index --;
        if(index < 0)
            index += players.length();
    }else{
        index ++;
        if(index >= players.length())
            index -= players.length();
    }

    DamageStruct damage;
    damage.card = this;
    damage.damage = 3;
    damage.nature = DamageStruct::Fire;
    damage.to = players.at(index);

    room->damage(damage);
}

JoyPackage::JoyPackage()
    :Package("joy")
{
    QList<Card *> cards;

    cards << new Shit(Card::Club, 1)
            << new Shit(Card::Heart, 8)
            << new Shit(Card::Diamond, 13);

    cards << new Deluge(Card::Spade, 1)
            << new Typhoon(Card::Spade, 4)
            << new Earthquake(Card::Club, 10)
            << new Volcano(Card::Heart, 13);

    foreach(Card *card, cards)
        card->setParent(this);
}

ADD_PACKAGE(Joy);
