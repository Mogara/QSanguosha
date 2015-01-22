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

#include "exppattern.h"
#include "engine.h"

ExpPattern::ExpPattern(const QString &exp) {
    this->exp = exp;
}

bool ExpPattern::match(const Player *player, const Card *card) const{
    foreach (const QString &one_exp, this->exp.split('#'))
        if (this->matchOne(player, card, one_exp)) return true;

    return false;
}

// '|' means 'and', '#' means 'or'.
// the expression splited by '#' has 3 parts,
// 1st part means the card name, and ',' means more than one options.
// 2nd patt means the card suit, and ',' means more than one options.
// 3rd part means the card number, and ',' means more than one options,
// the number uses '~' to make a scale for valid expressions
bool ExpPattern::matchOne(const Player *player, const Card *card, QString exp) const{
    QStringList factors = exp.split('|');

    bool checkpoint = false;
    QStringList card_types = factors.at(0).split(',');
    foreach (const QString &or_name, card_types) {
        checkpoint = false;
        foreach (const QString &_name, or_name.split('+')) {
            QString name = _name;
            if (name == ".") {
                checkpoint = true;
            } else {
                bool isInt = false;
                bool positive = true;
                if (name.startsWith('^')) {
                    positive = false;
                    name = name.mid(1);
                }
                if (card->isKindOf(name.toLocal8Bit().data())
                    || ("%" + card->objectName() == name)
                    || (card->getEffectiveId() == name.toInt(&isInt) && isInt))
                    checkpoint = positive;
                else
                    checkpoint = !positive;
            }
            if (!checkpoint) break;
        }
        if (checkpoint) break;
    }
    if (!checkpoint) return false;
    if (factors.size() < 2) return true;

    checkpoint = false;
    QStringList card_suits = factors.at(1).split(',');
    foreach (const QString &_suit, card_suits) {
        QString suit = _suit;
        if (suit == ".") { checkpoint = true; break; }
        bool positive = true;
        if (suit.startsWith('^')) {
            positive = false;
            suit = suit.mid(1);
        }
        if (card->getSuitString() == suit
            || (card->isBlack() && suit == "black")
            || (card->isRed() && suit == "red"))
            checkpoint = positive;
        else
            checkpoint = !positive;
        if (checkpoint) break;
    }
    if (!checkpoint) return false;
    if (factors.size() < 3) return true;

    checkpoint = false;
    QStringList card_numbers = factors.at(2).split(',');
    int cdn = card->getNumber();

    foreach (const QString &number, card_numbers) {
        if (number == ".") { checkpoint = true; break; }
        bool isInt = false;
        if (number.contains('~')) {
            QStringList params = number.split('~');
            int from, to;
            if (!params.at(0).size())
                from = 1;
            else
                from = params.at(0).toInt();
            if (!params.at(1).size())
                to = 13;
            else
                to = params.at(1).toInt();

            if (from <= cdn && cdn <= to) checkpoint = true;
        }
        else if (number.toInt(&isInt) == cdn && isInt) {
            checkpoint = true;
        }
        else if ((number == "A" && cdn == 1)
            || (number == "J" && cdn == 11)
            || (number == "Q" && cdn == 12)
            || (number == "K" && cdn == 13)) {
            checkpoint = true;
        }
        if (checkpoint) break;
    }
    if (!checkpoint) return false;
    if (factors.size() < 4) return true;

    checkpoint = false;
    QString place = factors.at(3);
    if (!player || place == ".") checkpoint = true;
    if (!checkpoint) {
        QList<int> ids;
        if (card->isVirtualCard())
            ids = card->getSubcards();
        else
            ids << card->getEffectiveId();
        if (!ids.isEmpty()) {
            foreach (int id, ids) {
                checkpoint = false;
                const Card *card = Sanguosha->getCard(id);
                foreach (const QString &_p, place.split(",")) {
                    QString p = _p;
                    if (p == "equipped" && player->hasEquip(card)) {
                        checkpoint = true;
                    } else if (p == "hand" && card->getEffectiveId() >= 0) {
                        foreach (const Card *c, player->getHandcards()) {
                            if (c->getEffectiveId() == id) {
                                checkpoint = true;
                                break;
                            }
                        }
                    } else if (p.startsWith("%")) {
                        p = p.mid(1);
                        foreach (const Player *pl, player->getAliveSiblings())
                            if (!pl->getPile(p).isEmpty() && pl->getPile(p).contains(id)) {
                                checkpoint = true;
                                break;
                            }
                    } else if (!player->getPile(p).isEmpty() && player->getPile(p).contains(id)) {
                        checkpoint = true;
                    }
                    if (checkpoint)
                        break;
                }
                if (!checkpoint)
                    break;
            }
        }
    }
    return checkpoint;
}

