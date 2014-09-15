/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#include "cardbutton.h"
#include "card.h"
#include "SkinBank.h"

CardButton::CardButton(const Card *card)
    :card(card), scale(1.0)
{
    if (card) {
        setText(card->getFullName());
        setIcon(G_ROOM_SKIN.getCardSuitPixmap(card->getSuit()));
        setToolTip(card->getDescription());
    }

    connect(this, SIGNAL(clicked()), this, SLOT(onClicked()));
}

QSize CardButton::sizeHint() const {
    QSize size = QCommandLinkButton::sizeHint();
    return QSize(size.width() * scale, size.height());
}

void CardButton::onClicked() {
    if (card)
        emit idSelected(card->getId());
    else
        emit idSelected(-1);
}
