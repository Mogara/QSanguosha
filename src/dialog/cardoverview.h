#ifndef CARDOVERVIEW_H
#define CARDOVERVIEW_H

#include "card.h"

#include <QDialog>
#include <QTableWidgetItem>

class MainWindow;
namespace Ui {
    class CardOverview;
}

class CardOverview : public QDialog {
    Q_OBJECT
public:
    static CardOverview *getInstance(QWidget *main_window);

    CardOverview(QWidget *parent = 0);
    void loadFromAll();
    void loadFromList(const QList<const Card*> &list);

    ~CardOverview();

private:
    Ui::CardOverview *ui;

    void addCard(int i, const Card *card);

private slots:
    void on_femalePlayButton_clicked();
    void on_malePlayButton_clicked();
    void on_tableWidget_itemDoubleClicked(QTableWidgetItem* item);
    void on_tableWidget_itemSelectionChanged();
    void askCard();
};

#endif // CARDOVERVIEW_H
