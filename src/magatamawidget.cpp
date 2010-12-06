#include "magatamawidget.h"
#include "player.h"

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

    QPixmap pixmap = *GetMagatama(qMin(5, hp));

    int i;
    for(i=0; i<hp; i++){
        QLabel *label = new QLabel;
        label->setPixmap(pixmap);

        layout->addWidget(label);
    }

    setLayout(layout);
}

QPixmap *MagatamaWidget::GetMagatama(int index){
    static QPixmap magatamas[6];
    if(magatamas[0].isNull()){
        int i;
        for(i=0; i<=5; i++)
            magatamas[i].load(QString(":/magatamas/%1.png").arg(i));
    }

    return &magatamas[index];
}

QPixmap *MagatamaWidget::GetSmallMagatama(int index){
    static QPixmap magatamas[6];
    if(magatamas[0].isNull()){
        int i;
        for(i=0; i<=5; i++)
            magatamas[i].load(QString(":/magatamas/small-%1.png").arg(i));
    }

    return &magatamas[index];
}
