#ifndef GENERALOVERVIEW_H
#define GENERALOVERVIEW_H

#include "generalmodel.h"

#include <QWidget>
#include <QDialog>
#include <QModelIndex>

class General;
class QAbstractButton;
class QHBoxLayout;
class QGridLayout;
class QLabel;
class QListView;
class QLineEdit;
class QTextEdit;
class QLayout;

class PackageSelector: public QDialog{
    Q_OBJECT

public:
    PackageSelector(QWidget *parent);

private slots:
    void onButtonClicked(QAbstractButton *button);

signals:
    void packageSelected(const QString &packageName);
};

class GeneralOverview : public QWidget {
    Q_OBJECT

public:
    GeneralOverview(); // you should never use its constructor, as it should be singleton

    static void display(const QString &name = QString());

private:
    void showGeneral(const QString &name);
    QHBoxLayout *addButtonsFromStringList(const QStringList &list, const char *configName);
    QLayout *createLeft();
    QLayout *createMiddle();
    QLayout *createRight();

    QMap<QString, QString> options;
    QLabel *generalLabel;
    QListView *generalView;
    QLabel *generalImage;
    QLabel *generalInfo;
    QTextEdit *generalSkill;
    QLabel *effectLabel;

private slots:
    void doSearch();
    void selectPackage();
    void onPackageSelected(const QString &packageName);
    void onSearchBoxDone();
    void onGeneralViewClicked(const QModelIndex &index);
    void onRadioButtonClicked(QAbstractButton *button);
    void onEffectLabelClicked(const QString &link);
};

#endif // GENERALOVERVIEW_H
