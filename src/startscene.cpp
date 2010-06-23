#include "startscene.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

StartScene::StartScene(QWidget *parent)
    :QGraphicsScene(parent)
{
    setBackgroundBrush(QBrush(QPixmap(":/images/background.png")));

    // game logo
    QGraphicsPixmapItem *logo = addPixmap(QPixmap(":/images/logo.png"));
    logo->setOffset(-logo->pixmap().width()/2, -logo->pixmap().height()/2);
    logo->setPos(0, -Config.Rect.height()/4);

    MainWindow *main_window = qobject_cast<MainWindow*>(parent);
    Ui::MainWindow *ui = main_window->ui;

    QList<QAction*> actions;
    actions << ui->actionStart_Game << ui->actionConfigure << ui->actionStart_Server
            << ui->actionGeneral_Preview << ui->actionAcknowledgement << ui->actionExit;

    qreal menu_height = Config.BigFont.pixelSize();
    int i;
    for(i=0; i< actions.size(); i++){
        Button *button = new Button(actions[i]->text());
        connect(button, SIGNAL(clicked()), actions[i], SLOT(trigger()));
        button->setPos(0, (i-0.8)*menu_height);

        addItem(button);
    }

    //my e-mail address
    QFont email_font(Config.SmallFont);
    email_font.setStyle(QFont::StyleItalic);
    QGraphicsSimpleTextItem *email_text = addSimpleText("moligaloo@gmail.com", email_font);
    email_text->setBrush(Qt::white);
    email_text->setPos(Config.Rect.width()/2 - email_text->boundingRect().width(),
                       Config.Rect.height()/2 - email_text->boundingRect().height());
}

