#include "uiUtils.h"
#include <qpixmap.h>
#include <qimage.h>
#include <qfile.h>
#include <qdir.h>
#include <qdesktopservices.h>
#include <qmutex.h>

QImage QSanUiUtils::produceShadow(const QImage &image, QColor shadowColor, int radius, double decade)
{
    const uchar* oldImage = image.bits();
    int cols = image.width();
    int rows = image.height();

    int alpha = shadowColor.alpha();
        
    uchar* newImage = new uchar[cols * rows * 4];
    
#define _NEW_PIXEL_CHANNEL(x, y, channel) (newImage[(y * cols + x) * 4 + channel])
#define _NEW_PIXEL(x, y) _NEW_PIXEL_CHANNEL(x, y, 3)
#define _OLD_PIXEL(x, y) (oldImage[(y * cols + x) * 4 + 3])

    for (int y = 0; y < rows; y++)
    {		
        for (int x = 0; x < cols; x++)
        {
            _NEW_PIXEL_CHANNEL(x, y, 0) = shadowColor.blue();
            _NEW_PIXEL_CHANNEL(x, y, 1) = shadowColor.green();
            _NEW_PIXEL_CHANNEL(x, y, 2) = shadowColor.red();
            _NEW_PIXEL_CHANNEL(x, y, 3) = 0;
        }
    }

    for (int y = 0; y < rows; y++)
    {		
        for (int x = 0; x < cols; x++)
        {
            uchar oldVal = _OLD_PIXEL(x, y);
            if (oldVal == 0) continue;
            for (int dy = -radius; dy <= radius; dy++)
            {
                for (int dx = -radius; dx <= radius; dx++)
                {
                    int wx = x + dx;
                    int wy = y + dy;
                    int dist = dx * dx + dy * dy;
                    if (wx < 0 || wy < 0 || wx >= cols || wy >= rows) continue;
                    if (dx * dx + dy * dy > radius * radius) continue;
                    int newVal = alpha - decade * dist;
                    Q_ASSERT((wy * cols + wx) * 4 < cols * rows * 4);
                    _NEW_PIXEL(wx, wy) = (uchar)qMax((int)_NEW_PIXEL(wx, wy), newVal); 
                }
            }
        }
    }
#undef _NEW_PIXEL_CHANNEL
#undef _NEW_PIXEL
#undef _OLD_PIXEL
    QImage result(newImage, cols, rows, QImage::Format_ARGB32);
    return result;
}

void QSanUiUtils::makeGray(QPixmap &pixmap)
{
    QImage img = pixmap.toImage();

    for(int i = 0; i < img.width(); i++){
        for(int j = 0; j < img.height(); j++){
            QRgb color = img.pixel(i, j);
            int gray = qGray(color);
            img.setPixel(i, j, qRgb(gray, gray, gray));
        }
    }

    pixmap = QPixmap::fromImage(img);
}

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include FT_OUTLINE_H

static FT_Library  _ftlib;
static bool _ftLibInitialized = false;

static bool _initLibrary()
{    
    FT_Error error = FT_Init_FreeType(&_ftlib);
    if (error) {
        qWarning("error loading library");
        return false;
    } else {
        _ftLibInitialized = true;
        return true;
    }
}

QString QSanUiUtils::QSanFreeTypeFont::resolveFont(const QString& fontName)
{
    QString result;
    if (QFile::exists(fontName))
        result = fontName;
    else
    {
        QStringList dirsToResolve;
        QStringList extsToTry;
        QString sysfolder = QDesktopServices::storageLocation(QDesktopServices::FontsLocation);
        dirsToResolve.push_back(sysfolder);
        dirsToResolve.push_back(QDir::currentPath());
        dirsToResolve.push_back("./font");
        extsToTry.push_back("ttf");
        extsToTry.push_back("ttc");
        foreach (QString sdir, dirsToResolve)
        {
            foreach (QString ext, extsToTry)
            {
                QDir dir(sdir);
                QString filePath = dir.filePath(QString("%1.%2").arg(fontName).arg(ext));
                if (QFile::exists(filePath))
                {
                    result = filePath;
                    break;
                }
            }
        }
    }
    return result;
}

int* QSanUiUtils::QSanFreeTypeFont::loadFont(const QString& fontName)
{
    if (!_ftLibInitialized)
        if (!_initLibrary())
            return NULL;
    FT_Face face = NULL;
    QString resolvedPath = resolveFont(fontName);
    QByteArray arr = resolvedPath.toAscii();
    const char* fontPath = arr.constData();
    FT_Error error = FT_New_Face(_ftlib, fontPath, 0, &face);
    if (error == FT_Err_Unknown_File_Format)
    {
        qWarning("Unsupported font format: %s.", fontPath);
    }
    else if (error)
    {
        qWarning("Cannot open font file: %s.", fontPath);    
    }
    else return (int*)face;
    return 0; 
}

