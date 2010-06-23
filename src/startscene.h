#ifndef STARTSCENE_H
#define STARTSCENE_H

#include "button.h"

#include <QGraphicsScene>
#include <QAction>

class StartScene: public QGraphicsScene{
Q_OBJECT
public:
    StartScene();
    void addButton(QAction *action, int i);
};

#endif // STARTSCENE_H
