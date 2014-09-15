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

#ifndef _CHAT_WIDGET_H
#define _CHAT_WIDGET_H

#include <QObject>
#include <QIcon>
#include <QPixmap>
#include <QGraphicsObject>
#include <QPushButton>
#include <QGraphicsProxyWidget>
#include <QGraphicsPixmapItem>

class MyPixmapItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    MyPixmapItem(const QPixmap &pixmap, QGraphicsItem *parentItem = 0);
    ~MyPixmapItem();

public:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void setSize(int x, int y);
    QString itemName;

private:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void initFaceBoardPos();
    void initEasyTextPos();
    int mouseCanClick(int x, int y);
    int mouseOnIcon(int x, int y);
    int mouseOnText(int x, int y);

    int sizex;
    int sizey;
    QList <QRect> faceboardPos;
    QList <QRect> easytextPos;
    QList <QString> easytext;

signals:
    void my_pixmap_item_msg(QString);
};

class ChatWidget : public QGraphicsObject {
    Q_OBJECT

public:
    ChatWidget();
    ~ChatWidget();
    virtual QRectF boundingRect() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QPixmap base_pixmap;
    QPushButton *returnButton;
    QPushButton *chatfaceButton;
    QPushButton *easytextButton;
    MyPixmapItem *chat_face_board, *easy_text_board;
    QGraphicsRectItem *base;

    QGraphicsProxyWidget *addWidget(QWidget *widget, int x);
    QPushButton *addButton(const QString &name, int x);
    QPushButton *createButton(const QString &name);

private slots:
    void showEasyTextBoard();
    void showFaceBoard();
    void sendText();

signals:
    void chat_widget_msg(QString);
    void return_button_click();
};

#endif

