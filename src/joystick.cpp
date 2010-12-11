#include "joystick.h"

#include "js.h"

Joystick::Joystick(QObject *parent)
    :QThread(parent)
{
    jsInit();
}

void Joystick::run(){
    jsJoystick *joystick = new jsJoystick();

    if(joystick->notWorking())
        QThread::exit(1);

    int num_axes = joystick->getNumAxes();
    float *axes = new float[num_axes];
    memset(axes, 0, sizeof(float) * num_axes);
    int buttons = 0;
    int now_buttons = 0;
    int axe1 = 0;
    int axe2 = 0;

    while(!joystick->notWorking()){
        joystick->read(&now_buttons, axes);

        if(now_buttons != buttons){
            int result = now_buttons ^ buttons;
            buttons = now_buttons;

            if(!(result & now_buttons)){
                int i;
                for(i=0; result; i++)
                    result >>= 1;

                emit button_clicked(i);
            }
        }

        int new_axe1 = int(axes[0]*100)/100;
        int new_axe2 = int(axes[1]*100)/100;

        if(axe1 != new_axe1){
            axe1 = new_axe1;

            if(axe1 < 0)
                emit direction_clicked(Left);
            else if(axe1 > 0)
                emit direction_clicked(Right);
        }

        if(axe2 != new_axe2){
            axe2 = new_axe2;

            if(axe2 < 0)
                emit direction_clicked(Up);
            else if(axe2 > 0)
                emit direction_clicked(Down);
        }

        msleep(100);
    }

    delete axes;
}
