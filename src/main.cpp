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

#include <QGuiApplication>
#include <QLocale>
#include <QTranslator>

#include <cmainwindow.h>
#include <cexceptionhandler.h>
#include <cnetwork.h>

int main(int argc, char *argv[])
{
    CExceptionHandler eh("./dmp");

    QGuiApplication app(argc, argv);

    app.setOrganizationName("Mogara");
    app.setOrganizationDomain("mogara.org");
    app.setApplicationName("QSanguosha");

    QTranslator translator;
    translator.load(QLocale::system().name(), QStringLiteral("translations"));
    app.installTranslator(&translator);

    CMainWindow window;
    window.setSource(QUrl(QStringLiteral("qrc:/src/main.qml")));
    window.show();

    cRegisterUrlScheme(window.title());

    return app.exec();
}
