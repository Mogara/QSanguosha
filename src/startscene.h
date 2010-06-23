#ifndef STARTSCENE_H
#define STARTSCENE_H

#include "button.h"
#include "pixmap.h"
#include "server.h"

#include <QGraphicsScene>
#include <QAction>
#include <QTextEdit>

class StartScene: public QGraphicsScene{
Q_OBJECT
public:
    StartScene();
    void addButton(QAction *action);
    void switchToServer(Server *server);

private:
    Pixmap *logo;
    QList<Button*> buttons;
};

#endif // STARTSCENE_H
