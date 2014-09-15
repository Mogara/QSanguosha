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

#ifndef _AUDIO_H
#define _AUDIO_H

#ifdef AUDIO_SUPPORT

#include <QString>

class Audio {
public:
    static void init();
    static void quit();

    static void play(const QString &filename);
    static void playAudioOfMoxuan();
    static void stop();

    static void playBGM(const QString &filename);
    static void setBGMVolume(float volume);
    static void stopBGM();

    static QString getVersion();
};

#endif

#endif

