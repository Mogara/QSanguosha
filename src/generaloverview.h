#ifndef GENERALOVERVIEW_H
#define GENERALOVERVIEW_H

#include <QDialog>
#include <QTableWidgetItem>

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

private slots:

private slots:
    void on_playEffecButton_clicked();
    void on_tableWidget_itemSelectionChanged();
};

#endif // GENERALOVERVIEW_H
