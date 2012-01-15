#include "customassigndialog.h"

#include <QPushButton>
#include <QMessageBox>
#include <QRadioButton>
#include <QPixmap>
#include <QIcon>

CustomAssignDialog::CustomAssignDialog(QWidget *parent)
    :QDialog(parent),
      choose_general2(false), free_choose_general(false), free_choose_general2(false)
{
    setWindowTitle(tr("Custom mini scene"));

    list = new QListWidget;
    list->setFlow(QListView::TopToBottom);
    list->setMovement(QListView::Static);

    QVBoxLayout *vlayout = new QVBoxLayout;

    num_combobox = new QComboBox;

    for(int i = 0; i <= 8; i++){
        num_combobox->addItem(tr("%1 persons").arg(QString::number(i+2)), i+2);
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

        QListWidgetItem *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, player);
        item_map[i] = item;
        player_maxhp[player] = 4;
        player_hp[player] = 4;
    }

    role_combobox = new QComboBox;
    role_combobox->addItem(tr("Lord"), "lord");
    role_combobox->addItem(tr("Loyalist"), "loyalist");
    role_combobox->addItem(tr("Renegade"), "renegade");
    role_combobox->addItem(tr("Rebel"), "rebel");

    for(int i=0; i<= num_combobox->currentIndex()+1; i++){
        list->addItem(item_map[i]);
    }
    list->setCurrentItem(item_map[0]);

    general_label = new LabelButton;
    general_label->setPixmap(QPixmap(Sanguosha->getGeneral("caocao")->getPixmapPath("tiny")));
    general_label->setFixedSize(42, 36);

    general_label2 = new LabelButton;
    general_label2->setPixmap(QPixmap(Sanguosha->getGeneral("anjiang")->getPixmapPath("tiny")));
    general_label2->setFixedSize(42, 36);

    QPushButton *equipAssign = new QPushButton(tr("EquipAssign"));
    QPushButton *handcardAssign = new QPushButton(tr("HandcardAssign"));

    max_hp_prompt = new QCheckBox(tr("Max Hp"));
    max_hp_prompt->setChecked(true);
    max_hp_spin = new QSpinBox();
    max_hp_spin->setRange(2,10);
    max_hp_spin->setValue(4);
    max_hp_spin->setEnabled(true);

    hp_prompt = new QCheckBox(tr("Hp"));
    hp_prompt->setChecked(true);
    hp_spin = new QSpinBox();
    hp_spin->setRange(1,max_hp_spin->value());
    hp_spin->setValue(4);
    hp_spin->setEnabled(true);

    self_select_general = new QCheckBox(tr("General Self Select"));
    self_select_general2 = new QCheckBox(tr("General2 Self Select"));

    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));

    vlayout->addWidget(role_combobox);
    vlayout->addWidget(num_combobox);
    QHBoxLayout *label_lay = new QHBoxLayout;
    label_lay->addWidget(general_label);
    label_lay->addWidget(general_label2);
    vlayout->addLayout(label_lay);
    vlayout->addWidget(self_select_general);
    vlayout->addWidget(self_select_general2);
    vlayout->addWidget(equipAssign);
    vlayout->addWidget(handcardAssign);
    vlayout->addLayout(HLay(max_hp_prompt,max_hp_spin));
    vlayout->addLayout(HLay(hp_prompt,hp_spin));
    vlayout->addStretch();
    vlayout->addWidget(okButton);
    vlayout->addWidget(cancelButton);

    equip_list = new QListWidget;
    hand_list = new QListWidget;
    QVBoxLayout *info_lay = new QVBoxLayout();
    info_lay->addWidget(list);
    info_lay->addLayout(HLay(equip_list,hand_list));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addLayout(info_lay);
    layout->addLayout(vlayout);
    QVBoxLayout *mainlayout = new QVBoxLayout();
    mainlayout->addLayout(layout);
    setLayout(mainlayout);

    connect(role_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateRole(int)));
    connect(list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(on_list_itemSelectionChanged(QListWidgetItem*)));
    connect(num_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateNumber(int)));
    connect(general_label, SIGNAL(clicked()), this, SLOT(doGeneralAssign()));
    connect(general_label2, SIGNAL(clicked()), this, SLOT(doGeneralAssign2()));
    connect(max_hp_prompt,SIGNAL(toggled(bool)),max_hp_spin,SLOT(setEnabled(bool)));
    connect(hp_prompt,SIGNAL(toggled(bool)),hp_spin,SLOT(setEnabled(bool)));
    connect(hp_prompt,SIGNAL(toggled(bool)), this, SLOT(setPlayerHpEnabled(bool)));
    connect(max_hp_prompt,SIGNAL(toggled(bool)), this, SLOT(setPlayerMaxHpEnabled(bool)));
    connect(self_select_general, SIGNAL(toggled(bool)), this, SLOT(freeChoose(bool)));
    connect(self_select_general2, SIGNAL(toggled(bool)), this, SLOT(freeChoose2(bool)));
    connect(self_select_general, SIGNAL(toggled(bool)), general_label, SLOT(setDisabled(bool)));
    connect(self_select_general2, SIGNAL(toggled(bool)), general_label2, SLOT(setDisabled(bool)));
    connect(hp_spin, SIGNAL(valueChanged(int)), this, SLOT(getPlayerHp(int)));
    connect(max_hp_spin, SIGNAL(valueChanged(int)), this, SLOT(getPlayerMaxHp(int)));
    connect(equipAssign, SIGNAL(clicked()), this, SLOT(doEquipCardAssign()));
    connect(handcardAssign, SIGNAL(clicked()), this, SLOT(doHandCardAssign()));
    //   connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void CustomAssignDialog::doEquipCardAssign(){
    CardAssignDialog *dialog = new CardAssignDialog(this, "equip");

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(card_chosen(int)), this, SLOT(getEquipCard(int)));
    dialog->exec();
}

