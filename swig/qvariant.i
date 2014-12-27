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

class QVariant {
public:
    QVariant();
    QVariant(int);
    QVariant(const char *);
    QVariant(bool);
    QVariant(QList<QVariant>);
    int toInt() const;
    QString toString() const;
    QStringList toStringList() const;
    bool toBool() const;
    QList<QVariant> toList() const;
};

%extend QVariant {
    void setValue(int value) {
        $self->setValue(QVariant::fromValue(value));
    }

    void setValue(DamageStruct *damage) {
        $self->setValue(QVariant::fromValue(*damage));
    }

    DamageStruct toDamage() const{
        return $self->value<DamageStruct>();
    }

    void setValue(CardEffectStruct *effect) {
        $self->setValue(QVariant::fromValue(*effect));
    }

    CardEffectStruct toCardEffect() const{
        return $self->value<CardEffectStruct>();
    }

    void setValue(SlashEffectStruct *effect) {
        $self->setValue(QVariant::fromValue(*effect));
    }

    SlashEffectStruct toSlashEffect() const{
        return $self->value<SlashEffectStruct>();
    }

    void setValue(CardUseStruct *use) {
        $self->setValue(QVariant::fromValue(*use));
    }

    CardUseStruct toCardUse() const{
        return $self->value<CardUseStruct>();
    }

    void setValue(const Card *card) {
        $self->setValue(QVariant::fromValue(card));
    }

    const Card *toCard() const{
        return $self->value<const Card *>();
    }

    void setValue(ServerPlayer *player) {
        $self->setValue(QVariant::fromValue(player));
    }

    ServerPlayer *toPlayer() const{
        return $self->value<ServerPlayer *>();
    }

    void setValue(DyingStruct *dying) {
        $self->setValue(QVariant::fromValue(*dying));
    }

    DyingStruct toDying() const{
        return $self->value<DyingStruct>();
    }

    void setValue(DeathStruct *death) {
        $self->setValue(QVariant::fromValue(*death));
    }

    DeathStruct toDeath() const{
        return $self->value<DeathStruct>();
    }

    void setValue(RecoverStruct *recover) {
        $self->setValue(QVariant::fromValue(*recover));
    }

    RecoverStruct toRecover() const{
        return $self->value<RecoverStruct>();
    }

    void setValue(JudgeStruct *judge) {
        $self->setValue(QVariant::fromValue(judge));
    }

    JudgeStruct *toJudge() const{
        return $self->value<JudgeStruct *>();
    }

    void setValue(PindianStruct *pindian) {
        $self->setValue(QVariant::fromValue(pindian));
    }

    PindianStruct *toPindian() const{
        return $self->value<PindianStruct *>();
    }

    void setValue(PhaseChangeStruct *phase) {
        $self->setValue(QVariant::fromValue(*phase));
    }

    PhaseChangeStruct toPhaseChange() const{
        return $self->value<PhaseChangeStruct>();
    }

    void setValue(CardsMoveOneTimeStruct *move) {
        $self->setValue(QVariant::fromValue(*move));
    }

    CardsMoveOneTimeStruct toMoveOneTime() const{
        return $self->value<CardsMoveOneTimeStruct>();
    }

    void setValue(CardResponseStruct *resp) {
        $self->setValue(QVariant::fromValue(*resp));
    }

    CardResponseStruct toCardResponse() const{
        return $self->value<CardResponseStruct>();
    }

    void setValue(PlayerNumStruct *cmcs) {
        $self->setValue(QVariant::fromValue(*cmcs));
    }

    PlayerNumStruct toPlayerNum() const{
        return $self->value<PlayerNumStruct>();
    }
/*
    void setValue(QList<int> intlist) {
        QVariantList varlist;
        for (int i = 0; i < intlist.length(); ++i)
            varlist.append(QVariant::fromValue(intlist.at(i)));
        $self->setValue(QVariant::fromValue(varlist));
    }

    QList<int> toIntList() const{
        QList<int> result;
        if ($self->canConvert<QVariantList>()) {
            QVariantList res_var = $self->toList();
            for (int i = 0; i < res_var.length(); ++i)
                result.append(res_var.at(i).toInt());
        }
        return result;
    }
*/
};
