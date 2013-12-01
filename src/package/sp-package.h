#ifndef _SP_H
#define _SP_H

#include "package.h"
#include "card.h"
#include "standard.h"

#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>
#include <QVBoxLayout>

class SPPackage: public Package {
    Q_OBJECT

public:
    SPPackage();
};

class OLPackage: public Package {
    Q_OBJECT

public:
    OLPackage();
};

class TaiwanSPPackage: public Package {
    Q_OBJECT

public:
    TaiwanSPPackage();
};

class WangZheZhiZhanPackage: public Package {
    Q_OBJECT

public:
    WangZheZhiZhanPackage();
};

class Yongsi: public TriggerSkill {
    Q_OBJECT

public:
    Yongsi();
    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *yuanshu, QVariant &data) const;

protected:
    virtual int getKingdoms(ServerPlayer *yuanshu) const;
};

class WeidiDialog: public QDialog {
    Q_OBJECT

public:
    static WeidiDialog *getInstance();

public slots:
    void popup();
    void selectSkill(QAbstractButton *button);

private:
    explicit WeidiDialog();

    QAbstractButton *createSkillButton(const QString &skill_name);
    QButtonGroup *group;
    QVBoxLayout *button_layout;

signals:
    void onButtonClick();
};

class YuanhuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE YuanhuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XuejiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE XuejiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class BifaCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE BifaCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class SongciCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE SongciCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class AocaiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE AocaiCard();

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validateInResponse(ServerPlayer *user) const;
    virtual const Card *validate(CardUseStruct &cardUse) const;
};

class DuwuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE DuwuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QingyiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE QingyiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ZhoufuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ZhoufuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class SPCardPackage: public Package {
    Q_OBJECT

public:
    SPCardPackage();
};

class SPMoonSpear: public Weapon {
    Q_OBJECT

public:
    Q_INVOKABLE SPMoonSpear(Card::Suit suit = Diamond, int number = 12);
};

class HegemonySPPackage: public Package {
    Q_OBJECT

public:
    HegemonySPPackage();
};

#endif

