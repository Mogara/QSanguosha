#include "TimedProgressBar.h"
#include "clientstruct.h"
#include <QPainter>
#include <SkinBank.h>

void TimedProgressBar::showEvent(QShowEvent* showEvent)
{
    if (!m_hasTimer || m_max <= 0) 
    {
        showEvent->setAccepted(false);
        return;
    }
    m_timer = startTimer(m_step);
    this->setMaximum(m_max);
    this->setValue(m_val);    
}

void TimedProgressBar::hide()
{
    if (m_timer != 0)
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
    m_hasTimer = (ServerInfo.OperationTimeout != 0);
    m_instanceType = S_CLIENT_INSTANCE;
}

void QSanCommandProgressBar::setCountdown(CommandType command)
{
    m_max = ServerInfo.getCommandTimeout(command, m_instanceType);
}

void QSanCommandProgressBar::paintEvent(QPaintEvent *e)
{
    if (this->m_val <= 0) return;
    int width = this->width();
    int height = this->height();
    QPainter painter(this);
    if (orientation() == Qt::Vertical)
    {
        painter.translate(0, height);
        qSwap(width, height); 
        painter.rotate(-90);
    }
    QPixmap progBg = G_ROOM_SKIN.getProgressBarPixmap(0);
    painter.drawPixmap(0, 0, width, height, progBg);
    double percent = 1 - (double) m_val / m_max;
    QPixmap prog = G_ROOM_SKIN.getProgressBarPixmap((int)(percent * 100));
    int drawWidth = percent * prog.width();
    painter.drawPixmap(0, 0, percent * width, height, prog, 0, 0, drawWidth, prog.height());
}

void QSanCommandProgressBar::setCountdown(Countdown countdown)
{
    if (countdown.m_type == Countdown::S_COUNTDOWN_NO_LIMIT)
        m_hasTimer = false;
    m_max = countdown.m_max;
    m_val = countdown.m_current;
}