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

#ifndef FLATDIALOG_H
#define FLATDIALOG_H

#include <QDialog>
#include <QColor>

class QLabel;
class QVBoxLayout;

class FlatDialog : public QDialog {
    Q_OBJECT

protected:
    FlatDialog(QWidget *parent, bool needTitle = true);

    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

    QVBoxLayout *layout;

private:
    QLabel *title;
    QPoint mousePressedPoint;
    bool mousePressed;
};

#endif // FLATDIALOG_H
