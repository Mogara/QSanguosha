#include "pixmap.h"

#include <QPainter>
#include <QGraphicsColorizeEffect>
#include <QMessageBox>
#include <QImageReader>

Pixmap::Pixmap(const QString &filename, bool center_as_origin)    
{
    load(filename, center_as_origin);
    markable = false;
    marked = false;
    _m_width = pixmap.width();
    _m_height = pixmap.height();
}

bool Pixmap::load(const QString& filename, bool center_as_origin)
{
    return _load(filename, QSize(), false, center_as_origin);
}

bool Pixmap::load(const QString &filename, QSize size, bool center_as_origin)
{
    return _load(filename, size, true, center_as_origin);
}

bool Pixmap::_load(const QString &filename, QSize size, bool useNewSize, bool center_as_origin)
{    
    bool success = pixmap.load(filename);

    if (!success){
        QImageReader reader(filename);
        QString error_string = reader.errorString();

        QString warning = tr("Can not load image %1[%2], error string is %3")
                          .arg(filename).arg(metaObject()->className()).arg(error_string);        
        QMessageBox::warning(NULL, tr("Warning"), warning);        
    } else {
        if (useNewSize)
        {
            _m_width = size.width();
            _m_height = size.height();
        }
        else
        {
            _m_width = pixmap.width();
            _m_height = pixmap.height();
        }
        if(center_as_origin)
        {
            resetTransform();
            this->translate(-_m_width / 2, -_m_height / 2);
        }
        else
            this->prepareGeometryChange();
    }
    return success; 
}

void Pixmap::setPixmap(const QPixmap &pixmap){
    this->pixmap = pixmap;
    prepareGeometryChange();
}

Pixmap::Pixmap(bool center_as_origin)
    :markable(false), marked(false)
{
    if(center_as_origin)
    {
        resetTransform();
        this->translate(-pixmap.width() / 2, -pixmap.height()/2);
    }
    _m_width = _m_height = 0;
}

QRectF Pixmap::boundingRect() const{
    return QRectF(0, 0, _m_width, _m_height);
}


void Pixmap::MakeGray(QPixmap &pixmap){
    QImage img = pixmap.toImage();

    for(int i = 0; i < img.width(); i++){
        for(int j = 0; j < img.height(); j++){
            QRgb color = img.pixel(i, j);
            int gray = qGray(color);
            color = qRgba(gray, gray, gray, qAlpha(color));
            img.setPixel(i, j, color);
        }
    }

    pixmap = QPixmap::fromImage(img);
}

void Pixmap::makeGray(){
    MakeGray(pixmap);
}

void Pixmap::scaleSmoothly(qreal ratio){
    _m_width *= ratio;
    _m_height *= ratio;
    pixmap = pixmap.scaled(_m_width, _m_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    prepareGeometryChange();
}

void Pixmap::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->drawPixmap(QRect(0, 0, _m_width, _m_height), pixmap);
}

QVariant Pixmap::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change == ItemSelectedHasChanged){
        if(value.toBool()){
            QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect(this);
            effect->setColor(QColor(0xCC, 0x00, 0x00));
            setGraphicsEffect(effect);
        }else
            setGraphicsEffect(NULL);

        emit selected_changed();
    }else if(change == ItemEnabledHasChanged){
        emit enable_changed();
    }

    return QGraphicsObject::itemChange(change, value);
}

bool Pixmap::isMarked() const{
    return markable && marked;
}

bool Pixmap::isMarkable() const{
    return markable;
}

void Pixmap::mark(bool marked){
    if(markable){
        if(this->marked != marked){
            this->marked = marked;
            emit mark_changed();
        }
    }
}

void Pixmap::setMarkable(bool markable){
    this->markable = markable;
}

