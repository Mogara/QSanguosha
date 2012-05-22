#ifndef _TIMED_PROGRESS_BAR_H
#define _TIMED_PROGRESS_BAR_H
#include <QProgressBar>
#include <QTimerEvent>
#include <QShowEvent>
#include <QPaintEvent>

class TimedProgressBar : public QProgressBar
{
Q_OBJECT
public:
    inline TimedProgressBar():m_hasTimer(false), m_autoHide(false), m_timer(NULL), m_step(0), m_max(0), m_val(0)
    {
        this->setTextVisible(false);
    }
    inline void setTimerEnabled(bool enabled) { m_hasTimer = enabled; }
    inline void setCountdown(time_t maximum, time_t startVal = 0)
    {
        m_max = maximum;
        m_val = startVal;
    }
    inline void setAutoHide(bool enabled) { m_autoHide = enabled; }
    inline void setUpdateInterval(time_t step) { m_step = step; }
    virtual void showEvent(QShowEvent* showEvent);
    virtual void hide();    
signals:
    void timedOut();
protected:
    virtual void timerEvent(QTimerEvent *);
    bool m_hasTimer;
    bool m_autoHide;
    int m_timer;
    time_t m_step, m_max, m_val;
};

#include "protocol.h"
#include "settings.h"

class QSanCommandProgressBar: public TimedProgressBar
{
Q_OBJECT    
public:
    QSanCommandProgressBar();
    inline void setInstanceType(QSanProtocol::ProcessInstanceType type) { m_instanceType = type; }
    void setCountdown(QSanProtocol::CommandType command);
    void setCountdown(QSanProtocol::Countdown countdown);    
protected:
    virtual void paintEvent(QPaintEvent *);
    QSanProtocol::ProcessInstanceType m_instanceType;
private:
    static QPixmap* sm_progress_100_image;
    static QPixmap* sm_progress_80_image;
    static QPixmap* sm_progress_50_image;
    static QPixmap* sm_progress_20_image;
    static QPixmap* sm_progress_0_image;
    static const int S_PROGRESS_IMAGE_WIDTH = 310;
    static const int S_PROGRESS_IMAGE_HEIGHT = 26;
};

#endif