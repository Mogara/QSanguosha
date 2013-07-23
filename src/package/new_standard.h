#ifndef NEWSTANDARDPACKAGE_H
#define NEWSTANDARDPACKAGE_H

#include "package.h"
#include "card.h"

class NewLeijiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE NewLeijiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class NewRendeCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE NewRendeCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>
class NewGuhuoCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE NewGuhuoCard();
    bool newguhuo(ServerPlayer *yuji) const;

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(CardUseStruct &card_use) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
};

class NewGuhuoDialog: public QDialog {
    Q_OBJECT

public:
    static NewGuhuoDialog *getInstance(const QString &object, bool left = true, bool right = true);

public slots:
    void popup();
    void selectCard(QAbstractButton *button);

private:
    explicit NewGuhuoDialog(const QString &object, bool left = true, bool right = true);

    QGroupBox *createLeft();
    QGroupBox *createRight();
    QAbstractButton *createButton(const Card *card);
    QButtonGroup *group;
    QHash<QString, const Card *> map;

    QString object_name;

signals:
    void onButtonClick();
};

class NewStandardPackage : public Package{
    Q_OBJECT

public:
    NewStandardPackage();
};

#endif // NEWSTANDARDPACKAGE_H
