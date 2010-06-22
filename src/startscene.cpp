#include "startscene.h"

#include "roomscene.h"

StartScene::StartScene(){
    setBackgroundBrush(QBrush(QPixmap(":/images/background.png")));

    QGraphicsPixmapItem *logo = addPixmap(QPixmap(":/images/logo.png"));
    logo->setOffset(-logo->pixmap().width()/2, -logo->pixmap().height()/2);
    logo->setPos(0, -Config.Rect.height()/4);

    start_game = new Button(tr("Start Game"));
    drama_mode = new Button(tr("Drama Mode"));
    challenge_mode = new Button(tr("Challenge Mode"));
    general_preview = new Button(tr("General Preview"));
    acknowledgement = new Button(tr("Acknowledgement"));
    quit = new Button(tr("Quit"));

    connect(start_game, SIGNAL(clicked()), SLOT(startGame()));
    connect(quit, SIGNAL(clicked()), SLOT(leave()));

    QList<Button*> menus;
    menus << start_game << drama_mode << challenge_mode << general_preview << acknowledgement << quit;

    qreal menu_height = Config.BigFont.pixelSize();
    int i;
    for(i=0; i< menus.size(); i++){
        addItem(menus[i]);
        menus[i]->setPos(0, (i-0.8)*menu_height);
    }

    //my e-mail address
    QFont email_font(Config.SmallFont);
    email_font.setStyle(QFont::StyleItalic);
    QGraphicsSimpleTextItem *email_text = addSimpleText("moligaloo@gmail.com", email_font);
    email_text->setBrush(Qt::white);
    email_text->setPos(Config.Rect.width()/2 - email_text->boundingRect().width(),
                       Config.Rect.height()/2 - email_text->boundingRect().height());
}

void StartScene::startGame(){
    emit switch_to_scene(new RoomScene);
}

void StartScene::leave(){
    emit switch_to_scene(NULL);
}
