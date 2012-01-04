#include "choosegeneraldialog.h"
#include "general.h"
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
#include <QCheckBox>
#include <QTabWidget>

OptionButton::OptionButton(QString icon_path, const QString &caption, QWidget *parent)
    :QToolButton(parent)
{
    QPixmap pixmap(icon_path);
    QIcon icon(pixmap);

    setIcon(icon);
    setIconSize(pixmap.size());

    if(!caption.isEmpty()){
        setText(caption);
        setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        if(caption.length()>= 4){
            QFont font = Config.SmallFont;
            font.setPixelSize(Config.SmallFont.pixelSize() - 5);
            setFont(font);
        }else
            setFont(Config.SmallFont);
    }
}

void OptionButton::mouseDoubleClickEvent(QMouseEvent *){
    emit double_clicked();
}


ChooseGeneralDialog::ChooseGeneralDialog(const QStringList &general_names, QWidget *parent)
    :QDialog(parent), free_chooser(NULL)
{
    setWindowTitle(tr("Choose general"));

    QString lord_name;

    QList<const General *> generals;
    foreach(QString general_name, general_names){
        if(general_name.contains("(lord)"))
        {
            general_name.chop(6);
            lord_name = general_name;
            continue;
        }
        const General *general = Sanguosha->getGeneral(general_name);
        generals << general;
    }

    QSignalMapper *mapper = new QSignalMapper(this);
    QList<OptionButton *> buttons;
    QString category("card");
    QSize icon_size(200*0.8, 290*0.8);
    if(generals.length() > 10){
        category = "big";
        icon_size = QSize(94, 96);
    }

    foreach(const General *general, generals){
        QString icon_path = general->getPixmapPath(category);
        QString caption = Sanguosha->translate(general->objectName());
        OptionButton *button = new OptionButton(icon_path, caption);
        button->setToolTip(general->getSkillDescription());
        button->setIconSize(icon_size);
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

        if(lord_name.size())
        {
            const General * lord = Sanguosha->getGeneral(lord_name);

            QLabel *label = new QLabel;
            //label->setCaption(tr("Lord's general"));
            label->setPixmap(lord->getPixmapPath(category));
            label->setToolTip(lord->getSkillDescription());
            layout->addWidget(label);
        }

        foreach(OptionButton *button, buttons)
            layout->addWidget(button);
    }else{
        QGridLayout *grid_layout = new QGridLayout;
        layout = grid_layout;

        if(lord_name.size())
        {
            const General * lord = Sanguosha->getGeneral(lord_name);

            QLabel *label = new QLabel;
            //label->setCaption(tr("Lord's general"));
            label->setPixmap(lord->getPixmapPath(category));
            label->setToolTip(lord->getSkillDescription());
            grid_layout->addWidget(label,0,0);
        }

        int i;
        for(i=0; i<buttons.length(); i++){
            int row = i / columns;
            int column = i % columns;
            grid_layout->addWidget(buttons.at(i), row, column+1);
        }
    }

    mapper->setMapping(this, generals.first()->objectName());
    connect(this, SIGNAL(rejected()), mapper, SLOT(map()));

    connect(mapper, SIGNAL(mapped(QString)), ClientInstance, SLOT(chooseItem(QString)));

    QVBoxLayout *dialog_layout = new QVBoxLayout;
    dialog_layout->addLayout(layout);

    // role prompt
    QLabel *role_label = new QLabel(tr("Your role is %1").arg(Sanguosha->translate(Self->getRole())));
    if(lord_name.size())role_label->setText(tr("The lord has chosen %1. %2")
                                            .arg(Sanguosha->translate(lord_name))
                                            .arg(role_label->text()));
    dialog_layout->addWidget(role_label);

    // progress bar & free choose button
    QHBoxLayout *last_layout = new QHBoxLayout;
    if(ServerInfo.OperationTimeout == 0){
        progress_bar = NULL;
    }else{
        progress_bar = new QProgressBar;
        progress_bar->setMinimum(0);
        progress_bar->setMaximum(100);
        progress_bar->setTextVisible(false);
        last_layout->addWidget(progress_bar);
    }

    bool free_choose = ServerInfo.FreeChoose;

    if(free_choose){
        QPushButton *free_choose_button = new QPushButton(tr("Free choose ..."));
        connect(free_choose_button, SIGNAL(clicked()), this, SLOT(freeChoose()));
        last_layout->addWidget(free_choose_button);
    }

    last_layout->addStretch();

    if(last_layout->count() != 0){
        dialog_layout->addLayout(last_layout);
    }

    setLayout(dialog_layout);

    if(ServerInfo.OperationTimeout != 0)
        startTimer(200);
}

