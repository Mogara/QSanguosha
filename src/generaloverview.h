#ifndef GENERALOVERVIEW_H
#define GENERALOVERVIEW_H

class General;

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
    QButtonGroup *button_group;

    void resetButtons();
    const General *currentGeneral();

private slots:
    void on_lastWordButton_clicked();
    void on_playEffecButton_clicked();
    void on_tableWidget_itemSelectionChanged();
};

#endif // GENERALOVERVIEW_H
