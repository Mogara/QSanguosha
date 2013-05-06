#ifndef _GENERAL_OVERVIEW_H
#define _GENERAL_OVERVIEW_H

class General;
class Skill;
class QCommandLinkButton;

#include <QDialog>
#include <QTableWidgetItem>
#include <QButtonGroup>
#include <QVBoxLayout>

namespace Ui {
    class GeneralOverview;
}

class GeneralOverview: public QDialog {
    Q_OBJECT

public:
    GeneralOverview(QWidget *parent = 0);
    ~GeneralOverview();
    void fillGenerals(const QList<const General *> &generals);

private:
    Ui::GeneralOverview *ui;
    QVBoxLayout *button_layout;

    void resetButtons();
    void addLines(const Skill *skill);
    void addCopyAction(QCommandLinkButton *button);
    bool hasSkin(const QString &general_name);
    QString getIllustratorInfo(const QString &general_name);

private slots:
    void playAudioEffect();
    void copyLines();
    void askTransfiguration();
    void askChangeSkin();
    void on_tableWidget_itemSelectionChanged();
    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);
};

#endif

