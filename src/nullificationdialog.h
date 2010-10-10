#ifndef NULLIFICATIONDIALOG_H
#define NULLIFICATIONDIALOG_H

#include "clientplayer.h"

#include <QDialog>
#include <QButtonGroup>
#include <QProgressBar>

class NullificationDialog : public QDialog{
    Q_OBJECT
public:
    explicit NullificationDialog(const QString &trick_name, ClientPlayer *source, ClientPlayer *target, const QList<int> &card_ids);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    QProgressBar *progress_bar;
    QButtonGroup *button_group;

private slots:
    void reply();
    void onReject();
};

#endif // NULLIFICATIONDIALOG_H
