#ifndef GENERALOVERVIEW_H
#define GENERALOVERVIEW_H

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

class GeneralOverview : public QDialog {
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

private slots:
    void playEffect();
    void copyLines();
    void on_tableWidget_itemSelectionChanged();
};

#endif // GENERALOVERVIEW_H
