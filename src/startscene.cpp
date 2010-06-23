#include "startscene.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

StartScene::StartScene()
    :server_log(NULL)
{
    setBackgroundBrush(QBrush(QPixmap(":/images/background.png")));

    // game logo
    logo = new Pixmap(":/images/logo.png");
    logo->setPos(- logo->boundingRect().width()/2, -Config.Rect.height()/4 - logo->boundingRect().height()/2);
    addItem(logo);

    //my e-mail address
    QFont email_font(Config.SmallFont);
    email_font.setStyle(QFont::StyleItalic);
    QGraphicsSimpleTextItem *email_text = addSimpleText("moligaloo@gmail.com", email_font);
    email_text->setBrush(Qt::white);
    email_text->setPos(Config.Rect.width()/2 - email_text->boundingRect().width(),
                       Config.Rect.height()/2 - email_text->boundingRect().height());
}

void StartScene::addButton(QAction *action){
    qreal menu_height = Config.BigFont.pixelSize();
    Button *button = new Button(action->text());
    connect(button, SIGNAL(clicked()), action, SLOT(trigger()));
    button->setPos(0, (buttons.size()-0.8)*menu_height);

    addItem(button);
    buttons << button;
}

void StartScene::leave(){
    // performs leaving animation
    QPropertyAnimation *logo_shift = new QPropertyAnimation(logo, "pos");
    logo_shift->setEndValue(Config.Rect.topLeft());

    QPropertyAnimation *logo_shrink = new QPropertyAnimation(logo, "scale");
    logo_shrink->setEndValue(0.5);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(logo_shift);
    group->addAnimation(logo_shrink);

    foreach(Button *button, buttons){
        removeItem(button);
        delete button;
    }
    buttons.clear();

    connect(group, SIGNAL(finished()), SLOT(showServerLog()));
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void StartScene::showServerLog(){
    server_log = new QTextEdit();

    QBrush brush(QPixmap(":/images/background.png"));
    QPalette palette;
    palette.setBrush(QPalette::Base, brush);
    server_log->setReadOnly(true);
    server_log->setPalette(palette);
    server_log->resize(600, 420);
    server_log->move(-400, -180);
    server_log->setFrameShape(QFrame::NoFrame);

    server_log->setFont(QFont("Verdana", 12));
    server_log->setTextColor(QColor("white"));
    server_log->setText("hello,world");

    addWidget(server_log);
}

