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

#include "quickwindow.h"
#include "settings.h"

#include <QApplication>

QuickWindow::QuickWindow()
{
    restoreFromConfig();
    setTitle(tr("QSanguosha"));
    connect(this, &QuickWindow::destroyed, this, &QuickWindow::saveWindowState);
}

void QuickWindow::restoreFromConfig()
{
    setPosition(Config.value("WindowPosition", QPoint(0, 0)).toPoint());

    if (Config.UIFont != Config.AppFont)
        QApplication::setFont(Config.UIFont, "QTextEdit");
}

void QuickWindow::saveWindowState()
{
    if (!(windowState() & (Qt::WindowMaximized | Qt::WindowFullScreen))) {
        Config.setValue("WindowSize", size());
        Config.setValue("WindowPosition", position());
    }
    Config.setValue("WindowState", (int)windowState());
}
