/********************************************************************
    Copyright (c) 2013-2015 - Mogara

    This file is part of Cardirector.

    This game engine is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the LICENSE file for more details.

    Mogara
    *********************************************************************/

#include "cmainwindow.h"

#include <QGuiApplication>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    app.setOrganizationName("Mogara");
    app.setOrganizationDomain("mogara.org");
    app.setApplicationName("QSanguosha");

    CMainWindow window(QUrl(QStringLiteral("qrc:/main.qml")));
    // Warning: DO NOT CALL window.show() HERE, USE QWindow::setVisible(true) INSTEAD
    // show() is equivalent to calling showFullScreen(), showMaximized(), or
    // showNormal(), depending on the platform's default behavior for the window type
    // and flags. So the last state stored in CMainWindow will be ignored.
    window.setVisible(true);

    return app.exec();
}
