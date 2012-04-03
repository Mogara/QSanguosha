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
#include <QCommandLinkButton>
#include <QCompleter>

static QLayout *HLay(QWidget *left, QWidget *right, QWidget *mid = NULL,
                     QWidget *rear = NULL, bool is_vertically = false){
    QBoxLayout *layout;
    if(is_vertically) layout = new QVBoxLayout;
    else layout = new QHBoxLayout;

    layout->addWidget(left);
    if(mid)
        layout->addWidget(mid);
    layout->addWidget(right);
    if(rear) layout->addWidget(rear);

    return layout;
}

CustomAssignDialog *CustomInstance = NULL;


CustomAssignDialog::CustomAssignDialog(QWidget *parent)
    :QDialog(parent),
      choose_general2(false),
      is_single_turn(false), is_before_next(false), is_random_roles(false)
{
    setWindowTitle(tr("Custom mini scene"));

    CustomInstance = this;

    list = new QListWidget;
    list->setFlow(QListView::TopToBottom);
    list->setMovement(QListView::Static);

    QVBoxLayout *vlayout = new QVBoxLayout;
    num_combobox = new QComboBox;
    for(int i = 0; i <= 9; i++){
        if(i < 9)
            num_combobox->addItem(tr("%1 persons").arg(QString::number(i+2)), i+2);

        QString player = (i == 0 ? "Player" : "AI");
        QString text = i == 0 ?
                    QString("%1[%2]").arg(Sanguosha->translate(player)).arg(tr("Unknown"))
                    : QString("%1%2[%3]")
                    .arg(Sanguosha->translate(player))
                    .arg(QString::number(i))
                    .arg(tr("Unknown"));
        if(i != 0)
            player.append(QString::number(i));
        player_mapping[i] = player;
        role_mapping[player] = "unknown";
        set_nationality[player] = false;

        QListWidgetItem *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, player);
        item_map[i] = item;
    }

    role_combobox = new QComboBox;
    role_combobox->addItem(tr("Unknown"), "unknown");
    role_combobox->addItem(tr("Lord"), "lord");
    role_combobox->addItem(tr("Loyalist"), "loyalist");
    role_combobox->addItem(tr("Renegade"), "renegade");
    role_combobox->addItem(tr("Rebel"), "rebel");

    for(int i=0; i< num_combobox->currentIndex()+2; i++){
        list->addItem(item_map[i]);
    }
    list->setCurrentItem(item_map[0]);

    player_draw = new QSpinBox();
    player_draw->setRange(0, Sanguosha->getCardCount());
    player_draw->setValue(4);
    player_draw->setEnabled(true);

    QGroupBox *starter_group = new QGroupBox(tr("Start Info"));
    starter_box = new QCheckBox(tr("Set as Starter"));
    QLabel *draw_text = new QLabel(tr("Start Draw"));
    QLabel *mark_text = new QLabel(tr("marks"));
    QLabel *mark_num_text = new QLabel(tr("pieces"));

    marks_combobox = new QComboBox;
    marks_combobox->addItem(tr("None"));
    QString path = "image/mark";
    QDir *dir = new QDir(path);
    QStringList filter;
    filter << "*.png";
    dir->setNameFilters(filter);
    QList<QFileInfo> file_info(dir->entryInfoList(filter));
    foreach(QFileInfo file, file_info){
        QString mark_name = file.fileName().split(".").first();
        QString mark_translate = Sanguosha->translate(mark_name);
        if(!mark_translate.startsWith("@")){
            marks_combobox->addItem(mark_translate, mark_name);
            QLabel *mark_icon = new QLabel(mark_translate);
            mark_icon->setPixmap(QPixmap(file.filePath()));
            mark_icon->setObjectName(mark_name);
            mark_icon->setToolTip(tr("%1 mark").arg(mark_translate));
            mark_icons << mark_icon;
        }
    }

    marks_count = new QSpinBox;
    marks_count->setRange(0, 999);
    marks_count->setEnabled(false);

    QVBoxLayout *starter_lay = new QVBoxLayout();
    starter_group->setLayout(starter_lay);
    starter_lay->addWidget(starter_box);
    starter_lay->addLayout(HLay(draw_text, player_draw));
    starter_lay->addLayout(HLay(marks_combobox, marks_count, mark_text, mark_num_text));

    QGridLayout *grid_layout = new QGridLayout;
    const int columns = mark_icons.length() > 10 ? 5 : 4;
    for(int i=0; i<mark_icons.length(); i++){
        int row = i / columns;
        int column = i % columns;
        grid_layout->addWidget(mark_icons.at(i), row, column+1);
        mark_icons.at(i)->hide();
    }
    starter_lay->addLayout(grid_layout);

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

    random_roles_box = new QCheckBox(tr("RandomRoles"));

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

    choose_nationality = new QCheckBox(tr("Customize Nationality"));
    nationalities = new QComboBox;
    int index = 0;
    foreach(QString kingdom, Sanguosha->getKingdoms()){
        nationalities->addItem(QIcon(QString("image/kingdom/icon/%1.png").arg(kingdom)), Sanguosha->translate(kingdom), kingdom);
        kingdom_index[kingdom] = index;
        index ++;
    }
    nationalities->setEnabled(false);

    extra_skill_set = new QPushButton(tr("Set Extra Skills"));

    single_turn_text = new QLabel(tr("After this turn "));
    single_turn_text2 = new QLabel(tr("win"));
    single_turn_box = new QComboBox();
    single_turn = new QCheckBox(tr("After this turn you lose"));
    single_turn_box->addItem(tr("Lord"), "lord+loyalist");
    single_turn_box->addItem(tr("Renegade"), "renegade");
    single_turn_box->addItem(tr("Rebel"), "rebel");

    before_next_text = new QLabel(tr("Before next turn "));
    before_next_text2 = new QLabel(tr("win"));
    before_next_box = new QComboBox();
    before_next = new QCheckBox(tr("Before next turn begin player lose"));
    before_next_box->addItem(tr("Lord"), "lord+loyalist");
    before_next_box->addItem(tr("Renegade"), "Renegade");
    before_next_box->addItem(tr("Rebel"), "Rebel");

    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    QPushButton *loadButton = new QPushButton(tr("load"));
    QPushButton *saveButton = new QPushButton(tr("save"));
    QPushButton *defaultLoadButton = new QPushButton(tr("Default load"));
    defaultLoadButton->setObjectName("default_load");

    vlayout->addWidget(role_combobox);
    vlayout->addWidget(num_combobox);
    QHBoxLayout *label_lay = new QHBoxLayout;
    label_lay->addWidget(general_box);
    label_lay->addWidget(general_box2);
    vlayout->addLayout(label_lay);
    vlayout->addLayout(HLay(self_select_general, self_select_general2));
    vlayout->addLayout(HLay(max_hp_prompt,max_hp_spin));
    vlayout->addLayout(HLay(hp_prompt,hp_spin));
    vlayout->addLayout(HLay(set_turned, set_chained));
    vlayout->addLayout(HLay(choose_nationality, nationalities));
    vlayout->addWidget(random_roles_box);
    vlayout->addWidget(extra_skill_set);
    vlayout->addWidget(starter_group);
    vlayout->addWidget(single_turn);
    vlayout->addLayout(HLay(single_turn_text, single_turn_text2, single_turn_box));
    vlayout->addWidget(before_next);
    vlayout->addLayout(HLay(before_next_text, before_next_text2, before_next_box));
    vlayout->addStretch();
    vlayout->addWidget(defaultLoadButton);
    vlayout->addLayout(HLay(loadButton, saveButton));
    vlayout->addLayout(HLay(okButton, cancelButton));

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

    move_list_up_button = new QPushButton(tr("Move Up"));
    move_list_down_button = new QPushButton(tr("Move Down"));
    move_list_check = new QCheckBox(tr("Move Player List"));
    move_pile_check = new QCheckBox(tr("Move Pile List"));

    move_list_check->setObjectName("list check");
    move_pile_check->setObjectName("pile check");
    move_list_up_button->setObjectName("list_up");
    move_list_down_button->setObjectName("list_down");
    move_list_up_button->setEnabled(false);
    move_list_down_button->setEnabled(false);
    QVBoxLayout *list_move_lay = new QVBoxLayout;
    list_move_lay->addWidget(move_list_check);
    list_move_lay->addWidget(move_pile_check);
    list_move_lay->addStretch();
    list_move_lay->addWidget(move_list_up_button);
    list_move_lay->addWidget(move_list_down_button);
    QHBoxLayout *list_lay = new QHBoxLayout;
    list_lay->addWidget(list);
    list_lay->addLayout(list_move_lay);
    info_lay->addLayout(list_lay);
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
    connect(move_list_up_button, SIGNAL(clicked()), this, SLOT(exchangeListItem()));
    connect(move_list_down_button, SIGNAL(clicked()), this, SLOT(exchangeListItem()));
    connect(move_list_check, SIGNAL(toggled(bool)), this, SLOT(setMoveButtonAvaliable(bool)));
    connect(move_pile_check, SIGNAL(toggled(bool)), this, SLOT(setMoveButtonAvaliable(bool)));
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
    connect(choose_nationality, SIGNAL(toggled(bool)), nationalities, SLOT(setEnabled(bool)));
    connect(choose_nationality, SIGNAL(toggled(bool)), this, SLOT(setNationalityEnable(bool)));
    connect(nationalities, SIGNAL(currentIndexChanged(int)), this, SLOT(setNationality(int)));
    connect(random_roles_box, SIGNAL(toggled(bool)), this, SLOT(updateAllRoles(bool)));
    connect(extra_skill_set, SIGNAL(clicked()), this, SLOT(doSkillSelect()));
    connect(hp_spin, SIGNAL(valueChanged(int)), this, SLOT(getPlayerHp(int)));
    connect(max_hp_spin, SIGNAL(valueChanged(int)), this, SLOT(getPlayerMaxHp(int)));
    connect(player_draw, SIGNAL(valueChanged(int)), this, SLOT(setPlayerStartDraw(int)));
    connect(starter_box, SIGNAL(toggled(bool)), this, SLOT(setStarter(bool)));
    connect(marks_count, SIGNAL(valueChanged(int)), this, SLOT(setPlayerMarks(int)));
    connect(marks_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(getPlayerMarks(int)));
    connect(pile_list, SIGNAL(currentRowChanged(int)), this, SLOT(updatePileInfo(int)));
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
    connect(defaultLoadButton, SIGNAL(clicked()), this, SLOT(load()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void CustomAssignDialog::exchangePlayersInfo(QListWidgetItem *first, QListWidgetItem *second){
    QString first_name = first->data(Qt::UserRole).toString();
    QString second_name = second->data(Qt::UserRole).toString();

    QString role = role_mapping[first_name], general = general_mapping[first_name],
            general2 = general2_mapping[first_name];
    QList<int> judges(player_judges[first_name]), equips(player_equips[first_name]), hands(player_handcards[first_name]);
    int hp = player_hp[first_name], maxhp = player_maxhp[first_name], start_draw = player_start_draw[first_name];
    bool turned = player_turned[first_name], chained = player_chained[first_name],
            free_general = free_choose_general[first_name], free_general2 = free_choose_general2[first_name];
    QStringList ex_skills(player_exskills[first_name]);
    QMap<QString, int> marks(player_marks[first_name]);
    bool setting_nationality = set_nationality.value(first_name, false);
    QString assigned_nationality = assign_nationality.value(first_name, "");

    role_mapping[first_name] = role_mapping[second_name];
    general_mapping[first_name] = general_mapping[second_name];
    general2_mapping[first_name] = general2_mapping[second_name];
    player_judges[first_name].clear();
    player_judges[first_name].append(player_judges[second_name]);
    player_equips[first_name].clear();
    player_equips[first_name].append(player_equips[second_name]);
    player_handcards[first_name].clear();
    player_handcards[first_name].append(player_handcards[second_name]);
    player_hp[first_name] = player_hp[second_name];
    player_maxhp[first_name] = player_maxhp[second_name];
    player_start_draw[first_name] = player_start_draw[second_name];
    player_turned[first_name] = player_turned[second_name];
    player_chained[first_name] = player_chained[second_name];
    free_choose_general[first_name] = free_choose_general[second_name];
    free_choose_general2[first_name] = free_choose_general2[second_name];
    player_exskills[first_name].clear();
    player_exskills[first_name].append(player_exskills[second_name]);
    player_marks[first_name].clear();
    player_marks[first_name] = player_marks[second_name];
    set_nationality[first_name] = set_nationality[second_name];
    assign_nationality[first_name] = set_nationality[second_name];

    role_mapping[second_name] = role;
    general_mapping[second_name] = general;
    general2_mapping[second_name] = general2;
    player_judges[second_name].clear();
    player_judges[second_name].append(judges);
    player_equips[second_name].clear();
    player_equips[second_name].append(equips);
    player_handcards[second_name].clear();
    player_handcards[second_name].append(hands);
    player_hp[second_name] = hp;
    player_maxhp[second_name] = maxhp;
    player_start_draw[second_name] = start_draw;
    player_turned[second_name] = turned;
    player_chained[second_name] = chained;
    free_choose_general[second_name] = free_general;
    free_choose_general2[second_name] = free_general2;
    player_exskills[second_name].clear();
    player_exskills[second_name].append(ex_skills);
    player_marks[second_name].clear();
    player_marks[second_name] = marks;
    set_nationality[second_name] = setting_nationality;
    assign_nationality[second_name] = assigned_nationality;
}

QString CustomAssignDialog::setListText(QString name, QString role, int index){
    QString text = is_random_roles ? QString("[%1]").arg(Sanguosha->translate(role)) :
                      QString("%1[%2]").arg(Sanguosha->translate(name)).arg(Sanguosha->translate(role));

    if(index >= 0)
        list->item(index)->setText(text);

    return text;
}

void CustomAssignDialog::updateListItems(){
    for(int i = 0; i <= 9; i++){
        QString name = (i == 0 ? "Player" : "AI");
        if(i != 0)
            name.append(QString::number(i));

        if(role_mapping[name].isEmpty()) role_mapping[name] = "unknown";
        QListWidgetItem *item = new QListWidgetItem(setListText(name, role_mapping[name]));
        item->setData(Qt::UserRole, name);
        item_map[i] = item;
    }
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
        }
    }
    else{
        for(int i= list->count(); i< count; i++){
            list->addItem(item_map[i]);
        }
    }
}

