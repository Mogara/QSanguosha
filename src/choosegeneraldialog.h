#ifndef CHOOSEGENERALDIALOG_H
#define CHOOSEGENERALDIALOG_H

class General;

#include <QDialog>
#include <QProgressBar>

class ChooseGeneralDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseGeneralDialog(const QList<const General *> &generals,
                                 QWidget *parent);

    void start();

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    QProgressBar *progress_bar;
};

#endif // CHOOSEGENERALDIALOG_H
