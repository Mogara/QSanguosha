#ifndef _UI_UTILS_H
#define _UI_UTILS_H
#include <QImage>
#include <QColor>
#include <qrect.h>

namespace QSanUiUtils
{
	// This is in no way a generic diation fuction. It is some dirty trick that
	// produces a shadow image for a pixmap whose foreground mask is binaryImage
	QImage produceShadow(const QImage &image, QColor shadowColor, int radius, double decade);
}

#endif