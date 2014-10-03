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

#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <QObject>
#include <QMutex>
#include <QFont>
#include <QColor>

#include "settings.h"

class QPushButton;

class StyleHelper : public QObject
{

private:
    explicit StyleHelper(QObject *parent = 0);
    QFont iconFont;
    static StyleHelper *instance;

public:
    static StyleHelper *getInstance();

    void setIcon(QPushButton *button, QChar iconId, int size = 10);

    static QFont getFontByFileName(const QString &fileName);

    inline static QString styleSheetOfTooltip()
    {
        return QString("QToolTip{ border: 1px solid rgb(166, 150, 122); padding: 1px; "
                       "border-radius: 3px; background: %1; opacity: 230; }")
                .arg(Config.ToolTipBackgroundColor.name());
    }

    static QString styleSheetOfScrollBar();

    static QColor backgroundColorOfFlatDialog();
};

#endif // STYLEHELPER_H
