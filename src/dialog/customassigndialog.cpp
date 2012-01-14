#include "customassigndialog.h"

#include <QPushButton>
#include <QMessageBox>
#include <QRadioButton>
#include <QPixmap>

CustomAssignDialog::CustomAssignDialog(QWidget *parent)
    :QDialog(parent), choose_general2(false)
{
    setWindowTitle(tr("Custom mini scene"));

    list = new QListWidget;
    list->setFlow(QListView::TopToBottom);
    list->setMovement(QListView::Static);

    QVBoxLayout *vlayout = new QVBoxLayout;

    num_combobox = new QComboBox;

    for(int i = 2; i <= 10; i++)
        num_combobox->addItem(tr("%1 persons").arg(QString::number(i)), i);

    role_combobox = new QComboBox;
    role_combobox->addItem(tr("Lord"), "lord");
    role_combobox->addItem(tr("Loyalist"), "loyalist");
    role_combobox->addItem(tr("Renegade"), "renegade");
    role_combobox->addItem(tr("Rebel"), "rebel");

    for(int i=0; i<= num_combobox->currentIndex()+1; i++){
        QString player = (i == 0 ? "player" : "ai");
        QString text = i == 0 ?
                    QString("%1[%2]").arg(Sanguosha->translate(player)).arg(Sanguosha->translate("unknown"))
                    : QString("%1%2[%3]")
                    .arg(Sanguosha->translate(player))
                    .arg(QString::number(i))
                    .arg(Sanguosha->translate("unknown"));
        if(i != 0)
            player.append(QString::number(i));
        player_mapping[i] = player;
        role_mapping[player] = "unknown";

        QListWidgetItem *item = new QListWidgetItem(text, list);
        item->setData(Qt::UserRole, player);
        item_map[i] = item;
    }
    list->setCurrentItem(item_map[0]);

    general_label = new QLabel;
    general_label->setPixmap(QPixmap(Sanguosha->getGeneral("caocao")->getPixmapPath("tiny")));
    general_label->setFixedSize(42, 36);

    general_label2 = new QLabel;
    general_label2->setFixedSize(42, 36);

    QPushButton *generalButton = new QPushButton(tr("ChooseGeneral"));
    QPushButton *generalButton2 = new QPushButton(tr("ChooseGeneral2"));

    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));

    vlayout->addWidget(role_combobox);
    vlayout->addWidget(num_combobox);
    vlayout->addWidget(general_label);
    vlayout->addWidget(generalButton);
    vlayout->addWidget(general_label2);
    vlayout->addWidget(generalButton2);
    vlayout->addStretch();
    vlayout->addWidget(okButton);
    vlayout->addWidget(cancelButton);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(list);
    layout->addLayout(vlayout);
    QVBoxLayout *mainlayout = new QVBoxLayout();
    mainlayout->addLayout(layout);
    setLayout(mainlayout);

    connect(role_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateRole(int)));
    connect(list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(on_list_itemSelectionChanged(QListWidgetItem*)));
    connect(num_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateNumber(int)));
    connect(generalButton, SIGNAL(clicked()), this, SLOT(doGeneralAssign()));
    connect(generalButton2, SIGNAL(clicked()), this, SLOT(doGeneralAssign2()));
    //   connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void CustomAssignDialog::updateNumber(int num){
    if(num+1 < list->count()){
        for(int i = list->count() - 1; i > num+1; i--){
            list->takeItem(i);
            player_mapping[i] = "";
        }
    }
    else{
        for(int i= list->count() - 1; i<= num; i++){
            QString text = QString("ai%1[%2]")
                        .arg(QString::number(i+1))
                        .arg(Sanguosha->translate("unknown"));
            QString player = "ai" + (QString::number(i+1));
            player_mapping[i] = player;
            role_mapping[player] = "unknown";

            QListWidgetItem *item = new QListWidgetItem(text, list);
            item->setData(Qt::UserRole, player);
            item_map[i] = item;
        }
    }
}
void CustomAssignDialog::updateRole(int index){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    QString role = role_combobox->itemData(index).toString();
    QString text = QString("%1[%2]").arg(name).arg(Sanguosha->translate(role));
    list->currentItem()->setText(text);
    role_mapping[name] = role;
}

void CustomAssignDialog::doGeneralAssign(){
    choose_general2 = false;
    GeneralAssignDialog *dialog = new GeneralAssignDialog(this);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(general_chosen(QString)), this, SLOT(getChosenGeneral(QString)));
    dialog->exec();
}

void CustomAssignDialog::doGeneralAssign2(){
    choose_general2 = true;
    GeneralAssignDialog *dialog = new GeneralAssignDialog(this);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(general_chosen(QString)), this, SLOT(getChosenGeneral(QString)));
    dialog->exec();
}

void CustomAssignDialog::reject(){
    QDialog::reject();
}

void CustomAssignDialog::getChosenGeneral(QString name){
    if(choose_general2){
        const General *general2 = Sanguosha->getGeneral(name);
        general_label2->setPixmap(QPixmap(general2->getPixmapPath("tiny")));
        if(list->currentItem())
            general2_mapping[list->currentItem()->data(Qt::UserRole).toString()] = name;
    }
    else{
        const General *general = Sanguosha->getGeneral(name);
        general_label->setPixmap(QPixmap(general->getPixmapPath("tiny")));
        if(list->currentItem())
            general_mapping[list->currentItem()->data(Qt::UserRole).toString()] = name;
    }
}

void CustomAssignDialog::on_list_itemSelectionChanged(QListWidgetItem *current){
    QString player_name = current->data(Qt::UserRole).toString();
    if(!general_mapping.value(player_name, "").isEmpty()){
        general_label->setPixmap(QPixmap
                                 (QString(Sanguosha->getGeneral(general_mapping.value(player_name))->getPixmapPath("tiny"))));
    }
    else
        general_label->setPixmap(QPixmap(QString(Sanguosha->getGeneral("caocao")->getPixmapPath("tiny"))));


    if(!general2_mapping.value(player_name, "").isEmpty())
        general_label2->setPixmap(QPixmap
                                  (QString(Sanguosha->getGeneral(general2_mapping.value(player_name))->getPixmapPath("tiny"))));
    else
        general_label2->setPixmap(QPixmap("null"));
}

//---------------------------------------

GeneralAssignDialog::GeneralAssignDialog(QWidget *parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Mini choose generals"));

    QTabWidget *tab_widget = new QTabWidget;

    group = new QButtonGroup(this);
    group->setExclusive(true);

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

    group->buttons().first()->click();
}

QWidget *GeneralAssignDialog::createTab(const QList<const General *> &generals){
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
    return tab;
}

void GeneralAssignDialog::uncheckExtraButton(QAbstractButton *click_button){
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

void GeneralAssignDialog::chooseGeneral(){
    QAbstractButton *button = group->checkedButton();
    if(button){
        emit general_chosen(button->objectName());
    }
    this->hide();
}
