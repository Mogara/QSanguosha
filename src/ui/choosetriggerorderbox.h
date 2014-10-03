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

#ifndef CHOOSETRIGGERORDERBOX_H
#define CHOOSETRIGGERORDERBOX_H

#include <QGraphicsObject>

#include "graphicsbox.h"

class Button;
class QGraphicsProxyWidget;
class QSanCommandProgressBar;

class TriggerOptionButton : public QGraphicsObject {
    Q_OBJECT

    friend class ChooseTriggerOrderBox;

signals:
    void clicked();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    virtual QRectF boundingRect() const;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
    explicit TriggerOptionButton(QGraphicsObject *parent, const QString &player, const QString &skill, const int width);

    QString getGeneralNameBySkill() const;

    QString skillName;
    QString playerName;
    int width;
};

class GeneralButton : public QGraphicsObject {
    Q_OBJECT

    friend class ChooseTriggerOrderBox;

signals:
    void clicked();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    virtual QRectF boundingRect() const;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
    explicit GeneralButton(QGraphicsObject *parent, const QString &general, const bool isHead);

    QString generalName;
    bool isHead;
};

class ChooseTriggerOrderBox : public GraphicsBox {
    Q_OBJECT

public:
    explicit ChooseTriggerOrderBox();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;
    void chooseOption(const QString &reason, const QStringList &options, const bool optional);
    void clear();

public slots:
    void reply();

private:
    QList<TriggerOptionButton *> optionButtons;
    QList<GeneralButton *> generalButtons;
    static const int top_dark_bar = 27;
    static const int top_blank_width = 42;
    static const int bottom_blank_width = 25;
    static const int interval = 15;
    static const int left_blank_width = 37;

    QString reason;
    QStringList options;
    bool optional;

    Button *cancel;
    QGraphicsProxyWidget *progress_bar_item;
    QSanCommandProgressBar *progressBar;

    int getGeneralNum() const;
};

#endif // CHOOSETRIGGERORDERBOX_H
