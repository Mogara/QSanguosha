#include <magatamasItem.h>
#include "SkinBank.h"

void MagatamasBoxItem::setOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;
    _updateLayout();
}

void MagatamasBoxItem::_updateLayout()
{
    int xStep, yStep;
    if (this->m_orientation == Qt::Horizontal)
    {
        xStep = m_iconSize.width(); yStep = 0;
    }
    else
    {
        xStep = 0; yStep = m_iconSize.height();
    }

    for (int i = 0; i < 6; i++)
    {
        _icons[i] = G_ROOM_SKIN.getPixmap(QString(QSanRoomSkin::S_SKIN_KEY_MAGATAMAS).arg(i)).
            scaled(m_iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    for (int i = 1; i < 6; i++)
    {
        QSize bgSize;
        if (this->m_orientation == Qt::Horizontal)
        {
            bgSize.setWidth((xStep + 1) * i); 
            bgSize.setHeight(m_iconSize.height());
        }
        else
        {
            bgSize.setWidth((yStep + 1) * i);
            bgSize.setHeight(m_iconSize.width());
        }
        _bgImages[i] = G_ROOM_SKIN.getPixmap(QString(QSanRoomSkin::S_SKIN_KEY_MAGATAMAS_BG).arg(i)).
            scaled(bgSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
}

void MagatamasBoxItem::setIconSize(QSize size)
{
    m_iconSize = size;
    _updateLayout();
}

QRectF MagatamasBoxItem::boundingRect() const
{
    int buckets = qMin(m_maxHp, 5) + G_COMMON_LAYOUT.m_hpExtraSpaceHolder;    
    if (m_orientation == Qt::Horizontal)
        return QRectF(0, 0, buckets * m_iconSize.width(), m_iconSize.height());
    else
        return QRectF(0, 0, m_iconSize.width(), buckets * m_iconSize.height());
}

void MagatamasBoxItem::setHp(int hp)
{
    m_hp = hp;
    _doHpChangeAnimation(hp);
}

void MagatamasBoxItem::setAnchor(QPoint anchor, Qt::Alignment align)
{
    m_anchor = anchor;
    m_align = align;
}

void MagatamasBoxItem::setMaxHp(int maxHp) 
{
    m_maxHp = maxHp;
    _autoAdjustPos();
}

void MagatamasBoxItem::_autoAdjustPos()
{
    if (!anchorEnabled) return;
    QRectF rect = boundingRect();
    Qt::Alignment hAlign = m_align & Qt::AlignHorizontal_Mask;
    if (hAlign == Qt::AlignRight)
        setX(m_anchor.x() - rect.width());
    else if (hAlign == Qt::AlignHCenter)
        setX(m_anchor.x() - rect.width() / 2);
    else
        setX(m_anchor.x());
    Qt::Alignment vAlign = m_align & Qt::AlignVertical_Mask;
    if (vAlign == Qt::AlignBottom)
        setY(m_anchor.y() - rect.height());
    else if (vAlign == Qt::AlignVCenter)
        setY(m_anchor.y() - rect.height() / 2);
    else
        setY(m_anchor.y());
}

void MagatamasBoxItem::_doHpChangeAnimation(int newHp)
{
    if (newHp == m_hp) return;
    // @todo: fix this
    /*
        i+=player->getHp();
        QSanSelectableItem *aniMaga = new QSanSelectableItem(QString("image/system/magatamas/small-%1.png").arg(index));
        addItem(aniMaga);

        QPoint pos = i>=5 ? QPoint(42,69):QPoint(26,86);
        pos.rx() += (i % 5)*16;
        pos.rx() += photo->scenePos().x();
        pos.ry() += photo->scenePos().y();
        aniMaga->setPos(pos);

        QPropertyAnimation *fade = new QPropertyAnimation(aniMaga,"opacity");
        fade->setEndValue(0);
        QPropertyAnimation *grow = new QPropertyAnimation(aniMaga,"scale");
        grow->setEndValue(4);

        connect(fade,SIGNAL(finished()),aniMaga,SLOT(deleteLater()));

        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        group->addAnimation(fade);
        group->addAnimation(grow);

        group->start(QAbstractAnimation::DeleteWhenStopped);

        aniMaga->show();

        i-=player->getHp();
        */
}

void MagatamasBoxItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (m_maxHp <= 0) return;
    int imageIndex = qBound(0, m_hp, 5);
    if (m_hp == m_maxHp) imageIndex = 5;

    int xStep, yStep;
    if (this->m_orientation == Qt::Horizontal)
    {
        xStep = m_iconSize.width(); yStep = 0;
    }
    else
    {
        xStep = 0; yStep = m_iconSize.height();
    }
    
    if (m_showBackground)
    {        
        if (this->m_orientation == Qt::Vertical)
        {
            painter->save();
            painter->translate(m_iconSize.width(), 0);
            painter->rotate(90);
        }
        painter->drawPixmap(0, 0, _bgImages[qMin(m_maxHp, 5)]);
        if (this->m_orientation == Qt::Vertical)
        {
            painter->restore();
        }        
    }

    if (m_maxHp <= 5)
    {
        int i;        
        for (i = 0; i < m_hp; i++)
            painter->drawPixmap(QRect(xStep * i, yStep * i, m_iconSize.width(), m_iconSize.height()),
                                _icons[imageIndex]);
        for (; i < m_maxHp; i++)
            painter->drawPixmap(QRect(xStep * i, yStep * i, m_iconSize.width(), m_iconSize.height()),
                                _icons[0]);
    }
    else
    {
        painter->drawPixmap(QRect(0, 0, m_iconSize.width(), m_iconSize.height()), _icons[imageIndex]);
        G_COMMON_LAYOUT.m_hpFont[imageIndex].paintText(
            painter, QRect(xStep * 1.5, yStep * 1.5, m_iconSize.width(),
            m_iconSize.height()), Qt::AlignCenter, QString::number(m_hp));
        G_COMMON_LAYOUT.m_hpFont[imageIndex].paintText(
            painter, QRect(xStep * 2.5, yStep * 2.5, m_iconSize.width(),
            m_iconSize.height()), Qt::AlignCenter, "/");
        G_COMMON_LAYOUT.m_hpFont[imageIndex].paintText(
            painter, QRect(xStep * 3.5, yStep * 3.5, m_iconSize.width(),
            m_iconSize.height()), Qt::AlignCenter, QString::number(m_maxHp));
    }
}