#ifndef DAQIAO_H
#define DAQIAO_H

#include "pixmap.h"

class Daqiao : public Pixmap{
    Q_OBJECT

public:
    Daqiao();

public slots:
    void setContent(const QString &content);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QString content;
};

#endif // DAQIAO_H
