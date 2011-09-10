#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QThread>

class Joystick : public QThread
{
    Q_OBJECT

public:
    explicit Joystick(QObject *parent = 0);
    static const int Left = 1;
    static const int Right = 2;
    static const int Up = 3;
    static const int Down = 4;

protected:
    virtual void run();

signals:
    void button_clicked(int bit);
    void direction_clicked(int direction);
};

#endif // JOYSTICK_H