static QMutex _paintTextMutex;

bool QSanUiUtils::QSanFreeTypeFont::paintQString(
    QPainter* painter, QString text, int* font, QColor color,
    QSize& fontSize, int spacing, int weight, QRect boundingBox,
    Qt::Orientation orient, Qt::Alignment align)
{
    if (!_ftLibInitialized || font == NULL || painter == NULL)
        return false;

    QVector<uint> charcodes = text.toUcs4();
    int len = charcodes.size();
    int pixelsAdded = (weight >> 6) * 2;
    int xstep, ystep;
    Qt::Alignment hAlign = align & Qt::AlignHorizontal_Mask;
    Qt::Alignment vAlign = align & Qt::AlignVertical_Mask;
    if (hAlign == 0) hAlign = Qt::AlignHCenter;
    if (vAlign == 0) vAlign = Qt::AlignVCenter;

    QPoint topLeft = boundingBox.topLeft();
    boundingBox.moveTopLeft(QPoint(0, 0));

    if (orient == Qt::Vertical)
    {
        xstep = 0;
        if (fontSize.width() > boundingBox.width())
            fontSize.setWidth(boundingBox.width());
        ystep = spacing + fontSize.height();
        // AlignJustify means the text should fill out the whole rect space
        // so we increase the step
        if (align & Qt::AlignJustify) {
            ystep = boundingBox.height() / len;
            if (fontSize.height() + spacing > ystep)
                fontSize.setHeight(ystep - spacing);
        }
    }
    else
    {        
        ystep = 0;
        if (fontSize.height() > boundingBox.height())
            fontSize.setHeight(boundingBox.height());
        xstep = spacing + fontSize.width();
        // AlignJustifx means the text should fill out the whole rect space
        // so we increase the step
        if (align & Qt::AlignJustify) {
            xstep = boundingBox.width() / len;
            if (fontSize.width() + spacing > xstep)
                fontSize.setWidth(xstep - spacing);
        }
    }
    if (fontSize.width() <= 0 || fontSize.height() <= 0) return false;
    // we allocate larger area than necessary in case we need bold font
    int rows = boundingBox.height() + pixelsAdded + 3;
    int cols = boundingBox.width() + pixelsAdded + 3;
    int imageSize = rows * cols;
    int imageBytes = imageSize * 4;
    uchar* newImage = new uchar[imageBytes];

    for (int i = 0; i < imageBytes;)
    {
        newImage[i++] = color.blue();
        newImage[i++] = color.green();
        newImage[i++] = color.red();
        newImage[i++] = 0;
    }

#if (defined(_NEW_PIXEL) || defined(_FONT_PIXEL))
#error("macro _NEW_PIXEL or _FONT_PIXEL already in use")
#endif

#define _NEW_PIXEL(x, y, channel) (newImage[((y) * cols + (x)) * 4 + channel])
#define _FONT_PIXEL(x, y) (bitmap.buffer[(y) * rowStep + (x)])
    // we do not do kerning for vertical layout for now
    bool useKerning = ((orient == Qt::Horizontal) && !(align & Qt::AlignJustify));
    
    _paintTextMutex.lock();
    FT_Face face = (FT_Face)font;
    FT_GlyphSlot slot = face->glyph;
    FT_Error error;
    error = FT_Set_Pixel_Sizes(face, fontSize.width(), fontSize.height());
    FT_UInt previous = 0;
    int currentX = 0;
    int currentY = 0;
    for (int i = 0; i < len; i++)
    {
        FT_Vector  delta;
       
        FT_UInt glyph_index = FT_Get_Char_Index(face, charcodes[i]);
        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP); 
        if (error) continue;
        
        if (useKerning && previous && glyph_index)
        {
            error = FT_Get_Kerning(face, previous, glyph_index,
                           FT_KERNING_DEFAULT, &delta);
            currentX += delta.x >> 6;
        }
        previous = glyph_index;
        
        
        FT_Bitmap bitmap;
        if (weight == 0) {
            FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER); 
            // FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
        } else {
            FT_Outline_Embolden(&slot->outline, weight);
            FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
        }

        bitmap = slot->bitmap;
        Q_ASSERT(bitmap.pitch == bitmap.width ||
            bitmap.pitch == (bitmap.width - 1) / 8 + 1);
        bool mono = true;
        if (bitmap.pitch == bitmap.width) 
            mono = false;
        
        int fontRows = bitmap.rows;
        int fontCols = bitmap.width;
        int rowStep = bitmap.pitch;
        int tmpYOffset = fontSize.height() - slot->bitmap_top;
        currentY = currentY + tmpYOffset;

        if (orient == Qt::Vertical)
        {
            currentX = (fontSize.width() - bitmap.width) / 2;
        }

        // now paint the bitmap to the new region;
        if (currentX < 0) currentX = 0;
        if (currentY < 0) currentY = 0;
        for (int y = 0; y < fontRows; y++)
        {
            if (currentY + y >= rows)
                break;
            uchar* fontPtr = &_FONT_PIXEL(0, y);
            uchar* imagePtr = &_NEW_PIXEL(currentX, currentY + y, 3);
            int fontClippedCols;
            if (fontCols + currentX < cols)
                fontClippedCols = fontCols;
            else
                fontClippedCols = cols - 1 - currentX;
            if (!mono)
            {
                for (int x = 0; x < fontClippedCols; x++)
                {
                    *imagePtr = *fontPtr;
                    fontPtr++;
                    imagePtr += 4;
                }
            }
            else
            {
                int mask = 0x80;
                for (int x = 0; x < fontClippedCols; x++)
                {
                    if (*fontPtr & mask)
                        *imagePtr = 255;
                    mask = mask >> 1;
                    if (mask == 0)
                    {
                        fontPtr++;
                        mask = 0x80;
                    }
                    imagePtr += 4;
                }
            }
        }
        if (useKerning)        
            currentX += (slot->advance.x >> 6) + spacing;
        else
            currentX += xstep;
        currentY = currentY - tmpYOffset + ystep;
        /*if (!mono && !(weight == 0))
        {
            FT_Bitmap_Done(_ftlib, &bitmap);
        }*/
    }
