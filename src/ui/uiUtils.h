#ifndef _UI_UTILS_H
#define _UI_UTILS_H
#include <QImage>
#include <QColor>
#include <qrect.h>
#include <qpainter.h>

namespace QSanUiUtils
{
    // This is in no way a generic diation fuction. It is some dirty trick that
    // produces a shadow image for a pixmap whose foreground mask is binaryImage
    QImage produceShadow(const QImage &image, QColor shadowColor, int radius, double decade);
    void makeGray(QPixmap &pixmap);

    namespace QSanFreeTypeFont
    {
        int* loadFont(const QString &fontPath);
        QString resolveFont(const QString &fontName);
        // @param painter
        //        Device to be painted on
        // @param font
        //        Pointer returned by loadFont used to index a font
        // @param text
        //        Text to be painted
        // @param fontSize [IN, OUT]
        //        Suggested width and height of each character in pixels. If the
        //        bounding box cannot contain the text using the suggested font
        //        size, font size may be shrinked. The output value will be the
        //        actual font size used.
        // @param boundingBox
        //        Text will be painted in the center of the bounding box on the device
        // @param orient
        //        Suggest whether the text is laid out horizontally or vertically.
        // @return True if succeed. 
        bool paintQString(QPainter* painter, QString text,
                          int* font, QColor color,
                          QSize& fontSize, int spacing, QRect boundingBox,
                          Qt::Orientation orient, Qt::Alignment align);
        
        // Currently, we online support horizotal layout for multiline text
        bool paintQStringMultiLine(QPainter* painter, QString text,
                                   int* font, QColor color,
                                   QSize& fontSize, int spacing, QRect boundingBox,
                                   Qt::Alignment align);
    }

}

#endif