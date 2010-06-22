#ifndef STARTSCENE_H
#define STARTSCENE_H

#include "button.h"

#include <QGraphicsScene>

class StartScene: public QGraphicsScene{
Q_OBJECT

private:
    Button *start_game, *drama_mode, *challenge_mode, *general_preview, *acknowledgement, *quit;
private slots:
    void startGame();
    void leave();

public:
    StartScene();

signals:
    void switch_to_scene(QGraphicsScene *);
};

#endif // STARTSCENE_H