#undef _NEW_PIXEL
#undef _FONT_PIXEL
    _paintTextMutex.unlock();

    int xstart, ystart;
    if (orient == Qt::Vertical)
    {
        if (hAlign & Qt::AlignLeft)
            xstart = spacing;
        else if (hAlign & Qt::AlignHCenter)
            xstart = (boundingBox.width() - fontSize.width()) / 2;
        else if (hAlign & Qt::AlignRight)
            xstart = boundingBox.right() - spacing - fontSize.width();
        
        if (vAlign & Qt::AlignTop) {            
            ystart = spacing;
        } else if (vAlign & Qt::AlignVCenter || align & Qt::AlignJustify) {
            ystart = (boundingBox.height() - currentY) / 2;
        } else if (vAlign & Qt::AlignBottom) {
            ystart = boundingBox.height() - currentY - spacing;
        }
    }
    else
    {
        if (vAlign & Qt::AlignTop)
            ystart = spacing;
        else if (vAlign & Qt::AlignVCenter)
            ystart = (boundingBox.height() - fontSize.height()) / 2;
        else if (vAlign & Qt::AlignBottom)
            ystart = boundingBox.bottom() - spacing - fontSize.height();

        if (hAlign & Qt::AlignLeft) {            
            xstart = spacing;
        } else if (hAlign & Qt::AlignHCenter || align & Qt::AlignJustify) {
            xstart = (boundingBox.width() - currentX) / 2;
        } else if (hAlign & Qt::AlignRight) {
            xstart = boundingBox.right() - currentX - spacing;
        }
    }
    if (xstart < 0) xstart = 0;
    if (ystart < 0) ystart = 0;
    QImage result(newImage, cols, rows, QImage::Format_ARGB32);
    painter->drawImage(topLeft.x() + xstart, topLeft.y() + ystart, result);
    return true;
}

