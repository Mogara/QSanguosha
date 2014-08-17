/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3.0 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Hegemony Team
    *********************************************************************/

#include "FlatDialog.h"

#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QVBoxLayout>
#include <QMouseEvent>

FlatDialog::FlatDialog(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::Dialog),
      title(new QLabel), layout(new QVBoxLayout)
{
    setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(18);
    effect->setOffset(0);
    effect->setColor(Qt::cyan);
    setGraphicsEffect(effect);

    title->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    title->setObjectName("window_title");
    connect(this, SIGNAL(windowTitleChanged(QString)), title, SLOT(setText(QString)));

    layout->addWidget(title);
    setLayout(layout);
}

void FlatDialog::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(214, 231, 239, 75));
    painter.drawRoundedRect(rect(), 5, 5);
}

void FlatDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() & Qt::LeftButton)
        mousePressedPoint = event->globalPos() - frameGeometry().topLeft();
}

void FlatDialog::mouseMoveEvent(QMouseEvent *event)
{
    QPoint path = event->globalPos() - mousePressedPoint;
    move(path);
}


