#include "magatamawidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

MagatamaWidget::MagatamaWidget(int hp, Qt::Orientation orientation)
{
    QBoxLayout *layout = NULL;
    if(orientation == Qt::Vertical)
        layout = new QVBoxLayout;
    else
        layout = new QHBoxLayout;

    QPixmap pixmap;
    if(hp>=5)
        pixmap.load(":/magatamas/5.png");
    else
        pixmap.load(QString(":/magatamas/%1.png").arg(hp));

    int i;
    for(i=0; i<hp; i++){
        QLabel *label = new QLabel;
        label->setPixmap(pixmap);

        layout->addWidget(label);
    }

    setLayout(layout);
}