bool QSanUiUtils::QSanFreeTypeFont::paintQStringMultiLine(
    QPainter* painter, QString text, int* font, QColor color,
    QSize& fontSize, int spacing, QRect boundingBox,
    Qt::Alignment align)
{
    if (!_ftLibInitialized || font == NULL || painter == NULL)
        return false;
    
    QVector<uint> charcodes = text.toUcs4();
    int len = charcodes.size();
    int charsPerLine = boundingBox.width() / fontSize.width();
    int numLines = (len - 1) / charsPerLine + 1;
    QPoint topLeft = boundingBox.topLeft();
    boundingBox.moveTopLeft(QPoint(0, 0));
    int xstep;
    if (align & Qt::AlignJustify) 
        xstep = boundingBox.width() / len;
    else
        xstep = spacing + fontSize.width();
    if (fontSize.height() * numLines > boundingBox.height())
        fontSize.setHeight(boundingBox.height() / numLines - spacing);

    int ystep = fontSize.height() + spacing;
    
    if (fontSize.width() <= 0 || fontSize.height() <= 0) return false;
    // AlignJustifx means the text should fill out the whole rect space
    // so we increase the step
    

    int rows = boundingBox.height();
    int cols = boundingBox.width();
    int imageSize = boundingBox.width() * boundingBox.height();
    int imageBytes = imageSize * 4;
    uchar* newImage = new uchar[imageBytes];

    for (int i = 0; i < imageBytes;)
    {
        newImage[i++] = color.blue();
        newImage[i++] = color.green();
        newImage[i++] = color.red();
        newImage[i++] = 0;
    }


    // we do not do kerning for vertical layout for now
    bool useKerning = (!(align & Qt::AlignJustify));
    FT_UInt previous = 0;
    int currentX = 0;
    int currentY = 0;
    int maxX = 0;
    int maxY = 0;
    FT_Face face = (FT_Face)font;

    _paintTextMutex.lock();    
    FT_GlyphSlot slot = face->glyph;
    FT_Error error = FT_Set_Pixel_Sizes(face, fontSize.width(), fontSize.height());    
    for (int i = 0; i < len; i++)
    {
        int line = i / charsPerLine;
        int cursor = i % charsPerLine;
        // whenever we start a new line, reset X and increment Y
        if (cursor == 0 && line > 0)
        {
            currentY += ystep;
            currentX = 0;
        }

        FT_Vector  delta;       
        FT_UInt glyph_index = FT_Get_Char_Index(face, charcodes[i]);
        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER); 
        if (error) continue;

        
        if (useKerning && previous && glyph_index)
        {
            error = FT_Get_Kerning(face, previous, glyph_index,
                           FT_KERNING_DEFAULT, &delta);
            currentX += delta.x >> 6;
        }
        previous = glyph_index;

        FT_Bitmap bitmap = slot->bitmap;
        int fontRows = bitmap.rows;
        int fontCols = bitmap.width;
        int rowStep = bitmap.pitch;
        int tmpYOffset = fontSize.height() - slot->bitmap_top;
        currentY = currentY + tmpYOffset;

        Q_ASSERT(bitmap.pitch == bitmap.width ||
                 bitmap.pitch == (bitmap.width - 1) / 8 + 1);
        //@todo putitback
        bool mono = true;
        if (bitmap.pitch == bitmap.width) 
            mono = false;
        // now paint the bitmap to the new region;
        Q_ASSERT(currentX >= 0 && currentY >= 0);
        for (int y = 0; y < fontRows; y++)
        {
            if (currentY + y >= rows)
                break;
#if (defined(_NEW_PIXEL) || defined(_FONT_PIXEL))
#error("macro _NEW_PIXEL or _FONT_PIXEL already in use")
#endif
#define _NEW_PIXEL(x, y, channel) (newImage[((y) * cols + (x)) * 4 + channel])
#define _FONT_PIXEL(x, y) (bitmap.buffer[(y) * rowStep + (x)])
            uchar* fontPtr = &_FONT_PIXEL(0, y);
            uchar* imagePtr = &_NEW_PIXEL(currentX, currentY + y, 3);
#undef _NEW_PIXEL
#undef _FONT_PIXEL
            int fontClippedCols;
            if (fontCols + currentX < cols)
                fontClippedCols = fontCols;
            else
                fontClippedCols = cols - 1 - currentX;
            if (!mono)
            {
                for (int x = 0; x < fontClippedCols; x++)
                {
                    *imagePtr = *fontPtr;
                    fontPtr++;
                    imagePtr += 4;
                }
            }
            else
            {
                int mask = 0x80;
                for (int x = 0; x < fontClippedCols; x++)
                {
                    if (*fontPtr & mask)
                        *imagePtr = 255;
                    mask = mask >> 1;
                    if (mask == 0)
                    {
                        fontPtr++;
                        mask = 0x80;
                    }
                    imagePtr += 4;
                }
            }
        }
        if (useKerning)        
            currentX += slot->advance.x >> 6;
        else
            currentX += xstep;
        if (currentX > maxX) maxX = currentX;
        currentY -= tmpYOffset;
    }
    _paintTextMutex.unlock();
    maxY = currentY + ystep;
    Qt::Alignment hAlign = align & Qt::AlignHorizontal_Mask;
    Qt::Alignment vAlign = align & Qt::AlignVertical_Mask;
    int xstart, ystart;
    if (hAlign & Qt::AlignLeft) {
        xstart = spacing;
    } else if (hAlign & Qt::AlignHCenter || align & Qt::AlignJustify) {
        xstart = (boundingBox.width() - maxX) / 2;
    } else if (hAlign & Qt::AlignRight) {
        xstart = boundingBox.right() - maxX - spacing;
    }
    if (vAlign & Qt::AlignTop) {
        ystart = spacing;
    } else if (vAlign & Qt::AlignVCenter) {
        ystart = (boundingBox.height() - maxY) / 2;
    } else if (vAlign & Qt::AlignBottom) {
        ystart = boundingBox.height() - maxY - spacing;
    }
    if (xstart < 0) xstart = 0;
    if (ystart < 0) ystart = 0;
    QImage result(newImage, cols, rows, QImage::Format_ARGB32);
    painter->drawImage(topLeft.x() + xstart, topLeft.y() + ystart, result);
    return true;
}