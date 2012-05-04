#ifndef CHOOSEGENERALDIALOG_H
#define CHOOSEGENERALDIALOG_H

class General;

#include "TimedProgressBar.h"

#include <QDialog>
#include <QGroupBox>
#include <QButtonGroup>

#include <QToolButton>

class OptionButton : public QToolButton
{
    Q_OBJECT
public:
    explicit OptionButton(const QString icon_path, const QString &caption = "", QWidget *parent = 0);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *);

signals:
    void double_clicked();

};

class ChooseGeneralDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseGeneralDialog(const QStringList &general_names,
                                 QWidget *parent);
public slots:
    void done(int);

protected:
    QDialog *m_freeChooseDialog;

private:
    QSanCommandProgressBar *progress_bar;    

private slots:
    void freeChoose();   
};

class FreeChooseDialog: public QDialog{
    Q_OBJECT

public:
    explicit FreeChooseDialog(QWidget *parent, bool pair_choose = false);

private:
    QButtonGroup *group;
    bool pair_choose;
    QWidget *createTab(const QList<const General *> &generals);

private slots:
    void chooseGeneral();
    void uncheckExtraButton(QAbstractButton *button);

signals:
    void general_chosen(const QString &name);
    void pair_chosen(const QString &first, const QString &second);
};

#endif // CHOOSEGENERALDIALOG_H
