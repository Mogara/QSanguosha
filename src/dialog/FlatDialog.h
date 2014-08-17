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

#ifndef FLATDIALOG_H
#define FLATDIALOG_H

#include <QDialog>

class QLabel;
class QVBoxLayout;

class FlatDialog : public QDialog {
    Q_OBJECT

protected:
    FlatDialog(QWidget *parent);

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    QVBoxLayout *layout;

private:
    QLabel *title;
    QPoint mousePressedPoint;
};

#endif // FLATDIALOG_H
