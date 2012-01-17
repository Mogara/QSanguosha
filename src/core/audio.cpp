#include "audio.h"
#include "fmod.h"
#include "settings.h"

#include <QCache>
#include <QtDebug>


class Sound;

static FMOD_SYSTEM *System;
static QCache<QString, Sound> SoundCache(1000);
static FMOD_SOUND *BGM;
static FMOD_CHANNEL *BGMChannel;

class Sound{
public:
    Sound(const QString &filename)
        :sound(NULL), channel(NULL)
    {
        FMOD_System_CreateSound(System, filename.toAscii(), FMOD_DEFAULT, NULL, &sound);
    }

    ~Sound(){
        if(sound)
            FMOD_Sound_Release(sound);
    }

    void play(){
        if(sound){
            FMOD_RESULT result = FMOD_System_PlaySound(System, FMOD_CHANNEL_FREE, sound, false, &channel);

            if(result == FMOD_OK){
                FMOD_Channel_SetVolume(channel, Config.EffectVolume);
                FMOD_System_Update(System);
            }
        }
    }

    bool isPlaying(){
        if(channel == NULL)
            return false;

        FMOD_BOOL is_playing = false;
        FMOD_Channel_IsPlaying(channel, &is_playing);
        return is_playing;
    }

private:
    FMOD_SOUND *sound;
    FMOD_CHANNEL *channel;
};

void Audio::init(){
    FMOD_RESULT result = FMOD_System_Create(&System);

    if(result == FMOD_OK){
        FMOD_System_Init(System, 100, 0, NULL);
    }
}

void Audio::quit(){
    if(System){
        SoundCache.clear();
        FMOD_System_Release(System);

        System = NULL;
    }
}



void Audio::play(const QString &filename){
    Sound *sound = SoundCache[filename];
    if(sound == NULL){
        sound = new Sound(filename);
        SoundCache.insert(filename, sound);
    }else if(sound->isPlaying())
        return;

    sound->play();
}

void Audio::stop(){
    if(System == NULL)
        return;

    int n;
    FMOD_System_GetChannelsPlaying(System, &n);

    QList<FMOD_CHANNEL *> channels;
    for(int i=0; i<n; i++){
        FMOD_CHANNEL *channel;
        FMOD_RESULT result = FMOD_System_GetChannel(System, i, &channel);
        if(result == FMOD_OK)
            channels << channel;
    }

    foreach(FMOD_CHANNEL *channel, channels){
        FMOD_Channel_Stop(channel);
    }

    stopBGM();

    FMOD_System_Update(System);
}

void Audio::playBGM(const QString &filename){
    FMOD_RESULT result = FMOD_System_CreateStream(System, filename.toAscii(), FMOD_DEFAULT, NULL, &BGM);

    if(result == FMOD_OK){
        FMOD_Sound_SetLoopCount(BGM, -1);
        FMOD_System_PlaySound(System, FMOD_CHANNEL_FREE, BGM, false, &BGMChannel);

        FMOD_System_Update(System);
    }
}

void Audio::setBGMVolume(float volume){
    if(BGMChannel)
        FMOD_Channel_SetVolume(BGMChannel, volume);
}

void Audio::stopBGM(){
    if(BGMChannel)
        FMOD_Channel_Stop(BGMChannel);
}

QString Audio::getVersion(){
    return "4.38.06";
}
