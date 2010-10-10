#include "choosegeneraldialog.h"
#include "general.h"
#include "optionbutton.h"
#include "engine.h"
#include "client.h"
#include "settings.h"

#include <QSignalMapper>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QTimerEvent>

static QSize GeneralSize(200 * 0.8, 290 * 0.8);

ChooseGeneralDialog::ChooseGeneralDialog(const QList<const General *> &generals, QWidget *parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Choose general"));

    QSignalMapper *mapper = new QSignalMapper(this);
    QList<OptionButton *> buttons;
    foreach(const General *general, generals){
        QString icon_path = general->getPixmapPath("card");
        QString caption = Sanguosha->translate(general->objectName());
        OptionButton *button = new OptionButton(icon_path, caption);
        button->setToolTip(general->getSkillDescription());
        button->setIconSize(GeneralSize);
        buttons << button;

        mapper->setMapping(button, general->objectName());
        connect(button, SIGNAL(double_clicked()), mapper, SLOT(map()));
        connect(button, SIGNAL(double_clicked()), this, SLOT(accept()));

        // special case
        if(Self->getRole() == "lord" && general->objectName() == "shencaocao"){
            button->setEnabled(false);
        }
    }

    QLayout *layout = NULL;
    const static int columns = 5;
    if(generals.length() <= columns){
        layout = new QHBoxLayout;
        foreach(OptionButton *button, buttons)
            layout->addWidget(button);
    }else{
        QGridLayout *grid_layout = new QGridLayout;
        layout = grid_layout;

        int i;
        for(i=0; i<buttons.length(); i++){
            int row = i / columns;
            int column = i % columns;
            grid_layout->addWidget(buttons.at(i), row, column);
        }
    }

    mapper->setMapping(this, generals.first()->objectName());
    connect(this, SIGNAL(rejected()), mapper, SLOT(map()));

    connect(mapper, SIGNAL(mapped(QString)), ClientInstance, SLOT(itemChosen(QString)));

    QVBoxLayout *dialog_layout = new QVBoxLayout;
    dialog_layout->addLayout(layout);

    // role prompt
    QLabel *role_label = new QLabel(tr("Your role is %1").arg(Sanguosha->translate(Self->getRole())));
    dialog_layout->addWidget(role_label);

    // progress bar
    if(Config.OperationNoLimit){
        progress_bar = NULL;
    }else{
        progress_bar = new QProgressBar;
        progress_bar->setMinimum(0);
        progress_bar->setMaximum(100);
        dialog_layout->addWidget(progress_bar);
    }

#ifndef QT_NO_DEBUG

    QLineEdit *cheat_edit = new QLineEdit;
    cheat_edit->setPlaceholderText(tr("Please input the general's pinyin ..."));
    dialog_layout->addWidget(cheat_edit);

    connect(cheat_edit, SIGNAL(returnPressed()), ClientInstance, SLOT(cheatChoose()));
    connect(cheat_edit, SIGNAL(returnPressed()), this, SLOT(accept()));

#endif

    setLayout(dialog_layout);
}

void ChooseGeneralDialog::start(){
    if(!Config.OperationNoLimit)
        startTimer(200);

    exec();
}

void ChooseGeneralDialog::timerEvent(QTimerEvent *event){
    if(progress_bar == NULL)
        return;

    static const int timeout = 15;

    int step = 100 / double(timeout * 5);
    int new_value = progress_bar->value() + step;
    new_value = qMin(progress_bar->maximum(), new_value);
    progress_bar->setValue(new_value);

    if(new_value >= progress_bar->maximum()){
        killTimer(event->timerId());
        if(isVisible())
            reject();
    }else
        progress_bar->setValue(new_value);
}
