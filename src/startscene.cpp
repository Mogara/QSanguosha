#include "startscene.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

StartScene::StartScene()
{
    setBackgroundBrush(Config.BackgroundBrush);

    // game logo
    logo = new Pixmap(":/logo.png");
    logo->shift();
    logo->moveBy(0, -Config.Rect.height()/4);
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

#include <QNetworkInterface>

void StartScene::switchToServer(Server *server){
    // performs leaving animation
    QPropertyAnimation *logo_shift = new QPropertyAnimation(logo, "pos");
    logo_shift->setEndValue(Config.Rect.topLeft());

    QPropertyAnimation *logo_shrink = new QPropertyAnimation(logo, "scale");
    logo_shrink->setEndValue(0.5);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(logo_shift);
    group->addAnimation(logo_shrink);
    group->start(QAbstractAnimation::DeleteWhenStopped);

    foreach(Button *button, buttons){
        removeItem(button);
        delete button;
    }
    buttons.clear();

    QTextEdit *server_log = new QTextEdit();

    // make its background the same as background, looks transparent    
    QPalette palette;
    palette.setBrush(QPalette::Base, Config.BackgroundBrush);

    server_log->setReadOnly(true);
    server_log->setPalette(palette);
    server_log->resize(700, 420);
    server_log->move(-400, -180);
    server_log->setFrameShape(QFrame::NoFrame);

    server_log->setFont(QFont("Verdana", 12));
    server_log->setTextColor(QColor("white"));

    addWidget(server_log);

    QStringList items;
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    foreach(QHostAddress address, addresses){
        quint32 ipv4 = address.toIPv4Address();
        if(ipv4)
            items << address.toString();
    }

    qSort(items);

    foreach(QString item, items){
        if(item.startsWith("192.168."))
            server_log->append(tr("Your LAN address: %1, this address is available only for hosts that in the same LAN").arg(item));
        else if(item == "127.0.0.1")
            server_log->append(tr("Your loopback address %1, this address is available only for your host").arg(item));
        else if(item.startsWith("5."))
            server_log->append(tr("Your Hamachi address: %1, the address is available for users that joined the same Hamachi network").arg(item));
        else if(!item.startsWith("169.254."))
            server_log->append(tr("Your other address: %1, if this is a public IP, that will be available for all cases").arg(item));
    }

    server_log->append(tr("Binding port number is %1").arg(Config.Port));
    server_log->append(tr("Player count is %1").arg(Config.PlayerCount));

    if(Config.OperationNoLimit)
        server_log->append(tr("There is no time limit"));
    else
        server_log->append(tr("Operation timeout is %1 seconds").arg(Config.OperationTimeout));

    connect(server, SIGNAL(server_message(QString)), server_log, SLOT(append(QString)));

    update();
}