void ChooseGeneralDialog::freeChoose(){
    FreeChooseDialog *dialog = new FreeChooseDialog(this);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(general_chosen(QString)), ClientInstance, SLOT(chooseItem(QString)));

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

// -------------------------------------

FreeChooseDialog::FreeChooseDialog(QWidget *parent, bool pair_choose)
    :QDialog(parent), pair_choose(pair_choose)
{
    setWindowTitle(tr("Free choose generals"));

    QTabWidget *tab_widget = new QTabWidget;

    group = new QButtonGroup(this);
    group->setExclusive(! pair_choose);

    QList<const General *> all_generals = Sanguosha->findChildren<const General *>();
    QMap<QString, QList<const General*> > map;
    foreach(const General *general, all_generals){
        map[general->getKingdom()] << general;
    }

    QStringList kingdoms = Sanguosha->getKingdoms();

    foreach(QString kingdom, kingdoms){
        QList<const General *> generals = map[kingdom];

        if(!generals.isEmpty()){
            QWidget *tab = createTab(generals);
            tab_widget->addTab(tab,
                               QIcon(QString("image/kingdom/icon/%1.png").arg(kingdom)),
                               Sanguosha->translate(kingdom));
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
    layout->addWidget(tab_widget);
    layout->addLayout(button_layout);

    setLayout(layout);

    if(!pair_choose)
        group->buttons().first()->click();
}

void FreeChooseDialog::chooseGeneral(){
    if(pair_choose){
        QList<QAbstractButton *> buttons = group->buttons();
        QString first, second;
        foreach(QAbstractButton *button, buttons){
            if(!button->isChecked())
                continue;

            if(first.isEmpty())
                first = button->objectName();
            else{
                second = button->objectName();
                emit pair_chosen(first, second);
                break;
            }
        }
    }else{
        QAbstractButton *button = group->checkedButton();
        if(button){
            emit general_chosen(button->objectName());
        }
    }

    accept();
}

QWidget *FreeChooseDialog::createTab(const QList<const General *> &generals){
    QWidget *tab = new QWidget;

    QGridLayout *layout = new QGridLayout;
    layout->setOriginCorner(Qt::TopLeftCorner);
    QIcon lord_icon("image/system/roles/lord.png");

    const int columns = 4;

    for(int i=0; i<generals.length(); i++){
        const General *general = generals.at(i);
        QString general_name = general->objectName();
        if(general->isTotallyHidden())
            continue;

        QString text = QString("%1[%2]")
                       .arg(Sanguosha->translate(general_name))
                       .arg(Sanguosha->translate(general->getPackage()));

        QAbstractButton *button;
        if(pair_choose)
            button = new QCheckBox(text);
        else
            button = new QRadioButton(text);
        button->setObjectName(general_name);
        button->setToolTip(general->getSkillDescription());
        if(general->isLord())
            button->setIcon(lord_icon);

        group->addButton(button);

        int row = i / columns;
        int column = i % columns;
        layout->addWidget(button, row, column);
    }

    tab->setLayout(layout);

    if(pair_choose){
        connect(group, SIGNAL(buttonClicked(QAbstractButton*)),
                this, SLOT(uncheckExtraButton(QAbstractButton*)));
    }

    return tab;
}

void FreeChooseDialog::uncheckExtraButton(QAbstractButton *click_button){
    QAbstractButton *first = NULL;
    QList<QAbstractButton *> buttons = group->buttons();
    foreach(QAbstractButton *button, buttons){
        if(!button->isChecked())
            continue;

        if(button == click_button)
            continue;

        if(first == NULL)
            first = button;
        else{
            first->setChecked(false);
            break;
        }
    }
}