void CustomAssignDialog::getEquipCard(int card_id){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    player_equips[name] << card_id;
    updatePlayerInfo(name);
}

void CustomAssignDialog::doHandCardAssign(){
    CardAssignDialog *dialog = new CardAssignDialog(this);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(card_chosen(int)), this, SLOT(getHandCard(int)));
    dialog->exec();
}

void CustomAssignDialog::getHandCard(int card_id){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    player_handcards[name] << card_id;
    updatePlayerInfo(name);
}

void CustomAssignDialog::updateNumber(int num){
    int count = num_combobox->itemData(num).toInt();
    if(count < list->count()){
        for(int i = list->count() - 1; i >= count; i--){
            list->takeItem(i);
        }
    }
    else{
        for(int i= list->count() - 1; i< count; i++){
            list->addItem(item_map[i]);
        }
    }
}

void CustomAssignDialog::updatePlayerInfo(QString name)
{
    equip_list->clear();
    hand_list->clear();

    foreach(int equip_id, player_equips[name])
    {
        const Card* card = Sanguosha->getCard(equip_id);
        QString card_name = Sanguosha->translate(card->objectName());
        QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(card->getSuitString()));
        QString point = card->getNumberString();

        QString card_info = point + "  " + card_name;
        QListWidgetItem *name_item = new QListWidgetItem(card_info, equip_list);
        name_item->setIcon(suit_icon);
        name_item->setData(Qt::UserRole, card->getId());
    }

    foreach(int hand_id, player_handcards[name])
    {
        const Card* card = Sanguosha->getCard(hand_id);
        QString card_name = Sanguosha->translate(card->objectName());
        QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(card->getSuitString()));
        QString point = card->getNumberString();

        QString card_info = point + "  " + card_name;
        QListWidgetItem *name_item = new QListWidgetItem(card_info, hand_list);
        name_item->setIcon(suit_icon);
        name_item->setData(Qt::UserRole, card->getId());
    }
}

void CustomAssignDialog::updatePlayerHpInfo(QString name){
    if(player_hp.value(name, 0) != 0){
        hp_spin->setValue(player_hp[name]);
        hp_prompt->setChecked(true);
    }
    else{
        hp_spin->setValue(4);
        hp_prompt->setChecked(false);
    }

    if(player_maxhp.value(name, 0) != 0){
        max_hp_spin->setValue(player_maxhp[name]);
        max_hp_prompt->setChecked(true);
    }
    else{
        max_hp_spin->setValue(4);
        max_hp_prompt->setChecked(false);
    }
}

