#ifndef OPTIONBUTTON_H
#define OPTIONBUTTON_H

#include <QToolButton>

class OptionButton : public QToolButton
{
    Q_OBJECT
public:
    explicit OptionButton(const QString icon_path, const QString &caption = "", QWidget *parent = 0);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *);

signals:
    void double_clicked();

};

#endif // OPTIONBUTTON_H
