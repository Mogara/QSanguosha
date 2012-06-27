#ifndef PLAYERCARDDIALOG_H
#define PLAYERCARDDIALOG_H

#include "clientplayer.h"

#include <QDialog>
#include <QMap>

class MagatamaWidget : public QWidget{
    Q_OBJECT

public:
    explicit MagatamaWidget(int hp, Qt::Orientation orientation);

    static QPixmap *GetMagatama(int index);
    static QPixmap *GetSmallMagatama(int index);
};

class PlayerCardDialog : public QDialog{
    Q_OBJECT

public:
    explicit PlayerCardDialog(const ClientPlayer *player, const QString &flags = "hej");

private:
    QWidget *createAvatar();
    QWidget *createHandcardButton();
    QWidget *createEquipArea();
    QWidget *createJudgingArea();

    const ClientPlayer *player;
    QMap<QObject *, int> mapper;

private slots:
    void emitId();

signals:
    void card_id_chosen(int card_id);
};

#endif // PLAYERCARDDIALOG_H
