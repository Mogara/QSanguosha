#ifndef STARTSCENE_H
#define STARTSCENE_H

#include "button.h"

#include <QGraphicsScene>

class StartScene: public QGraphicsScene{
Q_OBJECT
public:
    StartScene(QWidget *parent);
};

#endif // STARTSCENE_H
