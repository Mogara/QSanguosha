#ifndef DAMAGEMAKERDIALOG_H
#define DAMAGEMAKERDIALOG_H

#include <QDialog>
#include <QAbstractListModel>

class DamageMakerOperationsModel: public QAbstractListModel{
    Q_OBJECT

public:
    enum Operation{
        Normal,
        Thunder,
        Fire,
        Recover,
        LoseHp,
        LostMaxHp,
        ResetMaxHp,

        OperationCount
    };

    DamageMakerOperationsModel(QObject *parent);
    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
};

class DamageMakerDialog: public QDialog{
    Q_OBJECT

public:
    DamageMakerDialog(QWidget *parent);

public slots:
    virtual void accept();

private slots:
    void disableSource();
};

#endif // DAMAGEMAKERDIALOG_H
