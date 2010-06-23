#ifndef STARTSCENE_H
#define STARTSCENE_H

#include "button.h"
#include "pixmap.h"

#include <QGraphicsScene>
#include <QAction>
#include <QTextEdit>

class StartScene: public QGraphicsScene{
Q_OBJECT
public:
    StartScene();
    void addButton(QAction *action);
    void leave();

private:
    Pixmap *logo;
    QList<Button*> buttons;
    QTextEdit *server_log;

private slots:
    void showServerLog();
};

#endif // STARTSCENE_H
