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
        damage.damage = 1;
        damage.nature = DamageStruct::Normal;

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


JoyPackage::JoyPackage()
    :Package("joy")
{
    QList<Card *> cards;

    cards << new Shit(Card::Club, 1)
            << new Shit(Card::Heart, 8)
            << new Shit(Card::Diamond, 13);

    foreach(Card *card, cards)
        card->setParent(this);
}

ADD_PACKAGE(Joy);
