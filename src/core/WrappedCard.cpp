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

#include "WrappedCard.h"

WrappedCard::WrappedCard(Card *card)
    : Card(card->getSuit(), card->getNumber()), m_card(NULL), m_isModified(false)
{
    m_id = card->getId();
    copyEverythingFrom(card);
}

WrappedCard::~WrappedCard() {
    Q_ASSERT(m_card != NULL);
    delete m_card;
}

void WrappedCard::takeOver(Card *card) {
    Q_ASSERT(getId() >= 0);
    Q_ASSERT(card != this);
    Q_ASSERT(m_card != card);
    if (m_card != NULL) {
        m_isModified = true;
        delete m_card;
    }
    m_card = card;
    m_card->setId(getId());
    setObjectName(card->objectName());
    setSuit(card->getSuit());
    setNumber(card->getNumber());
    m_skillName = card->getSkillName(false);
}

void WrappedCard::copyEverythingFrom(Card *card) {
    Q_ASSERT(card->getId() >= 0);
    Q_ASSERT(card != this);
    Q_ASSERT(m_card != card);
    if (m_card != NULL) {
        m_isModified = true;
        m_card->deleteLater();
    }
    setObjectName(card->objectName());
    m_card = card;
    Card::setId(card->getEffectiveId());
    Card::setSuit(card->getSuit());
    Card::setNumber(card->getNumber());
    flags = card->getFlags();
    m_skillName = card->getSkillName(false);
}

void WrappedCard::setFlags(const QString &flag) const{
    Q_ASSERT(m_card != NULL);
    Card::setFlags(flag);
    m_card->setFlags(flag);
    m_isModified = true;
}