void CustomAssignDialog::setNationalityEnable(bool toggled){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    set_nationality[name] = toggled;
    assign_nationality[name] = nationalities->itemData(nationalities->currentIndex()).toString();
}

void CustomAssignDialog::setNationality(int index){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    assign_nationality[name] = nationalities->itemData(index).toString();
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

    int i = 0;
    for(i = 0; i < mark_icons.length(); i++)
        mark_icons.at(i)->hide();

    foreach(QString mark, player_marks[name].keys()){
        if(player_marks[name][mark] > 0){
            for(i = 0; i < mark_icons.length(); i++){
                if(mark_icons.at(i)->objectName() == mark){
                    mark_icons.at(i)->show();
                    break;
                }
            }
        }
    }
}

void CustomAssignDialog::updatePileInfo(int row){
    if(row >= 0){
        if(move_pile_check->isChecked()){
            move_list_up_button->setEnabled(row != 0);
            move_list_down_button->setEnabled(row != pile_list->count()-1);
        }
        return;
    }

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

void CustomAssignDialog::updateAllRoles(bool toggled){
    is_random_roles = toggled;

    int i = 0;
    for(i = 0; i < list->count(); i++){
        QString name = player_mapping[i];
        QString role = role_mapping[name];
        item_map[i]->setText(setListText(name, role, i));
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
    hp_spin->setRange(1, maxhp);
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
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    player_start_draw[name] = draw_num;
}

void CustomAssignDialog::setStarter(bool toggled){
    if(toggled)
        starter = list->currentItem()->data(Qt::UserRole).toString();
    else
        starter.clear();
}

void CustomAssignDialog::setPlayerMarks(int value){
    QString mark_name = marks_combobox->itemData(marks_combobox->currentIndex()).toString();
    QString player_name = list->item(list->currentRow())->data(Qt::UserRole).toString();
    player_marks[player_name][mark_name] = value;

    for(int i = 0; i < mark_icons.length(); i++){
        if(mark_icons.at(i)->objectName() == mark_name){
            if(value > 0)
                mark_icons.at(i)->show();
            else
                mark_icons.at(i)->hide();

            break;
        }
    }
}

void CustomAssignDialog::getPlayerMarks(int index){
    QString mark_name = marks_combobox->itemData(index).toString();
    QString player_name = list->item(list->currentRow())->data(Qt::UserRole).toString();
    if(mark_name.isEmpty())
        marks_count->setEnabled(false);
    else
        marks_count->setEnabled(true);

    marks_count->setValue(player_marks[player_name][mark_name]);
}

void CustomAssignDialog::updateRole(int index){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    QString role = role_combobox->itemData(index).toString();
    setListText(name, role, list->currentRow());
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

void CustomAssignDialog::setMoveButtonAvaliable(bool toggled){
    if(sender()->objectName() == "list check"){
        move_pile_check->setChecked(false);
        move_list_check->setChecked(toggled);
        if(toggled){
            move_list_up_button->setEnabled(list->currentRow() != 0);
            move_list_down_button->setEnabled(list->currentRow() != list->count()-1);
        }
    }
    else{
        move_list_check->setChecked(false);
        move_pile_check->setChecked(toggled);
        if(toggled){
            move_list_up_button->setEnabled(pile_list->count() > 0 && pile_list->currentRow() != 0);
            move_list_down_button->setEnabled(pile_list->count() > 0 && pile_list->currentRow() != pile_list->count()-1);
        }
    }

    if(!move_list_check->isChecked() && !move_pile_check->isChecked()){
        move_list_up_button->setEnabled(false);
        move_list_down_button->setEnabled(false);
    }
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
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    if(toggled)
        free_choose_general[name] = true;
    else
        free_choose_general[name] = false;
}

void CustomAssignDialog::freeChoose2(bool toggled){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    if(toggled)
        free_choose_general2[name] = true;
    else
        free_choose_general2[name] = false;
}

void CustomAssignDialog::doPlayerChains(bool toggled){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    player_chained[name] = toggled;
}

void CustomAssignDialog::doPlayerTurns(bool toggled){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    player_turned[name] = toggled;
}

void CustomAssignDialog::doSkillSelect(){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    SkillAssignDialog *dialog = new SkillAssignDialog(this, name, player_exskills[name]);

    connect(dialog, SIGNAL(skill_update(QStringList)), this, SLOT(updatePlayerExSkills(QStringList)));
    dialog->exec();
}

void CustomAssignDialog::updatePlayerExSkills(QStringList update_skills){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    player_exskills[name].clear();
    player_exskills[name].append(update_skills);
}

void CustomAssignDialog::exchangeListItem(){
    int first_index = -1, second_index = -1;
    if(move_list_check->isChecked()) first_index = list->currentRow();
    else if(move_pile_check->isChecked()) first_index = pile_list->currentRow();

    if(sender()->objectName() == "list_up") second_index = first_index-1;
    else if(sender()->objectName() == "list_down") second_index = first_index+1;

    if(first_index < 0 && second_index < 0) return;

    if(move_list_check->isChecked()){
        exchangePlayersInfo(item_map[first_index], item_map[second_index]);
        updateListItems();
        int row = list->count();
        list->clear();
        for(int i = 0; i < row; i ++){
            list->addItem(item_map[i]);
        }
        list->setCurrentRow(second_index);
    }
    else if(move_pile_check->isChecked()){
        int id1 = pile_list->item(first_index)->data(Qt::UserRole).toInt();
        int id2 = pile_list->item(second_index)->data(Qt::UserRole).toInt();

        set_pile.swap(set_pile.indexOf(id1), set_pile.indexOf(id2));
        updatePileInfo();
        pile_list->setCurrentRow(second_index);
    }
}

void CustomAssignDialog::on_list_itemSelectionChanged(QListWidgetItem *current){
    if(list->count() == 0 || current == NULL) return;

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

    self_select_general->setChecked(free_choose_general[player_name]);
    self_select_general2->setChecked(free_choose_general2[player_name]);

    set_turned->setChecked(player_turned.value(player_name, false));
    set_chained->setChecked(player_chained.value(player_name, false));

    single_turn->setChecked(is_single_turn);
    before_next->setChecked(is_before_next);

    if(move_list_check->isChecked()){
        move_list_up_button->setEnabled(list->currentRow() != 0);
        move_list_down_button->setEnabled(list->currentRow() != list->count()-1);
    }

    int val = 4;
    if(player_start_draw.keys().contains(player_name)) val = player_start_draw[player_name];
    player_draw->setValue(val);

    if(!starter.isEmpty() && starter != player_name)
        starter_box->setEnabled(false);
    else
        starter_box->setEnabled(true);

    QString kingdom = assign_nationality.value(player_name, "");
    if(!kingdom.isEmpty())
        nationalities->setCurrentIndex(kingdom_index[kingdom]);

    choose_nationality->setChecked(set_nationality.value(player_name, false));

    QString mark_name = marks_combobox->itemData(marks_combobox->currentIndex()).toString();
    if(!mark_name.isEmpty())
        marks_count->setValue(player_marks.value(player_name)[mark_name]);
    else
        marks_count->setValue(0);

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
    QString filename;
    if(sender()->objectName() == "default_load") filename = "etc/customScenes/custom_scenario.txt";
    else filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open mini scenario settings"),
                                                    "etc/customScenes",
                                                    tr("Pure text replay file (*.txt)"));


    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    set_pile.clear();
    item_map.clear();
    role_mapping.clear();
    general_mapping.clear();
    general2_mapping.clear();
    player_maxhp.clear();
    player_hp.clear();
    player_start_draw.clear();
    player_chained.clear();
    player_turned.clear();
    player_marks.clear();
    player_exskills.clear();
    player_handcards.clear();
    player_equips.clear();
    player_judges.clear();
    set_nationality.clear();
    assign_nationality.clear();

    free_choose_general.clear();
    free_choose_general2.clear();

    is_single_turn = false;
    is_before_next = false;
    is_random_roles = false;

    int i = 0;
    for(i = 0; i < mark_icons.length(); i++)
        mark_icons.at(i)->hide();

    QTextStream in(&file);
    int numPlayer = 0;
    QMap<QString, int> role_index;
    role_index["lord+loyalist"] = 0;
    role_index["renegade"] = 1;
    role_index["rebel"] = 2;

    while (!in.atEnd()) {
        QString line = in.readLine();
        line = line.trimmed();
        if(line.isEmpty()) continue;

        if(!line.startsWith("setPile:") && !line.startsWith("extraOptions:") && !line.startsWith("general:")){
            QMessageBox::warning(this, tr("Warning"), tr("Data is unreadable"));
            file.close();
            return;
        }

        if(line.startsWith("setPile:"))
        {
            QStringList list = line.replace("setPile:","").split(",");
            foreach(QString id,list)
            {
                set_pile.prepend(id.toInt());
            }
            continue;
        }
        else if(line.startsWith("extraOptions:")){
            line.remove("extraOptions:");

            QMap<QString, QString> option_map;
            foreach(QString option, line.split(" ")){
                if(option.isEmpty()) continue;

                option_map[option.split(":").first()] = option.split(":").last();
            }

            if(option_map["randomRoles"] == "true") is_random_roles = true;

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

        if(player["general"]=="select")free_choose_general[name] = true;
        else if(player["general"]!=NULL)general_mapping[name]=player["general"];

        if(player["general2"]=="select")free_choose_general2[name] = true;
        else if(player["general2"]!=NULL)general2_mapping[name]=player["general2"];

        if(player["maxhp"]!=NULL){
            player_maxhp[name] = player["maxhp"].toInt();
            if(player_hp[name]>player_maxhp[name]) player_hp[name] = player_maxhp[name];
        }
        if(player["hp"]!=NULL)player_hp[name]=player["hp"].toInt();
        if(player["draw"]!=NULL)player_start_draw[name]=player["draw"].toInt();
        else player_start_draw[name] = 4;

        if(player["starter"]!=NULL)starter = name;
        if(player["chained"]!=NULL)player_chained[name]=true;
        if(player["turned"]!=NULL)player_turned[name]=true;
        if(player["nationality"] != NULL){
            assign_nationality[name] = player["nationality"];
            set_nationality[name] = true;
        }
        else{
            set_nationality[name] = false;
        }
        if(player["acquireSkills"] != NULL){
            QStringList skills;
            foreach(QString skill_name, player["acquireSkills"].split(","))
                skills << skill_name;

            player_exskills[name].append(skills);
        }
        if(player["singleTurn"] != NULL){
            single_turn_box->setCurrentIndex(role_index.value(player["singleTurn"], 0));
            is_single_turn = true;
        }
        if(player["beforeNext"] != NULL){
            before_next_box->setCurrentIndex(role_index.value(player["beforeNext"], 0));
            is_before_next = true;
        }
        if(player["marks"] != NULL){
            foreach(QString mark, player["marks"].split(",")){
                QString mark_name = mark.split("*").at(0);
                int mark_number = mark.split("*").at(1).toInt();

                player_marks[name][mark_name] = mark_number;
            }
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

        updateListItems();
        numPlayer++;
    }

    updateNumber(numPlayer-2);
    for(int i=list->count()-1;i>=0;i--)
    {
        list->setCurrentItem(list->item(i));
        if(list->item(i)->data(Qt::UserRole).toString() == starter)
            starter_box->setChecked(true);
    }
    list->setCurrentRow(0);

    player_draw->setValue(player_start_draw[list->currentItem()->data(Qt::UserRole).toString()]);
    num_combobox->setCurrentIndex(list->count()-2);
    random_roles_box->setChecked(is_random_roles);

    updatePileInfo();
    file.close();
}

bool CustomAssignDialog::save(QString path)
{
    if(starter.isEmpty()){
        QMessageBox::warning(NULL, tr("Warning"), tr("There is not a starter"));
        return false;
    }

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

    if(!has_diff_roles){
        QMessageBox::warning(this, tr("Warning"), tr("No different camps in the game"));
        return false;
    }

    QString line;

    set_options << is_random_roles;
    foreach(bool option, set_options){
        if(option){
            line.append("extraOptions:");
            break;
        }
    }
    if(is_random_roles) line.append("randomRoles:true ");
    line.remove(line.length()-1, 1);
    line.append("\n");

    if(set_pile.length())
    {
        line.append("setPile:");
        for(int i = set_pile.length()-1; i >= 0; i--)
        {
            int id = set_pile.at(i);
            line.append(QString::number(id));
            line.append(",");
        }
        line.remove(line.length()-1, 1);
        line.append("\n");
    }

    for(int i=0;i<list->count();i++)
    {
        QString name = i==0 ? "Player" : QString("AI%1").arg(i);

        if(free_choose_general[name])line.append("general:select ");
        else if(general_mapping[name].isEmpty()){
            QMessageBox::warning(this, tr("Warning"), tr("%1's general cannot be empty").arg(Sanguosha->translate(name)));
            return false;
        }
        else
            line.append(QString("general:%1 ").arg(general_mapping[name]));

        if(free_choose_general2[name])line.append("general2:select ");
        else if(!general2_mapping[name].isEmpty())line.append(QString("general2:%1 ").arg(general2_mapping[name]));

        if(role_mapping[name] == "unknown"){
            QMessageBox::warning(this, tr("Warning"), tr("%1's role cannot be unknown").arg(Sanguosha->translate(name)));
            return false;
        }
        line.append(QString("role:%1 ").arg(role_mapping[name]));
        if(starter == name)line.append("starter:true ");
        if(!player_marks[name].isEmpty()){
            line.append("marks:");
            QMap<QString, int> marks = player_marks[name];
            foreach(QString mark_name, marks.keys()){
                if(marks.value(mark_name) > 0)
                    line.append(QString("%1*%2,").arg(mark_name).arg(QString::number(marks.value(mark_name))));
            }

            if(line.endsWith("marks:"))
                line.remove(line.length()-7, 6);
            else{
                line.remove(line.length()-1, 1);
                line.append(" ");
            }
        }
        if(player_maxhp[name]>0)line.append(QString("maxhp:%1 ").arg(player_maxhp[name]));
        if(player_hp[name]>0) line.append(QString("hp:%1 ").arg(player_hp[name]));
        if(player_turned[name]) line.append("turned:true ");
        if(player_chained[name]) line.append("chained:true ");
        if(set_nationality[name]) line.append(QString("nationality:%1 ").arg(assign_nationality[name]));
        if(player_exskills[name].length() > 0){
            line.append("acquireSkills:");
            foreach(QString skill_name, player_exskills[name]){
                line.append(skill_name + ",");
            }
            line.remove(line.length()-1, 1);
            line.append(" ");
        }
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

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(card_list);
    layout->addLayout(vlayout);
    QVBoxLayout *mainlayout = new QVBoxLayout;
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

//-----------------------------------

SkillAssignDialog::SkillAssignDialog(QDialog *parent, QString player_name, QStringList &player_skills)
    :QDialog(parent), update_skills(player_skills)
{
    setWindowTitle(tr("Skill Chosen"));
    QHBoxLayout *layout = new QHBoxLayout;
    skill_list = new QListWidget;

    input_skill = new QLineEdit;
    #if QT_VERSION >= 0x040700
    input_skill->setPlaceholderText(tr("Input the Skill Name"));
    #endif
    input_skill->setToolTip(tr("Internal skill name is a phonetic form, "
                               "the rest of the special circumstances, "
                               "please see the translation of documents in the lang directory."));

    QCompleter *completer = new QCompleter(Sanguosha->getSkillNames(), input_skill);
    input_skill->setCompleter(completer);

    QPushButton *add_skill = new QPushButton(tr("Add Skill"));
    add_skill->setObjectName("inline_add");

    select_skill = new QPushButton(tr("Select Skill from Generals"));
    delete_skill = new QPushButton(tr("Delete Current Skill"));

    QPushButton *ok_button = new QPushButton(tr("OK"));
    QPushButton *cancel_button = new QPushButton(tr("Cancel"));

    skill_info = new QTextEdit;
    skill_info->setReadOnly(true);

    updateSkillList();

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(new QLabel(Sanguosha->translate(player_name)));
    vlayout->addWidget(skill_list);
    layout->addLayout(vlayout);
    QVBoxLayout *sided_lay = new QVBoxLayout;
    sided_lay->addWidget(skill_info);
    sided_lay->addStretch();
    sided_lay->addLayout(HLay(input_skill, add_skill));
    sided_lay->addLayout(HLay(select_skill, delete_skill));
    sided_lay->addLayout(HLay(ok_button, cancel_button));
    layout->addLayout(sided_lay);
    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->addLayout(layout);
    setLayout(mainlayout);

    connect(add_skill, SIGNAL(clicked()), this, SLOT(addSkill()));
    connect(select_skill, SIGNAL(clicked()), this, SLOT(selectSkill()));
    connect(delete_skill, SIGNAL(clicked()), this, SLOT(deleteSkill()));
    connect(skill_list, SIGNAL(itemSelectionChanged()), this, SLOT(changeSkillInfo()));
    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
}

void SkillAssignDialog::changeSkillInfo(){
    QString skill_name = skill_list->currentItem()->data(Qt::UserRole).toString();
    skill_info->clear();

    skill_info->setText(Sanguosha->translate(":" + skill_name));
}

void SkillAssignDialog::selectSkill(){
    GeneralAssignDialog *dialog = new GeneralAssignDialog(this);

    connect(dialog, SIGNAL(general_chosen(QString)), this, SLOT(getSkillFromGeneral(QString)));
    dialog->exec();
}

void SkillAssignDialog::deleteSkill(){
    QString skill_name = skill_list->currentItem()->data(Qt::UserRole).toString();
    update_skills.removeOne(skill_name);

    updateSkillList();
}

void SkillAssignDialog::getSkillFromGeneral(QString general_name){
    QDialog *select_dialog = new QDialog(this);
    select_dialog->setWindowTitle(tr("Skill Chosen"));
    QVBoxLayout *layout = new QVBoxLayout;

    const General *general = Sanguosha->getGeneral(general_name);
    foreach(const Skill *skill, general->getVisibleSkillList()){
        QCommandLinkButton *button = new QCommandLinkButton;
        button->setObjectName(skill->objectName());
        button->setText(Sanguosha->translate(skill->objectName()));
        button->setToolTip(Sanguosha->translate(":" + skill->objectName()));

        connect(button, SIGNAL(clicked()), select_dialog, SLOT(accept()));
        connect(button, SIGNAL(clicked()), this, SLOT(addSkill()));

        layout->addWidget(button);
    }

    select_dialog->setLayout(layout);
    select_dialog->exec();
}

void SkillAssignDialog::addSkill(){
    QString name = sender()->objectName();
    if(name == "inline_add"){
        name = input_skill->text();

        const Skill *skill = Sanguosha->getSkill(name);
        if(skill == NULL){
            QMessageBox::warning(this, tr("Warning"), tr("There is no skill that internal name is %1").arg(name));
            return;
        }
    }

    if(!update_skills.contains(name)){
        update_skills << name;
        updateSkillList();
    }

    input_skill->clear();
}

void SkillAssignDialog::updateSkillList(){
    int index = skill_list->count() > 0 ? skill_list->currentRow() : 0;

    skill_list->clear();
    skill_info->clear();

    foreach(QString skill_name, update_skills){
        if(Sanguosha->getSkill(skill_name) != NULL){
            QListWidgetItem *item = new QListWidgetItem(Sanguosha->translate(skill_name));
            item->setData(Qt::UserRole, skill_name);
            skill_list->addItem(item);
        }
    }
    skill_list->setCurrentRow(index >= skill_list->count() ? skill_list->count()-1 : index);

    if(skill_list->count() > 0){
        changeSkillInfo();
        delete_skill->setEnabled(true);
    }
    else delete_skill->setEnabled(false);
}

void SkillAssignDialog::accept(){
    emit skill_update(update_skills);

    QDialog::accept();
}
