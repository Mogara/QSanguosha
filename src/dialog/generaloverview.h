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
class QLayout;
class QTextBrowser;

class GeneralOverview : public QWidget {
    Q_OBJECT

public:
    GeneralOverview(); // you should never use its constructor, as it should be singleton

    static void display(const QString &name = QString());

private:
    QHBoxLayout *addButtonsFromStringList(const QStringList &list, const char *configName);
    QLayout *createLeft();
    QLayout *createMiddle();
    QLayout *createRight();

    QMap<QString, QString> options;
    QLabel *generalLabel;
    QListView *generalView;
    QLabel *generalImage;
    QLabel *generalInfo;
    QTextBrowser *generalSkill;
    QTextBrowser *effectBrowser;

private slots:
    void showGeneral(const QString &name);

    void doSearch();
    void onPackageActionTriggered(QAction *action);
    void onMaxHpIndexChanged(int index);
    void onGeneralViewClicked(const QModelIndex &index);
    void onRadioButtonClicked(QAbstractButton *button);
    void onEffectLabelClicked(const QUrl &url);
};

#endif // GENERALOVERVIEW_H
