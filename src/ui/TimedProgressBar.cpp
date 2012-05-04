#include "TimedProgressBar.h"

void TimedProgressBar::showEvent(QShowEvent*)
{
    if (!m_hasTimer) return;
    m_timer = startTimer(m_step);
    this->setMaximum(m_max);
    this->setValue(m_val);
    QProgressBar::show();    
}

void TimedProgressBar::hide()
{
    if (m_timer != NULL)
    {        
        killTimer(m_timer);
        m_timer = NULL;
    }
    QProgressBar::hide();
}

void TimedProgressBar::timerEvent(QTimerEvent* timerEvent)
{
    m_val += m_step;
    if(m_val >= m_max){
        m_val = m_max;
        if (m_autoHide) hide();
        else
        {            
            killTimer(m_timer);
            m_timer = NULL;
        }
        emit timedOut();    
    }
    this->setValue(m_val);
}

using namespace QSanProtocol;

QSanCommandProgressBar::QSanCommandProgressBar()
{
    m_step = Config.S_PROGRESS_BAR_UPDATE_INTERVAL;
    m_hasTimer = !Config.OperationNoLimit;
    m_instanceType = S_CLIENT_INSTANCE;
}

void QSanCommandProgressBar::setCountdown(CommandType command)
{
    m_max = Config.getCommandTimeout(command, m_instanceType);
}

void QSanCommandProgressBar::setCountdown(Countdown countdown)
{
    if (countdown.m_type == Countdown::S_COUNTDOWN_NO_LIMIT)
        m_hasTimer = false;
    m_max = countdown.m_max;
    m_val = countdown.m_current;
}