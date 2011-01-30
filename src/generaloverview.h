#ifndef GENERALOVERVIEW_H
#define GENERALOVERVIEW_H

class General;
class Skill;

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

private:
    Ui::GeneralOverview *ui;
    QVBoxLayout *button_layout;

    void resetButtons();
    void addLines(const Skill *skill);

private slots:
    void playEffect();
    void on_tableWidget_itemSelectionChanged();
};

#endif // GENERALOVERVIEW_H
