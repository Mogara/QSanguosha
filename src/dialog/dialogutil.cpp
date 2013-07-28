#include "dialogutil.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QDialog>

QHBoxLayout *CreateOKCancelLayout(QDialog *dialog){
    QPushButton *ok_button = new QPushButton(QObject::tr("OK"));
    QPushButton *cancel_button = new QPushButton(QObject::tr("Cancel"));

    QObject::connect(ok_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    QObject::connect(cancel_button, SIGNAL(clicked()), dialog, SLOT(reject()));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();

#ifdef Q_OS_MACX
    // follow Mac OS X standard
    hlayout->addWidget(cancel_button);
    hlayout->addWidget(ok_button);
#else
    hlayout->addWidget(ok_button);
    hlayout->addWidget(cancel_button);
#endif

    return hlayout;
}
