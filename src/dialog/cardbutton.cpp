#include "cardbutton.h"

#include "card.h"

CardButton::CardButton(const Card *card)
    :card(card)
{
    if(card){
        setText(card->getFullName());
        setIcon(card->getSuitIcon());
        setToolTip(card->getDescription());
    }

    connect(this, SIGNAL(clicked()), this, SLOT(onClicked()));
}

void CardButton::onClicked()
{
    if(card)
        emit idSelected(card->getId());
    else
        emit idSelected(-1);
}
