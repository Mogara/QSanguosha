#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "pixmap.h"
#include "carditem.h"
#include "general.h"

#include <QPushButton>
#include <QComboBox>

class Dashboard : public Pixmap
{
    Q_OBJECT

public:
    Dashboard();
    void addCardItem(CardItem *card_item);
    void setGeneral(General *general);
    Pixmap *getAvatar();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QList<CardItem*> card_items;
    General *general;
    QPixmap magatamas[5];
    Pixmap *avatar;
    bool use_skill;
    QComboBox *sort_combobox;

    void adjustCards();

private slots:
    void sortCards();
};

#endif // DASHBOARD_H
