/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#include "bubblechatbox.h"
#include "settings.h"
#include "stylehelper.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QBitmap>
#include <QPropertyAnimation>

static const int PixelsPerLine = 168;
static const int BoxMinWidth = 42;
static const int MaxLineCount = 2;
static const int ChatFaceWidth = 16;
static const int BoxLeftFrameWidth = 6;
static const int BoxRightFrameWidth = 10;
static const int BoxFrameHeight = 28;
static const int AnimationDuration = 500;

BubbleChatBox::BubbleChatBox(const QRect &area, QGraphicsItem *parent)
    : QGraphicsObject(parent), backgroundPixmap("image/system/bubble.png"),
    rect(backgroundPixmap.rect()), area(area), chatLabel(new BubbleChatLabel(this)),
    appearAndDisappear(new QPropertyAnimation(this, "opacity", this))
{
    chatLabel->setFont(StyleHelper::getFontByFileName("wqy-microhei.ttc"));
    chatLabel->setWrapMode(QTextOption::WrapAnywhere);

    setFlag(ItemClipsChildrenToShape);
    setOpacity(0);

    connect(&timer, SIGNAL(timeout()), this, SLOT(clear()));

    appearAndDisappear->setStartValue(0);
    appearAndDisappear->setEndValue(1);
    appearAndDisappear->setDuration(AnimationDuration);
}

BubbleChatBox::~BubbleChatBox()
{
    timer.stop();
}

QRectF BubbleChatBox::boundingRect() const
{
    return rect;
}

void BubbleChatBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter->drawPixmap(rect.toRect(), backgroundPixmap);
}

QPainterPath BubbleChatBox::shape() const
{
    QRegion maskRegion(backgroundPixmap.mask().scaled(rect.size().toSize()));
    QPainterPath path;
    path.addRegion(maskRegion);
    return path;
}

void BubbleChatBox::setText(const QString &text)
{
    chatLabel->setHtml(text);

    QString plainText = chatLabel->toPlainText();
    if (plainText.isEmpty()) {
        return;
    }

    QFontMetrics fontMetrics(chatLabel->font());
    int imageCount = text.count("</img>");
    int width = qAbs(fontMetrics.width(plainText)) + imageCount * ChatFaceWidth;
    int lineCount = 1;
    if (width > PixelsPerLine) {
        lineCount = width / PixelsPerLine;
        if (lineCount >= MaxLineCount) {
            lineCount = MaxLineCount;
        } else if (width % PixelsPerLine != 0) {
            ++lineCount;
        }

        width = PixelsPerLine;
    }

    int boxWidth = width + fontMetrics.maxWidth();
    if (boxWidth <= BoxMinWidth) {
        boxWidth = BoxMinWidth;
        chatLabel->setAlignment(Qt::AlignHCenter);
    } else {
        chatLabel->setAlignment(Qt::AlignLeft);
    }
    chatLabel->setTextWidth(boxWidth);

    QRectF oldRect = rect;

    int height = fontMetrics.lineSpacing() + fontMetrics.xHeight();
    rect.setSize(QSize(boxWidth + BoxRightFrameWidth, height * lineCount + BoxFrameHeight));

    chatLabel->setPos(QPointF(BoxLeftFrameWidth,
        rect.center().y() - (height * lineCount) + (lineCount - 1) * (height / 2) - (imageCount > 0 ? 1 : 0)));
    chatLabel->setBoundingRect(QRectF(0, 0, boxWidth, height * lineCount + (MaxLineCount - lineCount) * 1));

    updatePos();

    if (opacity() != 1) {
        appearAndDisappear->setDirection(QAbstractAnimation::Forward);
        appearAndDisappear->start();
    }

    if (oldRect.width() > rect.width()) {
        QRectF sceneRect = mapRectToScene(oldRect);
        scene()->update(sceneRect);
    } else {
        update();
    }

    timer.start(Config.BubbleChatBoxKeepSeconds * 1000 - AnimationDuration);
}

void BubbleChatBox::setArea(const QRect &newArea)
{
    area = newArea;
    updatePos();
}

QVariant BubbleChatBox::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneHasChanged && scene() && !scene()->items().contains(chatLabel))
        scene()->addItem(chatLabel);

    return QGraphicsItem::itemChange(change, value);
}

void BubbleChatBox::updatePos()
{
    const int xOffset = (area.width() - rect.width()) / 2;
    const int yOffset = (area.height() -rect.height()) / 2;
    setPos(QPointF(area.left() + xOffset, area.top() + yOffset));
}

void BubbleChatBox::clear()
{
    timer.stop();
    appearAndDisappear->setDirection(QAbstractAnimation::Backward);
    appearAndDisappear->start();
}

BubbleChatBox::BubbleChatLabel::BubbleChatLabel(QGraphicsItem *parent)
    : QGraphicsTextItem(parent), doc(document())
{

}

QRectF BubbleChatBox::BubbleChatLabel::boundingRect() const
{
    return rect;
}

void BubbleChatBox::BubbleChatLabel::setBoundingRect(const QRectF &newRect)
{
    rect = newRect;
}

void BubbleChatBox::BubbleChatLabel::setAlignment(Qt::Alignment alignment)
{
    QTextOption option = doc->defaultTextOption();
    option.setAlignment(alignment);
    doc->setDefaultTextOption(option);
}

void BubbleChatBox::BubbleChatLabel::setWrapMode(QTextOption::WrapMode wrap)
{
    QTextOption option = doc->defaultTextOption();
    option.setWrapMode(wrap);
    doc->setDefaultTextOption(option);
}
