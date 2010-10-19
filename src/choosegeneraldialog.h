#ifndef CHOOSEGENERALDIALOG_H
#define CHOOSEGENERALDIALOG_H

class General;

#include <QDialog>
#include <QProgressBar>
#include <QGroupBox>
#include <QButtonGroup>

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

private slots:
    void freeChoose();
};

class FreeChooseDialog: public QDialog{
    Q_OBJECT

public:
    explicit FreeChooseDialog(ChooseGeneralDialog *parent);

private:
    QButtonGroup *group;

    QGroupBox *createGroupBox(const QList<const General *> &generals);

private slots:
    void chooseGeneral();
};

#endif // CHOOSEGENERALDIALOG_H