void CustomAssignDialog::getPlayerHp(int hp)
{
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    player_hp[name] = hp;
}

void CustomAssignDialog::getPlayerMaxHp(int maxhp){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    player_maxhp[name] = maxhp;
}

void CustomAssignDialog::setPlayerHpEnabled(bool toggled){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    if(!toggled){
        player_hp.remove(name);
    }
    else{
        player_hp[name] = hp_spin->value();
    }
}

void CustomAssignDialog::setPlayerMaxHpEnabled(bool toggled){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    if(!toggled){
        player_maxhp.remove(name);
    }
    else{
        player_maxhp[name] = max_hp_spin->value();
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

void CustomAssignDialog::freeChoose(bool toggled){
    if(list->currentItem()->data(Qt::UserRole).toString() != "player")
        return;

    if(toggled)
        free_choose_general = true;
    else
        free_choose_general = false;
}

void CustomAssignDialog::freeChoose2(bool toggled){
    if(list->currentItem()->data(Qt::UserRole).toString() != "player")
        return;

    if(toggled)
        free_choose_general2 = true;
    else
        free_choose_general2 = false;
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
        general_label2->setPixmap(QPixmap(QString(Sanguosha->getGeneral("anjiang")->getPixmapPath("tiny"))));

    if(!player_name.contains("player")){
        self_select_general->setEnabled(false);
        self_select_general2->setEnabled(false);
        self_select_general->setChecked(false);
        self_select_general2->setChecked(false);
    }
    else{
        self_select_general->setEnabled(true);
        self_select_general2->setEnabled(true);
        self_select_general->setChecked(free_choose_general);
        self_select_general2->setChecked(free_choose_general2);
    }

    updatePlayerInfo(player_name);
    updatePlayerHpInfo(player_name);
}

//extern CustomAssignDialog *CustomInstance;

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

void GeneralAssignDialog::chooseGeneral(){
    QAbstractButton *button = group->checkedButton();
    if(button){
        emit general_chosen(button->objectName());
    }
    this->hide();
}

//------------------------------

CardAssignDialog::CardAssignDialog(QWidget *parent, QString card_type, QString class_name) :
    QDialog(parent)
{

    QVBoxLayout *vlayout = new QVBoxLayout;
    card_list = new QListWidget;

    int i, n = Sanguosha->getCardCount();
    QList<const Card *> reasonable_cards;
    if(!card_type.isEmpty() || !class_name.isEmpty()){
        for(i=0; i<n ;i++){
            const Card *card = Sanguosha->getCard(i);
            if(card->getType() == card_type || card->inherits(class_name.toStdString().c_str()))
                reasonable_cards << card;
        }
    }
    else{
        for(i=0; i<n ;i++){
            const Card *card = Sanguosha->getCard(i);
            reasonable_cards << card;
        }
    }

    for(i = 0; i < reasonable_cards.length(); i++)
        addCard(reasonable_cards.at(i));

    if(n>0)
        card_list->setCurrentRow(0);

    QPushButton *getCardButton = new QPushButton(tr("Get card"));
    QPushButton *back = new QPushButton(tr("Back"));

    vlayout->addWidget(getCardButton);
    vlayout->addWidget(back);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(card_list);
    layout->addLayout(vlayout);
    QVBoxLayout *mainlayout = new QVBoxLayout();
    mainlayout->addLayout(layout);
    setLayout(mainlayout);

    connect(back, SIGNAL(clicked()), this, SLOT(reject()));
    connect(getCardButton, SIGNAL(clicked()), this, SLOT(askCard()));
}

void CardAssignDialog::addCard(const Card *card){
    QString name = Sanguosha->translate(card->objectName());
    QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(card->getSuitString()));
    QString point = card->getNumberString();

    QString card_info = point + "  " + name;
    QListWidgetItem *name_item = new QListWidgetItem(card_info, card_list);
    name_item->setIcon(suit_icon);
    name_item->setData(Qt::UserRole, card->getId());
}

void CardAssignDialog::askCard(){
    QListWidgetItem *card_item = card_list->currentItem();
    int card_id = card_item->data(Qt::UserRole).toInt();
    emit card_chosen(card_id);
    this->hide();
}
