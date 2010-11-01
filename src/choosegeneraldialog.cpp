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
#include <QPushButton>
#include <QRadioButton>

static QSize GeneralSize(200 * 0.8, 290 * 0.8);

ChooseGeneralDialog::ChooseGeneralDialog(const QList<const General *> &generals, QWidget *parent)
    :QDialog(parent), free_chooser(NULL)
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
        if(Self->getRoleEnum() == Player::Lord && general->objectName() == "shencaocao"){
            button->setEnabled(false);
        }
    }

    QLayout *layout = NULL;
    const int columns = generals.length() > 10 ? 6 : 5;
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

    connect(mapper, SIGNAL(mapped(QString)), ClientInstance, SLOT(chooseItem(QString)));

    QVBoxLayout *dialog_layout = new QVBoxLayout;
    dialog_layout->addLayout(layout);

    // role prompt
    QLabel *role_label = new QLabel(tr("Your role is %1").arg(Sanguosha->translate(Self->getRole())));
    dialog_layout->addWidget(role_label);

    // progress bar & free choose button
    QHBoxLayout *last_layout = new QHBoxLayout;
    if(ServerInfo.OperationTimeout == 0){
        progress_bar = NULL;
    }else{
        progress_bar = new QProgressBar;
        progress_bar->setMinimum(0);
        progress_bar->setMaximum(100);
        last_layout->addWidget(progress_bar);
    }

    if(Config.FreeChoose){
        QPushButton *free_choose_button = new QPushButton(tr("Free choose ..."));
        connect(free_choose_button, SIGNAL(clicked()), this, SLOT(freeChoose()));
        last_layout->addWidget(free_choose_button);
    }

    last_layout->addStretch();

    if(last_layout->count() != 0){
        dialog_layout->addLayout(last_layout);
    }

    setLayout(dialog_layout);
}

void ChooseGeneralDialog::start(){
    if(ServerInfo.OperationTimeout != 0)
        startTimer(200);

    exec();
}

void ChooseGeneralDialog::freeChoose(){
    FreeChooseDialog *dialog = new FreeChooseDialog(this);
    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));

    free_chooser = dialog;

    dialog->exec();
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
        if(isVisible()){
            reject();
            if(free_chooser)
                free_chooser->reject();
        }
    }else
        progress_bar->setValue(new_value);
}

FreeChooseDialog::FreeChooseDialog(ChooseGeneralDialog *parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Free choose generals"));

    QHBoxLayout *box_layout = new QHBoxLayout;
    group = new QButtonGroup(this);

    QList<const General *> all_generals = Sanguosha->findChildren<const General *>();
    QMap<QString, QList<const General*> > map;
    foreach(const General *general, all_generals){
        map[general->getKingdom()] << general;
    }

    QStringList kingdoms = Sanguosha->getKingdoms();

    foreach(QString kingdom, kingdoms){
        QList<const General *> generals = map[kingdom];

        if(!generals.isEmpty()){
            QGroupBox *box = createGroupBox(generals);
            box_layout->addWidget(box);
        }
    }

    QPushButton *ok_button = new QPushButton(tr("OK"));
    connect(ok_button, SIGNAL(clicked()), this, SLOT(chooseGeneral()));

    QPushButton *cancel_button = new QPushButton(tr("Cancel"));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *button_layout = new QHBoxLayout;
    button_layout->addStretch();
    button_layout->addWidget(ok_button);
    button_layout->addWidget(cancel_button);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(box_layout);
    layout->addLayout(button_layout);

    setLayout(layout);

    group->buttons().first()->click();
}

void FreeChooseDialog::chooseGeneral(){
    QAbstractButton *button = group->checkedButton();
    if(button){
        ClientInstance->chooseItem(button->objectName());
        accept();
    }
}

QGroupBox *FreeChooseDialog::createGroupBox(const QList<const General *> &generals){
    QGroupBox *box = new QGroupBox;
    QString kingdom = generals.first()->getKingdom();
    box->setTitle(Sanguosha->translate(kingdom));

    QVBoxLayout *layout = new QVBoxLayout;
    QIcon lord_icon(":/roles/lord.png");

    foreach(const General *general, generals){
        QString general_name = general->objectName();

        QString text = QString("%1[%2]")
                       .arg(Sanguosha->translate(general_name))
                       .arg(Sanguosha->translate(general->getPackage()));

        QRadioButton *button = new QRadioButton(text);
        button->setObjectName(general_name);
        button->setToolTip(general->getSkillDescription());
        if(general->isLord())
            button->setIcon(lord_icon);

        group->addButton(button);
        layout->addWidget(button);
    }

    layout->addStretch();
    box->setLayout(layout);

    return box;
}
