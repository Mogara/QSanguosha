#include "button.h"
#include "audio.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRotation>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

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

	title = new QPixmap(size.toSize());
	title->fill(QColor(0,0,0,0));
	QPainter pt(title);
	pt.setFont(font);
	pt.setPen(Config.TextEditColor);
	pt.setRenderHint(QPainter::TextAntialiasing);
	pt.drawText(boundingRect(), Qt::AlignCenter, label);

	title_item = new QGraphicsPixmapItem(this);
	title_item->setPixmap(*title);
	title_item->show();

	QGraphicsDropShadowEffect *de = new QGraphicsDropShadowEffect;
	de->setOffset(0);
	de->setBlurRadius(12);
	de->setColor(QColor(255,165,0));

	title_item->setGraphicsEffect(de);
	
	QImage bgimg("image/system/button/button.png");
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
	effect->setBlurRadius(5);
	effect->setOffset(this->boundingRect().height()/7.0);
	effect->setColor(QColor(0, 0, 0, 200));
	this->setGraphicsEffect(effect);
	
	glow = 0;

	timer_id = 0;
}

void Button::setMute(bool mute){
	this->mute = mute;
}

void Button::setFont(const QFont &font){
	this->font = font;
	title->fill(QColor(0,0,0,0));
	QPainter pt(title);
	pt.setFont(font);
	pt.setPen(Config.TextEditColor);
	pt.setRenderHint(QPainter::TextAntialiasing);
	pt.drawText(boundingRect(), Qt::AlignCenter, label);

	title_item->setPixmap(*title);
}

#include "engine.h"

void Button::hoverEnterEvent(QGraphicsSceneHoverEvent *){
	setFocus(Qt::MouseFocusReason);

#ifdef AUDIO_SUPPORT

	if(!mute)
		Sanguosha->playSystemAudioEffect("button-hover");

#endif

	if(!timer_id)timer_id = QObject::startTimer(40);
}

void Button::mousePressEvent(QGraphicsSceneMouseEvent *event){
	event->accept();
}

void Button::mouseReleaseEvent(QGraphicsSceneMouseEvent *){
#ifdef AUDIO_SUPPORT

	if(!mute)
		Sanguosha->playSystemAudioEffect("button-down");

#endif

	emit clicked();
}

QRectF Button::boundingRect() const{
	return QRectF(QPointF(), size);
}

void Button::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
	QRectF rect = boundingRect();

	//painter->setOpacity(0.8);
	painter->drawImage(rect,*outimg);
	painter->fillRect(rect,QColor(255,255,255,glow*10));
	//painter->drawPixmap(rect.toRect(),*title);
}

void Button::timerEvent(QTimerEvent *)
{
	update();
	if(hasFocus())
	{
		if(glow<5)glow++;
	}else
	{
		if(glow>0)glow--;
		else if(timer_id)
		{
			QObject::killTimer(timer_id);
			timer_id = 0;
		}
	}
}
