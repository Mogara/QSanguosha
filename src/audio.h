#ifndef AUDIO_H
#define AUDIO_H

class Audio
{
public:
    virtual void play() = 0;
    virtual void stop() = 0;
    static void init();
    static void quit();
};

class AudioSample:public Audio{
public:
    AudioSample();
};

class AudioStream:public Audio{
public:
};

#endif // AUDIO_H
