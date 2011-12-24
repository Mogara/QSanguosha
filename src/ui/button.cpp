#include "button.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRotation>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

#ifdef AUDIO_SUPPORT

#ifdef  Q_OS_WIN32
    #include "irrKlang.h"
    extern irrklang::ISoundEngine *SoundEngine;
#else
    #include <phonon/MediaObject>
    extern Phonon::MediaObject *SoundEngine;
#endif
#endif

static QRectF ButtonRect(0, 0, 189, 46);

Button::Button(const QString &label, qreal scale)
    :label(label), size(ButtonRect.size() * scale),
    mute(true), font(Config.SmallFont)
{

    init();
}

Button::Button(const QString &label, const QSizeF &size)
    :label(label), size(size), mute(true), font(Config.SmallFont)
{
    init();
}

void Button::init()
{
    setFlags(ItemIsFocusable);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);

    QPixmap *bg = new QPixmap("image/system/button/button.png");
    QImage bgimg = bg->toImage();
    outimg = new QImage(size.toSize(),QImage::Format_ARGB32);

    qreal pad = 10;

    int w = bgimg.width();
    int h = bgimg.height();

    int tw = outimg->width();
    int th  =outimg->height();

    qreal xc = (w - 2*pad)/(tw - 2*pad);
    qreal yc = (h - 2*pad)/(th - 2*pad);

    for(int i=0;i<tw;i++)
        for(int j=0;j<th;j++)
        {
            int x = i;
            int y = j;

            if( x>=pad && x<=(tw - pad) ) x = pad + (x - pad)*xc;
            else if(x>=(tw-pad))x = w - (tw - x);

            if( y>=pad && y<=(th - pad) ) y = pad + (y - pad)*yc;
            else if(y>=(th-pad))y = h - (th - y);


            QRgb rgb = bgimg.pixel(x,y);
            outimg->setPixel(i,j,rgb);
        }

    QGraphicsDropShadowEffect * effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(10);
    effect->setColor(QColor(0,0,0,100));
    this->setGraphicsEffect(effect);

    glow = 0;
}

void Button::setMute(bool mute){
    this->mute = mute;
}

void Button::setFont(const QFont &font){
    this->font = font;
}

void Button::hoverEnterEvent(QGraphicsSceneHoverEvent *){
    setFocus(Qt::MouseFocusReason);

#ifdef AUDIO_SUPPORT
    if(SoundEngine && !mute) {
#ifdef Q_OS_WIN32
        SoundEngine->play2D("audio/system/button-hover.ogg");
#else
        SoundEngine->setCurrentSource(Phonon::MediaSource("audio/system/button-hover.ogg"));
        SoundEngine->play();
#endif
    }
#endif
    timer_id = QObject::startTimer(40);
}

void Button::hoverLeaveEvent(QGraphicsSceneHoverEvent *){
    timer_id = QObject::startTimer(40);
}

void Button::mousePressEvent(QGraphicsSceneMouseEvent *event){
    event->accept();
}

void Button::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
#ifdef AUDIO_SUPPORT
    if(SoundEngine && !mute) {
#ifdef Q_OS_WIN32
        SoundEngine->play2D("audio/system/button-down.ogg");
#else
        SoundEngine->setCurrentSource(Phonon::MediaSource("audio/system/button-down.ogg"));
        SoundEngine->play();
#endif
    }
#endif


    emit clicked();
}

QRectF Button::boundingRect() const{
    return QRectF(QPointF(), size);
}

void Button::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QRectF rect = boundingRect();

//    QPainterPath path;
//    path.addRoundedRect(rect, 5, 5);

//    QColor rect_color(Qt::black);
//    if(hasFocus())
//        rect_color = QColor("orange");

//    rect_color.setAlpha(0.43 * 255);
//    painter->fillPath(path, rect_color);

//    QPen pen(Config.TextEditColor);
//    pen.setWidth(3);
//    painter->setPen(pen);
//    painter->drawPath(path);

    painter->drawImage(rect,*outimg);


    if(hasFocus())
    {
        if(glow<5)glow++;
        else QObject::killTimer(timer_id);
    }else
    {
        if(glow>0)glow--;
        else QObject::killTimer(timer_id);
    }
    painter->fillRect(rect,QColor(255,255,255,glow*10));

    painter->setFont(font);
    painter->setPen(Config.TextEditColor);
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->drawText(rect, Qt::AlignCenter, label);
}

void Button::timerEvent(QTimerEvent *)
{
    update();
}
