#ifndef GENERALSEARCHBOX_H
#define GENERALSEARCHBOX_H

#include <QLineEdit>

class GeneralSearchBox : public QLineEdit
{
    Q_OBJECT
public:
    explicit GeneralSearchBox(QWidget *parent = 0);
    
private slots:
    void onReturnPressed();

signals:
    void generalInput(const QString &name);
};

#endif // GENERALSEARCHBOX_H
