#ifndef WIND_H
#define WIND_H

#include "package.h"
#include "card.h"

#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>


class GuidaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuidaoCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LeijiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LeijiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HuangtianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuangtianCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class ShensuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShensuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class TianxiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TianxiangCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class GuhuoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuhuoCard();
    bool guhuo(ServerPlayer* yuji, const QString& message) const;

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(const CardUseStruct *card_use) const;
    virtual const Card *validateInResposing(ServerPlayer *user, bool *continuable) const;
};

class GuhuoDialog: public QDialog{
    Q_OBJECT

public:
    static GuhuoDialog *GetInstance();

public slots:
    void popup();
    void selectCard(QAbstractButton *button);

private:
    GuhuoDialog();

    QGroupBox *createLeft();
    QGroupBox *createRight();
    QAbstractButton *createButton(const Card *card);
    QButtonGroup *group;
    QHash<QString, const Card *> map;
};

class WindPackage: public Package{
    Q_OBJECT

public:
    WindPackage();
};

#endif // WIND_H
