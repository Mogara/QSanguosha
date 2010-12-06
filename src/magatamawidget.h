#ifndef MAGATAMAWIDGET_H
#define MAGATAMAWIDGET_H

#include <QWidget>

class Player;

class MagatamaWidget : public QWidget{
    Q_OBJECT

public:
    explicit MagatamaWidget(int hp, Qt::Orientation orientation);

    static QPixmap *GetMagatama(int index);
    static QPixmap *GetSmallMagatama(int index);
};

#endif // MAGATAMAWIDGET_H
