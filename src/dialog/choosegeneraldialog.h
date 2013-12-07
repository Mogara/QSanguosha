#ifndef _CHOOSE_GENERAL_DIALOG_H
#define _CHOOSE_GENERAL_DIALOG_H

class General;

#include "TimedProgressBar.h"

#include <QDialog>
#include <QGroupBox>
#include <QButtonGroup>

#include <QToolButton>

class OptionButton: public QToolButton {
    Q_OBJECT

public:
    explicit OptionButton(const QString icon_path, const QString &caption = "", QWidget *parent = 0);
#ifdef Q_WS_X11
    virtual QSize sizeHint() const{ return iconSize(); } // it causes bugs under Windows
#endif

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *);

signals:
    void double_clicked();
};

class ChooseGeneralDialog: public QDialog {
    Q_OBJECT

public:
    explicit ChooseGeneralDialog(const QStringList &general_names, QWidget *parent, bool view_only = false, const QString &title = QString());

public slots:
    void done(int);

protected:
    QDialog *m_freeChooseDialog;

private:
    QSanCommandProgressBar *progress_bar;    

private slots:
    void freeChoose();   
};

class FreeChooseDialog: public QDialog {
    Q_OBJECT
    Q_ENUMS(ButtonGroupType)

public:
    enum ButtonGroupType { Exclusive, Pair, Multi };

    explicit FreeChooseDialog(QWidget *parent, ButtonGroupType type = Exclusive);

private:
    QButtonGroup *group;
    ButtonGroupType type;
    QWidget *createTab(const QList<const General *> &generals);

private slots:
    void chooseGeneral();
    void uncheckExtraButton(QAbstractButton *button);

signals:
    void general_chosen(const QString &name);
    void pair_chosen(const QString &first, const QString &second);
};

#endif

