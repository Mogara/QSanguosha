#include "customassigndialog.h"
#include "miniscenarios.h"

#include <QPushButton>
#include <QMessageBox>
#include <QRadioButton>
#include <QPixmap>
#include <QIcon>
#include <QGroupBox>
#include <QFrame>
#include <QFile>
#include <QFileDialog>

static QLayout *HLay(QWidget *left, QWidget *right, QWidget *mid = NULL){
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(left);
    if(mid)
        layout->addWidget(mid);
    layout->addWidget(right);

    return layout;
}

CustomAssignDialog *CustomInstance = NULL;

CustomAssignDialog::CustomAssignDialog(QWidget *parent)
    :QDialog(parent),
      choose_general2(false), free_choose_general(false), free_choose_general2(false),
      is_single_turn(false), is_before_next(false)
{
    setWindowTitle(tr("Custom mini scene"));

    CustomInstance = this;

    list = new QListWidget;
    list->setFlow(QListView::TopToBottom);
    list->setMovement(QListView::Static);

    QVBoxLayout *vlayout = new QVBoxLayout;


    num_combobox = new QComboBox;
    starter_box = new QComboBox;

    for(int i = 0; i <= 9; i++){
        if(i < 9)
            num_combobox->addItem(tr("%1 persons").arg(QString::number(i+2)), i+2);

        QString player = (i == 0 ? "Player" : "AI");
        QString text = i == 0 ?
                    QString("%1[%2]").arg(Sanguosha->translate(player)).arg(tr("unknown"))
                    : QString("%1%2[%3]")
                    .arg(Sanguosha->translate(player))
                    .arg(QString::number(i))
                    .arg(tr("unknown"));
        if(i != 0)
            player.append(QString::number(i));
        player_mapping[i] = player;
        role_mapping[player] = "unknown";

        QListWidgetItem *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, player);
        item_map[i] = item;
    }

    role_combobox = new QComboBox;
    role_combobox->addItem(tr("Lord"), "lord");
    role_combobox->addItem(tr("Loyalist"), "loyalist");
    role_combobox->addItem(tr("Renegade"), "renegade");
    role_combobox->addItem(tr("Rebel"), "rebel");

    for(int i=0; i< num_combobox->currentIndex()+2; i++){
        list->addItem(item_map[i]);
        QString name = player_mapping[i];
        starter_box->addItem(Sanguosha->translate(name), name);
    }
    list->setCurrentItem(item_map[0]);

    player_draw = new QSpinBox();
    player_draw->setRange(0, Sanguosha->getCardCount());
    player_draw->setValue(4);
    player_draw->setEnabled(true);

    QGroupBox *starter_group = new QGroupBox(tr("Start Info"));
    QLabel *start_text = new QLabel(tr("Starter"));
    QLabel *draw_text = new QLabel(tr("Start Draw"));
    QVBoxLayout *starter_lay = new QVBoxLayout();
    starter_group->setLayout(starter_lay);
    starter_lay->addLayout(HLay(start_text, draw_text));
    starter_lay->addLayout(HLay(starter_box, player_draw));

    general_label = new LabelButton;
    general_label->setPixmap(QPixmap("image/system/disabled.png"));
    general_label->setFixedSize(42, 36);
    QGroupBox *general_box = new QGroupBox(tr("General"));
    QVBoxLayout *general_lay = new QVBoxLayout();
    general_box->setLayout(general_lay);
    general_lay->addWidget(general_label);

    general_label2 = new LabelButton;
    general_label2->setPixmap(QPixmap("image/system/disabled.png"));
    general_label2->setFixedSize(42, 36);
    QGroupBox *general_box2 = new QGroupBox(tr("General2"));
    QVBoxLayout *general_lay2 = new QVBoxLayout();
    general_box2->setLayout(general_lay2);
    general_lay2->addWidget(general_label2);

    QPushButton *equipAssign = new QPushButton(tr("EquipAssign"));
    QPushButton *handcardAssign = new QPushButton(tr("HandcardAssign"));
    QPushButton *judgeAssign = new QPushButton(tr("JudgeAssign"));
    QPushButton *pileAssign = new QPushButton(tr("PileCardAssign"));

    max_hp_prompt = new QCheckBox(tr("Max Hp"));
    max_hp_prompt->setChecked(false);
    max_hp_spin = new QSpinBox();
    max_hp_spin->setRange(2,10);
    max_hp_spin->setValue(4);
    max_hp_spin->setEnabled(false);

    hp_prompt = new QCheckBox(tr("Hp"));
    hp_prompt->setChecked(false);
    hp_spin = new QSpinBox();
    hp_spin->setRange(1,max_hp_spin->value());
    hp_spin->setValue(4);
    hp_spin->setEnabled(false);

    self_select_general = new QCheckBox(tr("General Self Select"));
    self_select_general2 = new QCheckBox(tr("General2 Self Select"));

    set_turned = new QCheckBox(tr("Player Turned"));
    set_chained = new QCheckBox(tr("Player Chained"));

    single_turn_text = new QLabel(tr("After this turn "));
    single_turn_text2 = new QLabel(tr("win"));
    single_turn_box = new QComboBox();
    single_turn = new QCheckBox(tr("After this turn you lose"));
    single_turn_box->addItem(tr("Lord"), "Lord+Loyalist");
    single_turn_box->addItem(tr("Renegade"), "Renegade");
    single_turn_box->addItem(tr("Rebel"), "Rebel");

    before_next_text = new QLabel(tr("Before next turn "));
    before_next_text2 = new QLabel(tr("win"));
    before_next_box = new QComboBox();
    before_next = new QCheckBox(tr("Before next turn begin player lose"));
    before_next_box->addItem(tr("Lord"), "Lord+Loyalist");
    before_next_box->addItem(tr("Renegade"), "Renegade");
    before_next_box->addItem(tr("Rebel"), "Rebel");

    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    QPushButton *loadButton = new QPushButton(tr("load"));
    QPushButton *saveButton = new QPushButton(tr("save"));

    vlayout->addWidget(role_combobox);
    vlayout->addWidget(num_combobox);
    QHBoxLayout *label_lay = new QHBoxLayout;
    label_lay->addWidget(general_box);
    label_lay->addWidget(general_box2);
    vlayout->addLayout(label_lay);
    vlayout->addWidget(self_select_general);
    vlayout->addWidget(self_select_general2);
    vlayout->addLayout(HLay(max_hp_prompt,max_hp_spin));
    vlayout->addLayout(HLay(hp_prompt,hp_spin));
    vlayout->addWidget(set_turned);
    vlayout->addWidget(set_chained);
    vlayout->addWidget(starter_group);
    vlayout->addWidget(single_turn);
    vlayout->addLayout(HLay(single_turn_text, single_turn_text2, single_turn_box));
    vlayout->addWidget(before_next);
    vlayout->addLayout(HLay(before_next_text, before_next_text2, before_next_box));
    vlayout->addStretch();
    vlayout->addWidget(loadButton);
    vlayout->addWidget(saveButton);
    vlayout->addWidget(okButton);
    vlayout->addWidget(cancelButton);

    single_turn_text->hide();
    single_turn_text2->hide();
    single_turn_box->hide();
    before_next_text->hide();
    before_next_text2->hide();
    before_next_box->hide();

    equip_list = new QListWidget;
    hand_list = new QListWidget;
    judge_list = new QListWidget;
    pile_list = new QListWidget;
    QVBoxLayout *info_lay = new QVBoxLayout(), *equip_lay = new QVBoxLayout(), *hand_lay = new QVBoxLayout()
            , *judge_lay = new QVBoxLayout(), *pile_lay = new QVBoxLayout();
    info_lay->addWidget(list);
    QGroupBox *equip_group = new QGroupBox(tr("Equips"));
    QGroupBox *hands_group = new QGroupBox(tr("Handcards"));
    QGroupBox *judge_group = new QGroupBox(tr("Judges"));
    QGroupBox *pile_group = new QGroupBox(tr("DrawPile"));
    equip_group->setLayout(equip_lay);
    hands_group->setLayout(hand_lay);
    judge_group->setLayout(judge_lay);
    pile_group->setLayout(pile_lay);

    removeEquipButton = new QPushButton(tr("Remove Equip"));
    removeHandButton = new QPushButton(tr("Remove Handcard"));
    removeJudgeButton = new QPushButton(tr("Remove Judge"));
    removePileButton = new QPushButton(tr("Remove Pilecard"));

    removeEquipButton->setEnabled(false);
    removeHandButton->setEnabled(false);
    removeJudgeButton->setEnabled(false);
    removePileButton->setEnabled(false);
    equip_lay->addWidget(equip_list);
    equip_lay->addLayout(HLay(equipAssign, removeEquipButton));
    hand_lay->addWidget(hand_list);
    hand_lay->addLayout(HLay(handcardAssign, removeHandButton));
    info_lay->addLayout(HLay(equip_group, hands_group));
    judge_lay->addWidget(judge_list);
    judge_lay->addLayout(HLay(judgeAssign, removeJudgeButton));
    pile_lay->addWidget(pile_list);
    pile_lay->addLayout(HLay(pileAssign, removePileButton));
    info_lay->addLayout(HLay(judge_group, pile_group));

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
    connect(set_turned, SIGNAL(toggled(bool)), this, SLOT(doPlayerTurns(bool)));
    connect(set_chained, SIGNAL(toggled(bool)), this, SLOT(doPlayerChains(bool)));
    connect(hp_spin, SIGNAL(valueChanged(int)), this, SLOT(getPlayerHp(int)));
    connect(max_hp_spin, SIGNAL(valueChanged(int)), this, SLOT(getPlayerMaxHp(int)));
    connect(player_draw, SIGNAL(valueChanged(int)), this, SLOT(setPlayerStartDraw(int)));
    connect(starter_box, SIGNAL(currentIndexChanged(int)), this, SLOT(setPlayerDrawNum(int)));
    connect(removeEquipButton, SIGNAL(clicked()), this, SLOT(removeEquipCard()));
    connect(removeHandButton, SIGNAL(clicked()), this, SLOT(removeHandCard()));
    connect(removeJudgeButton, SIGNAL(clicked()), this, SLOT(removeJudgeCard()));
    connect(removePileButton, SIGNAL(clicked()), this, SLOT(removePileCard()));
    connect(equipAssign, SIGNAL(clicked()), this, SLOT(doEquipCardAssign()));
    connect(handcardAssign, SIGNAL(clicked()), this, SLOT(doHandCardAssign()));
    connect(judgeAssign, SIGNAL(clicked()), this, SLOT(doJudgeCardAssign()));
    connect(pileAssign, SIGNAL(clicked()), this, SLOT(doPileCardAssign()));
    connect(single_turn, SIGNAL(toggled(bool)), this, SLOT(checkBeforeNextBox(bool)));
    connect(before_next, SIGNAL(toggled(bool)), this, SLOT(checkSingleTurnBox(bool)));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(loadButton,SIGNAL(clicked()),this,SLOT(load()));
    connect(saveButton,SIGNAL(clicked()),this,SLOT(save()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void CustomAssignDialog::doEquipCardAssign(){
    QList<int> excluded;
    for(int i = 0; i < list->count(); i++){
        excluded.append(player_equips[list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(player_handcards[list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(set_pile);
    }

    CardAssignDialog *dialog = new CardAssignDialog(this, "equip", "", excluded);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(card_chosen(int)), this, SLOT(getEquipCard(int)));
    dialog->exec();
}

void CustomAssignDialog::getEquipCard(int card_id){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    QString card_type = Sanguosha->getCard(card_id)->getSubtype();
    foreach(int id, player_equips[name]){
        if(card_type == Sanguosha->getCard(id)->getSubtype()){
            emit card_addin(id);
            player_equips[name].removeOne(id);
            break;
        }
    }

    player_equips[name] << card_id;
    updatePlayerInfo(name);
    equip_list->setCurrentRow(0);
    removeEquipButton->setEnabled(true);
}

void CustomAssignDialog::doHandCardAssign(){
    QList<int> excluded;
    for(int i = 0; i < list->count(); i++){
        excluded.append(player_handcards[list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(player_equips[list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(player_judges[list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(set_pile);
    }

    CardAssignDialog *dialog = new CardAssignDialog(this, "", "", excluded);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(card_chosen(int)), this, SLOT(getHandCard(int)));
    dialog->exec();
}

void CustomAssignDialog::getHandCard(int card_id){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    if(player_handcards[name].contains(card_id))
        return;

    player_handcards[name] << card_id;
    updatePlayerInfo(name);
    hand_list->setCurrentRow(0);
    removeHandButton->setEnabled(true);
}

void CustomAssignDialog::doJudgeCardAssign(){
    QList<int> excluded;
    for(int i = 0; i < list->count(); i++){
        excluded.append(player_judges[list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(player_handcards[list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(set_pile);
    }

    CardAssignDialog *dialog = new CardAssignDialog(this, "", "DelayedTrick", excluded);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(card_chosen(int)), this, SLOT(getJudgeCard(int)));
    dialog->exec();
}

void CustomAssignDialog::getJudgeCard(int card_id){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    QString card_name = Sanguosha->getCard(card_id)->objectName();
    foreach(int id, player_judges[name]){
        if(Sanguosha->getCard(id)->objectName() == card_name){
            emit card_addin(id);
            player_judges[name].removeOne(id);
            break;
        }
    }

    player_judges[name] << card_id;
    updatePlayerInfo(name);
    judge_list->setCurrentRow(0);
    removeJudgeButton->setEnabled(true);
}

void CustomAssignDialog::doPileCardAssign(){
    QList<int> excluded;
    for(int i = 0; i < list->count(); i++){
        excluded.append(player_handcards[list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(player_equips[list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(player_judges[list->item(i)->data(Qt::UserRole).toString()]);
        excluded.append(set_pile);
    }

    CardAssignDialog *dialog = new CardAssignDialog(this, "", "", excluded);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(card_chosen(int)), this, SLOT(getPileCard(int)));
    dialog->exec();
}

void CustomAssignDialog::getPileCard(int card_id){
    if(set_pile.contains(card_id))
        return;

    set_pile << card_id;
    updatePileInfo();
    pile_list->setCurrentRow(0);
    removePileButton->setEnabled(true);
}

void CustomAssignDialog::updateNumber(int num){
    int count = num_combobox->itemData(num).toInt();
    if(count < list->count()){
        for(int i = list->count() - 1; i >= count; i--){
            list->takeItem(i);
            starter_box->removeItem(i);
        }
    }
    else{
        for(int i= list->count(); i< count; i++){
            list->addItem(item_map[i]);
            QString name = player_mapping[i];
            starter_box->addItem(Sanguosha->translate(name), name);
        }
    }
}

void CustomAssignDialog::updatePlayerInfo(QString name)
{
    equip_list->clear();
    hand_list->clear();
    judge_list->clear();

    if(player_equips[name].isEmpty())
        removeEquipButton->setEnabled(false);
    else
        removeEquipButton->setEnabled(true);
    if(player_handcards[name].isEmpty())
        removeHandButton->setEnabled(false);
    else
        removeHandButton->setEnabled(true);
    if(player_judges[name].isEmpty())
        removeJudgeButton->setEnabled(false);
    else
        removeJudgeButton->setEnabled(true);

    foreach(int equip_id, player_equips[name]){
        const Card* card = Sanguosha->getCard(equip_id);
        QString card_name = Sanguosha->translate(card->objectName());
        QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(card->getSuitString()));
        QString point = card->getNumberString();

        QString card_info = point + "  " + card_name + "\t\t" + Sanguosha->translate(card->getSubtype());
        QListWidgetItem *name_item = new QListWidgetItem(card_info, equip_list);
        name_item->setIcon(suit_icon);
        name_item->setData(Qt::UserRole, card->getId());
    }

    foreach(int hand_id, player_handcards[name]){
        const Card* card = Sanguosha->getCard(hand_id);
        QString card_name = Sanguosha->translate(card->objectName());
        QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(card->getSuitString()));
        QString point = card->getNumberString();

        QString card_info = point + "  " + card_name + "\t\t" + Sanguosha->translate(card->getSubtype());
        QListWidgetItem *name_item = new QListWidgetItem(card_info, hand_list);
        name_item->setIcon(suit_icon);
        name_item->setData(Qt::UserRole, card->getId());
    }

    foreach(int judge_id, player_judges[name]){
        const Card* card = Sanguosha->getCard(judge_id);
        QString card_name = Sanguosha->translate(card->objectName());
        QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(card->getSuitString()));
        QString point = card->getNumberString();

        QString card_info = point + "  " + card_name + "\t\t" + Sanguosha->translate(card->getSubtype());
        QListWidgetItem *name_item = new QListWidgetItem(card_info, judge_list);
        name_item->setIcon(suit_icon);
        name_item->setData(Qt::UserRole, card->getId());
    }

    equip_list->setCurrentRow(0);
    hand_list->setCurrentRow(0);
    judge_list->setCurrentRow(0);
}

void CustomAssignDialog::updatePileInfo(){
    pile_list->clear();

    if(set_pile.isEmpty())
        removePileButton->setEnabled(false);
    else
        removePileButton->setEnabled(true);

    foreach(int card_id, set_pile){
        const Card* card = Sanguosha->getCard(card_id);
        QString card_name = Sanguosha->translate(card->objectName());
        QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(card->getSuitString()));
        QString point = card->getNumberString();

        QString card_info = point + "  " + card_name + "\t\t" + Sanguosha->translate(card->getSubtype());
        QListWidgetItem *name_item = new QListWidgetItem(card_info, pile_list);
        name_item->setIcon(suit_icon);
        name_item->setData(Qt::UserRole, card->getId());
    }

    pile_list->setCurrentRow(0);
}

void CustomAssignDialog::updatePlayerHpInfo(QString name){
    if(player_hp.value(name, 0) != 0){
        hp_spin->setValue(player_hp[name]);
        hp_prompt->setChecked(true);
    }
    else{
        hp_prompt->setChecked(false);
    }

    if(player_maxhp.value(name, 0) != 0){
        max_hp_spin->setValue(player_maxhp[name]);
        max_hp_prompt->setChecked(true);
    }
    else{
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

void CustomAssignDialog::setPlayerStartDraw(int draw_num){
    QString name = starter_box->itemData(starter_box->currentIndex()).toString();
    player_start_draw[name] = draw_num;
}

void CustomAssignDialog::setPlayerDrawNum(int index){
    QString name = starter_box->itemData(index).toString();
    int val = 4;
    if(player_start_draw.keys().contains(name)) val = player_start_draw[name];
    player_draw->setValue(val);
}

void CustomAssignDialog::updateRole(int index){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    QString role = role_combobox->itemData(index).toString();
    QString text = QString("%1[%2]").arg(Sanguosha->translate(name)).arg(Sanguosha->translate(role));
    list->currentItem()->setText(text);
    role_mapping[name] = role;
}

void CustomAssignDialog::removeEquipCard(){
    int card_id = equip_list->currentItem()->data(Qt::UserRole).toInt();
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    if(player_equips[name].contains(card_id)){
        player_equips[name].removeOne(card_id);
        int row = equip_list->currentRow();
        equip_list->takeItem(row);
        if(equip_list->count() > 0)
            equip_list->setCurrentRow(row >= equip_list->count() ? row-1 : row);
        else
            removeEquipButton->setEnabled(false);
    }
}

void CustomAssignDialog::removeHandCard(){
    int card_id = hand_list->currentItem()->data(Qt::UserRole).toInt();
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    if(player_handcards[name].contains(card_id)){
        player_handcards[name].removeOne(card_id);
        int row = hand_list->currentRow();
        hand_list->takeItem(row);
        if(hand_list->count() > 0)
            hand_list->setCurrentRow(row >= hand_list->count() ? row-1 : row);
        else
            removeHandButton->setEnabled(false);
    }
}

void CustomAssignDialog::removeJudgeCard(){
    int card_id = judge_list->currentItem()->data(Qt::UserRole).toInt();
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    if(player_judges[name].contains(card_id)){
        player_judges[name].removeOne(card_id);
        int row = judge_list->currentRow();
        judge_list->takeItem(row);
        if(judge_list->count() > 0)
            judge_list->setCurrentRow(row >= judge_list->count() ? row-1 : row);
        else
            removeJudgeButton->setEnabled(false);
    }
}

void CustomAssignDialog::removePileCard(){
    int card_id = pile_list->currentItem()->data(Qt::UserRole).toInt();
    if(set_pile.contains(card_id)){
        set_pile.removeOne(card_id);
        int row = pile_list->currentRow();
        pile_list->takeItem(row);
        if(pile_list->count() > 0)
            pile_list->setCurrentRow(row >= pile_list->count() ? row-1 : row);
        else
            removePileButton->setEnabled(false);
    }
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
    GeneralAssignDialog *dialog = new GeneralAssignDialog(this, true);

    connect(dialog, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialog, SIGNAL(general_chosen(QString)), this, SLOT(getChosenGeneral(QString)));
    connect(dialog, SIGNAL(general_cleared()), this, SLOT(clearGeneral2()));
    dialog->exec();
}

void CustomAssignDialog::accept(){
    if(save("etc/customScenes/custom_scenario.txt"))
    {
        const Scenario * scene = Sanguosha->getScenario("custom_scenario");
        MiniSceneRule *rule = qobject_cast<MiniSceneRule*>(scene->getRule());

        rule->loadSetting("etc/customScenes/custom_scenario.txt");
        emit scenario_changed();
        QDialog::accept();
    }
}

void CustomAssignDialog::reject(){
    QDialog::reject();
}

void CustomAssignDialog::clearGeneral2(){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    general2_mapping[name].clear();

    general_label2->setPixmap(QPixmap("image/system/disabled.png"));
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
    if(list->currentItem()->data(Qt::UserRole).toString() != "Player")
        return;

    if(toggled)
        free_choose_general = true;
    else
        free_choose_general = false;
}

void CustomAssignDialog::freeChoose2(bool toggled){
    if(list->currentItem()->data(Qt::UserRole).toString() != "Player")
        return;

    if(toggled)
        free_choose_general2 = true;
    else
        free_choose_general2 = false;
}

void CustomAssignDialog::doPlayerChains(bool toggled){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    player_chained[name] = toggled;
}

void CustomAssignDialog::doPlayerTurns(bool toggled){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    player_turned[name] = toggled;
}

void CustomAssignDialog::on_list_itemSelectionChanged(QListWidgetItem *current){
    QString player_name = current->data(Qt::UserRole).toString();
    if(!general_mapping.value(player_name, "").isEmpty()){
        general_label->setPixmap(QPixmap
                                 (QString(Sanguosha->getGeneral(general_mapping.value(player_name))->getPixmapPath("tiny"))));
    }
    else
        general_label->setPixmap(QPixmap(QString("image/system/disabled.png")));


    if(!general2_mapping.value(player_name, "").isEmpty())
        general_label2->setPixmap(QPixmap
                                  (QString(Sanguosha->getGeneral(general2_mapping.value(player_name))->getPixmapPath("tiny"))));
    else
        general_label2->setPixmap(QPixmap(QString("image/system/disabled.png")));

    if(!role_mapping[player_name].isEmpty()){
        for(int i = 0; i < role_combobox->count(); i++){
            if(role_mapping[player_name] == role_combobox->itemData(i).toString()){
                role_combobox->setCurrentIndex(i);
                updateRole(i);
                break;
            }
        }
    }

    if(!player_name.contains("Player")){
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

    set_turned->setChecked(player_turned.value(player_name, false));
    set_chained->setChecked(player_chained.value(player_name, false));

    single_turn->setChecked(is_single_turn);
    before_next->setChecked(is_before_next);

    updatePlayerInfo(player_name);
    updatePlayerHpInfo(player_name);
}

void CustomAssignDialog::checkBeforeNextBox(bool toggled){
    if(toggled){
        before_next->setChecked(false);
        is_before_next = false;
        is_single_turn = true;

        single_turn_text->show();
        single_turn_text2->show();
        single_turn_box->show();
    }
    else{
        is_single_turn = false;

        single_turn_text->hide();
        single_turn_text2->hide();
        single_turn_box->hide();
    }
}

void CustomAssignDialog::checkSingleTurnBox(bool toggled){
    if(toggled){
        single_turn->setChecked(false);
        is_before_next = true;
        is_single_turn = false;

        before_next_text->show();
        before_next_text2->show();
        before_next_box->show();
    }
    else{
        is_before_next = false;

        before_next_text->hide();
        before_next_text2->hide();
        before_next_box->hide();
    }
}

void CustomAssignDialog::load()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open mini scenario settings"),
                                                    "etc/customScenes",
                                                    tr("Pure text replay file (*.txt)"));


    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    role_mapping.clear();
    general_mapping.clear();
    general2_mapping.clear();
    player_maxhp.clear();
    player_hp.clear();
    player_start_draw.clear();
    player_chained.clear();
    player_turned.clear();

    player_handcards.clear();
    player_equips.clear();
    player_judges.clear();

    free_choose_general = false;
    free_choose_general2= false;

    QTextStream in(&file);
    int numPlayer = 0;
    QMap<QString, int> role_index;
    role_index["Lord+Loyalist"] = 0;
    role_index["Renegade"] = 1;
    role_index["Rebel"] = 2;

    while (!in.atEnd()) {
        QString line = in.readLine();
        line = line.trimmed();

        if(!line.startsWith("general") && !line.startsWith("setPile")){
            QMessageBox::warning(this, tr("Warning"), tr("Data is unreadable"));
            file.close();
            return;
        }

        if(line.startsWith("setPile:"))
        {
            set_pile.clear();
            QStringList list = line.replace("setPile:","").split(",");
            foreach(QString id,list)
            {
                set_pile.prepend(id.toInt());
            }
            continue;
        }

        QString name = numPlayer == 0 ? "Player" : QString("AI%1").arg(numPlayer);

        QMap<QString, QString> player;
        QStringList features;
        if(line.contains("|"))features= line.split("|");
        else features = line.split(" ");
        foreach(QString str, features)
        {
            QStringList keys = str.split(":");
            if(keys.size()<2)continue;
            if(keys.first().size()<1)continue;
            player.insert(keys.at(0),keys.at(1));
        }

        if(player["role"]!= NULL)role_mapping[name] = player["role"];

        if(player["general"]=="select")free_choose_general = true;
        else if(player["general"]!=NULL)general_mapping[name]=player["general"];

        if(player["general2"]=="select")free_choose_general2 = true;
        else if(player["general2"]!=NULL)general2_mapping[name]=player["general2"];

        if(player["maxhp"]!=NULL)player_maxhp[name]=player["maxhp"].toInt();
        if(player["hp"]!=NULL)player_hp[name]=player["hp"].toInt();
        if(player_hp[name]>player_maxhp[name])player_hp[name]=player_maxhp[name];
        if(player["draw"]!=NULL)player_start_draw[name]=player["draw"].toInt();
        else player_start_draw[name] = 4;

        if(player["starter"]!=NULL)starter = name;
        if(player["chained"]!=NULL)player_chained[name]=true;
        if(player["turned"]!=NULL)player_turned[name]=true;
        if(player["singleTurn"] != NULL){
            single_turn_box->setCurrentIndex(role_index.value(player["singleTurn"], 0));
            is_single_turn = true;
        }
        if(player["beforeNext"] != NULL){
            before_next_box->setCurrentIndex(role_index.value(player["beforeNext"], 0));
            is_before_next = true;
        }

        if(player["hand"]!=NULL)
        {
            foreach(QString id,player["hand"].split(","))
            {
                bool ok;
                int num = id.toInt(&ok);
                if(!ok){
                    for(int i = 0; i < Sanguosha->getCardCount(); i++){
                        if(Sanguosha->getCard(i)->objectName() == id){
                            player_handcards[name].prepend(i);
                            break;
                        }
                    }
                }else
                    player_handcards[name].prepend(num);
            }
        }

        if(player["equip"]!=NULL)
        {
            foreach(QString id,player["equip"].split(","))
            {
                bool ok;
                int num = id.toInt(&ok);
                if(!ok){
                    for(int i = 0; i < Sanguosha->getCardCount(); i++){
                        if(Sanguosha->getCard(i)->objectName() == id){
                            player_equips[name].prepend(i);
                            break;
                        }
                    }
                }else
                    player_equips[name].prepend(num);
            }
        }

        if(player["judge"]!=NULL)
        {
            foreach(QString id,player["judge"].split(","))
            {
                bool ok;
                int num = id.toInt(&ok);
                if(!ok){
                    for(int i = 0; i < Sanguosha->getCardCount(); i++){
                        if(Sanguosha->getCard(i)->objectName() == id){
                            player_judges[name].prepend(i);
                            break;
                        }
                    }
                }else
                    player_judges[name].prepend(num);
            }
        }
        numPlayer++;
    }

    updateNumber(numPlayer-2);
    list->setCurrentRow(0);
    for(int i=list->count()-1;i>=0;i--)
    {
        list->setCurrentItem(list->item(i));

    }

    player_draw->setValue(player_start_draw[starter_box->currentText()]);
    num_combobox->setCurrentIndex(list->count()-2);

    updatePileInfo();
    file.close();
}

bool CustomAssignDialog::save(QString path)
{
    starter = starter_box->itemData(starter_box->currentIndex()).toString();
    QMap<QString, int> role_index;
    role_index["loyalist"] = 0;
    role_index["lord"] = 0;
    role_index["rebel"] = 1;
    role_index["renegade"] = 2;

    int role_index_check = -1;
    bool has_lord = false, has_diff_roles = false;
    for(int index = 0; index < list->count(); index++){
        QString name = list->item(index)->data(Qt::UserRole).toString();
        if(!has_diff_roles){
            int role_int = role_index.value(role_mapping[name], 3);
            if(role_int != 3){
                if(role_index_check != -1 && role_int != role_index_check){
                    has_diff_roles = true;
                }
                else if(role_index_check == -1){
                    role_index_check = role_int;
                }
            }
        }

        if(role_mapping[name] == "lord"){
            if(has_lord){
                QMessageBox::warning(this, tr("Warning"), tr("Two many lords in the game"));
                return false;
            }
            else
                has_lord = true;
        }
    }

    if(!has_lord){
        QMessageBox::warning(this, tr("Warning"), tr("No lord in the game"));
        return false;
    }
    if(!has_diff_roles){
        QMessageBox::warning(this, tr("Warning"), tr("No different camps in the game"));
        return false;
    }

    QString line;
    if(set_pile.length())
    {
        foreach(int id, set_pile)
        {
            line.prepend(QString::number(id));
            line.prepend(",");
        }
        line.remove(0,1);
        line.prepend("setPile:");
        line.append("\n");
    }

    if(free_choose_general)line.append("general:select ");
    else if(general_mapping["Player"].isEmpty()){
        QMessageBox::warning(this, tr("Warning"), tr("%1's general cannot be empty").arg(Sanguosha->translate("Player")));
        return false;
    }
    else
        line.append(QString("general:%1 ").arg(general_mapping["Player"]));

    if(free_choose_general2)line.append("general2:select ");
    else if(!general2_mapping["Player"].isEmpty())line.append(QString("general2:%1 ").arg(general2_mapping["Player"]));

    for(int i=0;i<list->count();i++)
    {
        QString name = i==0 ? "Player" : QString("AI%1").arg(i);

        if(general_mapping[name].isEmpty() && !line.split('\n').last().contains("general:")){
            QMessageBox::warning(this, tr("Warning"), tr("%1's general cannot be empty").arg(Sanguosha->translate(name)));
            return false;
        }
        else if(!line.split('\n').last().contains(QString("general:")))
            line.append(QString("general:%1 ").arg(general_mapping[name]));

        if(!general2_mapping[name].isEmpty() && !line.split('\n').last().contains(QString("general2:")))
            line.append(QString("general2:%1 ").arg(general2_mapping[name]));

        if(role_mapping[name] == "unknown"){
            QMessageBox::warning(this, tr("Warning"), tr("%1's role cannot be unknown").arg(Sanguosha->translate(name)));
            return false;
        }
        line.append(QString("role:%1 ").arg(role_mapping[name]));
        if(starter == name)line.append("starter:true ");
        if(player_maxhp[name]>0)line.append(QString("maxhp:%1 ").arg(player_maxhp[name]));
        if(player_hp[name]>0)line.append(QString("hp:%1 ").arg(player_hp[name]));
        if(player_turned[name])line.append("turned:true ");
        if(player_chained[name])line.append("chained:true ");
        if(i == 0){
            if(is_single_turn){
                QString winner = single_turn_box->itemData(single_turn_box->currentIndex()).toString();
                line.append(QString("singleTurn:%1 ").arg(winner));
            }
            else if(is_before_next){
                QString winner = before_next_box->itemData(before_next_box->currentIndex()).toString();
                line.append(QString("beforeNext:%1 ").arg(winner));
            }
        }
        if(player_start_draw.contains(name) && player_start_draw[name] != 4)
            line.append(QString("draw:%1 ").arg(player_start_draw[name]));

        if(player_equips[name].length())
        {
            line.append("equip:");
            foreach(int equip,player_equips[name])line.append(QString("%1,").arg(equip));
            line.chop(1);
            line.append(" ");
        }

        if(player_handcards[name].length())
        {
            line.append("hand:");
            foreach(int hand,player_handcards[name])line.append(QString("%1,").arg(hand));
            line.chop(1);
            line.append(" ");
        }

        if(player_judges[name].length())
        {
            line.append("judge:");
            foreach(int judge,player_judges[name])line.append(QString("%1,").arg(judge));
            line.chop(1);
            line.append(" ");
        }

        line.append("\n");
    }

    QString filename = path;
    if(path.size()<1)filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save mini scenario settings"),
                                                    "etc/customScenes/",
                                                    tr("Pure text replay file (*.txt)"));

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;


    QTextStream out(&file);
    out << line;
    file.close();
    return true;
}
//---------------------------------------

GeneralAssignDialog::GeneralAssignDialog(QWidget *parent, bool can_ban)
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
    if(can_ban){
        QPushButton *clear_button = new QPushButton(tr("Clear General"));
        connect(clear_button, SIGNAL(clicked()), this, SLOT(clearGeneral()));

        button_layout->addWidget(clear_button);
    }

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
    this->reject();
}

void GeneralAssignDialog::clearGeneral(){
    emit general_cleared();
    this->reject();
}

//------------------------------

CardAssignDialog::CardAssignDialog(QWidget *parent, QString card_type, QString class_name, QList<int> excluded) :
    QDialog(parent), card_type(card_type), class_name(class_name),
    excluded_card(excluded)
{

    setWindowTitle(tr("Custom Card Chosen"));
    QVBoxLayout *vlayout = new QVBoxLayout;
    card_list = new QListWidget;

    updateCardList();

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
    connect(CustomInstance, SIGNAL(card_addin(int)), this, SLOT(updateExcluded(int)));
}

void CardAssignDialog::addCard(const Card *card){
    QString name = Sanguosha->translate(card->objectName());
    QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(card->getSuitString()));
    QString point = card->getNumberString();

    QString card_info = point + "  " + name + "\t\t" + Sanguosha->translate(card->getSubtype());
    QListWidgetItem *name_item = new QListWidgetItem(card_info, card_list);
    name_item->setIcon(suit_icon);
    name_item->setData(Qt::UserRole, card->getId());
}

void CardAssignDialog::askCard(){
    QListWidgetItem *card_item = card_list->currentItem();
    int card_id = card_item->data(Qt::UserRole).toInt();
    emit card_chosen(card_id);

    int row = card_list->currentRow();
    int id = card_list->item(row)->data(Qt::UserRole).toInt();
    excluded_card << id;
    updateCardList();
    card_list->setCurrentRow(row >= card_list->count() ? row-1 : row);
}

void CardAssignDialog::updateExcluded(int card_id){
    excluded_card.removeOne(card_id);
}

void CardAssignDialog::updateCardList(){
    card_list->clear();

    int i, n = Sanguosha->getCardCount();
    QList<const Card *> reasonable_cards;
    if(!card_type.isEmpty() || !class_name.isEmpty()){
        for(i=0; i<n ;i++){
            if(excluded_card.contains(i))
                continue;

            const Card *card = Sanguosha->getCard(i);
            if(card->getType() == card_type || card->inherits(class_name.toStdString().c_str()))
                reasonable_cards << card;
        }
    }
    else{
        for(i=0; i<n ;i++){
            if(excluded_card.contains(i))
                continue;

            const Card *card = Sanguosha->getCard(i);
            reasonable_cards << card;
        }
    }

    for(i = 0; i < reasonable_cards.length(); i++)
        addCard(reasonable_cards.at(i));

    if(n>0)
        card_list->setCurrentRow(0);
}
