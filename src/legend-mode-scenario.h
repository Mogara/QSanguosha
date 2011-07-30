#ifndef LEGEND_MODE_H
#define LEGEND_MODE_H

#include "scenario.h"

#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>

class LegendScenario : public Scenario{
    Q_OBJECT

public:
    explicit LegendScenario();

    virtual void onTagSet(Room *room, const QString &key) const;
    virtual bool generalSelection() const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
     virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual bool exposeRoles() const;
};

class ChuanqiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChuanqiCard();

    void loadChuanqiConfig();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    static QMap<QString,int>card_map;
    static QMap<QString,int>thresh_map;
};

class ArcChuanqiCard: public ChuanqiCard{
    Q_OBJECT

public:
    Q_INVOKABLE ArcChuanqiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    static QMap<QString,int>card_map;
    static QMap<QString,int>thresh_map;
};

class ChuanqiDialog: public QDialog{
    Q_OBJECT

public:
    static ChuanqiDialog *GetInstance();

public slots:
    void popup();
    void selectCard(QAbstractButton *button);

private:
    ChuanqiDialog();

    QGroupBox *createSelection(int code,int thresh,int id);
    QAbstractButton *createButton(int code,int id);
    QButtonGroup *group;
};

#endif // LEGEND_MODE_H
