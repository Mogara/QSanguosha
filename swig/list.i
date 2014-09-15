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

template <class T>
class QList {
public:
    QList();
    ~QList();
    int length() const;
    void append(const T &elem);
    void prepend(const T &elem);
    bool isEmpty() const;
    bool contains(const T &value) const;
    T first() const;
    T last() const;
    void replace(int i, const T &t);
    void removeAt(int i);
    int removeAll(const T &value);
    bool removeOne(const T &value);
    QList<T> mid(int pos, int length = -1) const;
    int indexOf(const T &value, int from = 0);
};

%extend QList {
    T at(int i) const{
        return $self->value(i);
    }
}

%template(SPlayerList) QList<ServerPlayer *>;
%template(PlayerList)  QList<const Player *>;
%template(CardList) QList<const Card *>;
%template(IntList) QList<int>;
%template(SkillList) QList<const Skill *>;
%template(DelayedTrickList) QList<const DelayedTrick *>;
%template(CardsMoveList) QList<CardsMoveStruct>;
%template(PlaceList) QList<Player::Place>;
%template(PhaseList) QList<Player::Phase>;
%template(VariantList) QList<QVariant>;

typedef QList<QVariant> QVariantList;


