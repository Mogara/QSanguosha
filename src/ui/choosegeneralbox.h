#ifndef _CHOOSE_GENERAL_BOX_H
#define _CHOOSE_GENERAL_BOX_H

#include "carditem.h"
#include "qsanbutton.h"
#include "TimedProgressBar.h"
#include "sprite.h"

class GeneralCardItem: public CardItem {
    Q_OBJECT

public:
    GeneralCardItem(const QString &general_name);

    void showCompanion();
    void hideCompanion();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    bool has_companion;
};

class ChooseGeneralBox: public QGraphicsObject {
    Q_OBJECT

public:
    explicit ChooseGeneralBox();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    QRectF boundingRect() const;
    void clear();
    void adjustItems();
    inline void setSingleResult(const bool single_result) {    this->single_result = single_result;    };

public slots:
    void chooseGeneral(QStringList generals);
    void reply();

private:
    int general_number;
    QList<GeneralCardItem *> items, selected;
    static const int top_dark_bar = 27;
    static const int top_blank_width = 42;
    static const int bottom_blank_width = 68;
    static const int card_bottom_to_split_line = 23;
    static const int card_to_center_line = 5;
    static const int left_blank_width = 37;
    static const int split_line_to_card_seat = 15;

    //data index
    static const int S_DATA_INITIAL_HOME_POS = 9527;

    QSanButton *confirm;
    QGraphicsProxyWidget *progress_bar_item;
    QSanCommandProgressBar *progress_bar;

    bool single_result;

    EffectAnimation *animations;

    void _initializeItems();

private slots:
    void _adjust();
    void _onItemClicked();
    void _onCardItemHover();
    void _onCardItemLeaveHover();
};

#endif // _CHOOSE_GENERAL_BOX_H