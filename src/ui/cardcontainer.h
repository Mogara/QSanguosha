#ifndef _CARD_CONTAINER_H
#define _CARD_CONTAINER_H

class ClientPlayer;

#include "carditem.h"
#include "GenericCardContainerUI.h"

#include <QStack>

class CardContainer: public GenericCardContainer {
    Q_OBJECT

public:
    explicit CardContainer();
    virtual QList<CardItem *> removeCardItems(const QList<int> &card_ids, Player::Place place);
    int getFirstEnabled() const;
    void startChoose();
    void startGongxin(const QList<int> &enabled_ids);
    //************************************
    // Method:    addConfirmButton
    // FullName:  CardContainer::addConfirmButton
    // Access:    public 
    // Returns:   void
    // Qualifier:
    // Description: Show a confirm button. The container will be closed immediately when click the
    // button.
    //
    // Last Updated By Yanguam Siliagim
    // To fix no-response when click "confirm" in pile box
    //
    // QSanguosha-Hegemony Team
    // March 14 2014
    //************************************
    void addConfirmButton();
    void view(const ClientPlayer *player);
    virtual QRectF boundingRect() const;
    ClientPlayer *m_currentPlayer;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    bool retained();

public slots:
    void fillCards(const QList<int> &card_ids = QList<int>(), const QList<int> &disabled_ids = QList<int>());
    void clear();
    void freezeCards(bool is_disable);

protected:
    virtual bool _addCardItems(QList<CardItem *> &card_items, const CardsMoveStruct &moveInfo);
    QSanButton *confirm_button;
    int scene_width;
    int item_count;

private:
    QList<CardItem *> items;
    QStack<QList<CardItem *> > items_stack;
    QStack<bool> retained_stack;

    void _addCardItem(int card_id, const QPointF &pos);

private slots:
    void grabItem();
    void chooseItem();
    void gongxinItem();

signals:
    void item_chosen(int card_id);
    void item_gongxined(int card_id);
};

class GuanxingBox: public CardContainer {
    Q_OBJECT

public:
    GuanxingBox();
    void clear();
    void reply();
    virtual QRectF boundingRect() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public slots:
    void doGuanxing(const QList<int> &card_ids, bool up_only);
    void adjust();

private:
    QList<CardItem *> up_items, down_items;
    bool up_only;
};

#endif

