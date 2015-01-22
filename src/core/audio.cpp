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

#include "audio.h"
#include "settings.h"

#include <QCache>
#include <fmod.hpp>

class Sound;

static FMOD_SYSTEM *System;
static QCache<QString, Sound> SoundCache;
static FMOD_SOUND *BGM;
static FMOD_CHANNEL *BGMChannel;

class Sound {
public:
    Sound(const QString &filename) : sound(NULL), channel(NULL) {
        FMOD_System_CreateSound(System, filename.toLatin1(), FMOD_DEFAULT, NULL, &sound);
    }

    ~Sound() {
        if (sound) FMOD_Sound_Release(sound);
    }

    void play(const bool doubleVolume = false) {
        if (sound) {
            FMOD_RESULT result = FMOD_System_PlaySound(System, FMOD_CHANNEL_FREE, sound, false, &channel);

            if (result == FMOD_OK) {
                FMOD_Channel_SetVolume(channel, (doubleVolume ? 2 : 1) * Config.EffectVolume);
                FMOD_System_Update(System);
            }
        }
    }

    bool isPlaying() const {
        if (channel == NULL) return false;

        FMOD_BOOL is_playing = false;
        FMOD_Channel_IsPlaying(channel, &is_playing);
        return is_playing;
    }

private:
    FMOD_SOUND *sound;
    FMOD_CHANNEL *channel;
};

void Audio::init() {
    FMOD_RESULT result = FMOD_System_Create(&System);
    if (result == FMOD_OK) FMOD_System_Init(System, 100, 0, NULL);
}

void Audio::quit() {
    if (System) {
        SoundCache.clear();
        FMOD_System_Release(System);

        System = NULL;
    }
}

void Audio::play(const QString &filename) {
    Sound *sound = SoundCache[filename];
    if (sound == NULL) {
        sound = new Sound(filename);
        SoundCache.insert(filename, sound);
    } else if (sound->isPlaying()) {
        return;
    }

    sound->play();
}

void Audio::playAudioOfMoxuan()
{
    Sound *sound = new Sound("audio/system/moxuan.ogg");
    sound->play(true);
}

void Audio::stop() {
    if (System == NULL) return;

    int n;
    FMOD_System_GetChannelsPlaying(System, &n);

    QList<FMOD_CHANNEL *> channels;
    for (int i = 0; i < n; i++) {
        FMOD_CHANNEL *channel;
        FMOD_RESULT result = FMOD_System_GetChannel(System, i, &channel);
        if (result == FMOD_OK) channels << channel;
    }

    foreach (FMOD_CHANNEL * const &channel, channels)
        FMOD_Channel_Stop(channel);

    stopBGM();

    FMOD_System_Update(System);
}

void Audio::playBGM(const QString &filename) {
    FMOD_RESULT result = FMOD_System_CreateStream(System, filename.toLocal8Bit(), FMOD_LOOP_NORMAL, NULL, &BGM);

    if (result == FMOD_OK) {
        FMOD_Sound_SetLoopCount(BGM, -1);
        FMOD_System_PlaySound(System, FMOD_CHANNEL_FREE, BGM, false, &BGMChannel);

        FMOD_Channel_SetVolume(BGMChannel, Config.BGMVolume);
        FMOD_System_Update(System);
    }
}

void Audio::setBGMVolume(float volume) {
    if (BGMChannel) FMOD_Channel_SetVolume(BGMChannel, volume);
}

void Audio::stopBGM() {
    if (BGMChannel) FMOD_Channel_Stop(BGMChannel);
}

QString Audio::getVersion() {
    unsigned int version = 0;
    FMOD_System_GetVersion(System, &version);
    // convert it to QString
    return QString("%1.%2.%3").arg((version & 0xFFFF0000) >> 16, 0, 16)
        .arg((version & 0xFF00) >> 8, 2, 16, QChar('0'))
        .arg((version & 0xFF), 2, 16, QChar('0'));
}

