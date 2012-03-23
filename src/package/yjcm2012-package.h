#ifndef YJCM2012PACKAGE_H
#define YJCM2012PACKAGE_H

#include "package.h"
#include "card.h"
#include <QMutex>
#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCommandLinkButton>
#include <QLayout>
#include <QDialog>

class YJCM2012Package: public Package{
    Q_OBJECT

public:
    YJCM2012Package();
};

class QiceCard: public SkillCard{
    Q_OBJECT


public:
    Q_INVOKABLE QiceCard();

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(const CardUseStruct *card_use) const;
};

class QiceDialog: public QDialog{
    Q_OBJECT

public:
    static QiceDialog *GetInstance();

public slots:
    void popup();
    void selectCard(QAbstractButton *button);

private:
    QiceDialog();

    QGroupBox *createRight();
    QAbstractButton *createButton(const Card *card);
    QButtonGroup *group;
    QHash<QString, const Card *> map;
};

class ZhenlieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhenlieCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class AnxuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE AnxuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ChunlaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChunlaoCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};
#endif // YJCM2012PACKAGE_H
