#include "TimedProgressBar.h"
#include "clientstruct.h"
#include <QPainter>

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
    if (sm_progress_100_image == NULL)
    {
        sm_progress_100_image = new QPixmap("image/system/controls/progress-100.png");
        sm_progress_80_image = new QPixmap("image/system/controls/progress-79.png");
        sm_progress_50_image = new QPixmap("image/system/controls/progress-49.png");
        sm_progress_20_image = new QPixmap("image/system/controls/progress-19.png");
        sm_progress_0_image = new QPixmap("image/system/controls/progress-0.png");
    }
}

void QSanCommandProgressBar::setCountdown(CommandType command)
{
    m_max = ServerInfo.getCommandTimeout(command, m_instanceType);
}

QPixmap* QSanCommandProgressBar::sm_progress_100_image = NULL;
QPixmap* QSanCommandProgressBar::sm_progress_80_image = NULL;
QPixmap* QSanCommandProgressBar::sm_progress_50_image = NULL;
QPixmap* QSanCommandProgressBar::sm_progress_20_image = NULL;
QPixmap* QSanCommandProgressBar::sm_progress_0_image = NULL;

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
    painter.drawPixmap(0, 0, width, height, *sm_progress_0_image,
        0, 0, S_PROGRESS_IMAGE_WIDTH, S_PROGRESS_IMAGE_HEIGHT);
    const QPixmap *image;
    double percent = 1 - (double) m_val / m_max;
    if (percent >= 0.8)
        image = sm_progress_100_image;
    else if (percent >= 0.5)    
        image = sm_progress_80_image;
    else if (percent >= 0.2)    
        image = sm_progress_50_image;
    else 
        image = sm_progress_20_image;
    int drawWidth = percent * S_PROGRESS_IMAGE_WIDTH;
    painter.drawPixmap(0, 0, percent * width, height, *image, 0, 0, drawWidth, S_PROGRESS_IMAGE_HEIGHT);
    
}

void QSanCommandProgressBar::setCountdown(Countdown countdown)
{
    if (countdown.m_type == Countdown::S_COUNTDOWN_NO_LIMIT)
        m_hasTimer = false;
    m_max = countdown.m_max;
    m_val = countdown.m_current;
}