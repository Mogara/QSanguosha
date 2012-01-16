#ifndef AUDIO_H
#define AUDIO_H

#ifdef AUDIO_SUPPORT

#include <QString>

class Audio{
public:
    static void init();
    static void quit();

    static void play(const QString &filename);
    static void stop();

    static void playBGM(const QString &filename);
    static void setBGMVolume(float volume);
    static void stopBGM();
};

#endif

#endif // AUDIO_H
