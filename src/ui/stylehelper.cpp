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

#include "stylehelper.h"
#include "engine.h"
#include "util.h"

#include <QApplication>
#include <QFontDatabase>
#include <QPushButton>
#include <QMap>
#include <QFile>
#include <QTextStream>

StyleHelper *StyleHelper::instance = NULL;

StyleHelper::StyleHelper(QObject *):
    QObject(qApp)
{
    iconFont = getFontByFileName("fontawesome-webfont.ttf");
}

StyleHelper *StyleHelper::getInstance()
{
    static QMutex mutex;
    if (!instance) {
        QMutexLocker locker(&mutex);
        if (!instance) {
            instance = new StyleHelper;
        }
    }
    return instance;
}

QFont StyleHelper::getFontByFileName(const QString &fileName)
{
    static QMap<QString, QFont> loadedFonts;
    if (loadedFonts.contains(fileName)) {
        return loadedFonts.value(fileName);
    } else {
        int fontId = QFontDatabase::addApplicationFont("font/" + fileName);
        Q_ASSERT(fontId != -1);
        QString fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont font(fontName);
        loadedFonts[fileName] = font;
        return font;
    }
}

QString StyleHelper::styleSheetOfScrollBar()
{
    static QString style;
    if (style.isEmpty()) {
        QFile file("style-sheet/scroll.qss");
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            style = stream.readAll();
        }
    }
    return style;
}

QColor StyleHelper::backgroundColorOfFlatDialog()
{
    static QColor color;
    static bool loaded = false;
    if (!loaded) {
        lua_State *L = Sanguosha->getLuaState();
        const QString colorString = GetConfigFromLuaState(L, "dialog_background_color").toString();
        color.setNamedColor(colorString);
        if (!color.isValid()) {
            qWarning("Invalid color for dialog background");
            color.setRgb(214, 231, 239);
        }
        const int alpha = GetConfigFromLuaState(L, "dialog_background_alpha").toInt();
        color.setAlpha(qBound(0, alpha, 255));
        loaded = true;
    }
    return color;
}

void StyleHelper::setIcon(QPushButton* button, QChar iconId, int size)
{
    iconFont.setPointSize(size);
    button->setFont(iconFont);
    button->setText(iconId);
}
