#ifndef NOSTALGIA_H
#define NOSTALGIA_H

#include "package.h"
#include "card.h"

#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>

class NostalgiaPackage: public Package{
    Q_OBJECT

public:
    NostalgiaPackage();
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
    bool guhuo(ServerPlayer *yuji) const;

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual bool targetsFeasible(const QList<const ClientPlayer *> &targets) const;

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

#endif // NOSTALGIA_H
