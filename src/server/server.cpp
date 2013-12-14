#include "server.h"
#include "settings.h"
#include "room.h"
#include "engine.h"
#include "nativesocket.h"
#include "banpair.h"
#include "scenario.h"
#include "choosegeneraldialog.h"
#include "customassigndialog.h"
#include "miniscenarios.h"
#include "SkinBank.h"

#include <QMessageBox>
#include <QFormLayout>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QApplication>
#include <QHostInfo>
#include <QAction>

static QLayout *HLay(QWidget *left, QWidget *right) {
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(left);
    layout->addWidget(right);
    return layout;
}

ServerDialog::ServerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Start server"));

    QTabWidget *tab_widget = new QTabWidget;
    tab_widget->addTab(createBasicTab(), tr("Basic"));
    tab_widget->addTab(createPackageTab(), tr("Game Pacakge Selection"));
    tab_widget->addTab(createAdvancedTab(), tr("Advanced"));
    tab_widget->addTab(createMiscTab(), tr("Miscellaneous"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tab_widget);
    layout->addLayout(createButtonLayout());
    setLayout(layout);

    setMinimumWidth(300);
}

QWidget *ServerDialog::createBasicTab() {
    server_name_edit = new QLineEdit;
    server_name_edit->setText(Config.ServerName);

    timeout_spinbox = new QSpinBox;
    timeout_spinbox->setMinimum(5);
    timeout_spinbox->setMaximum(60);
    timeout_spinbox->setValue(Config.OperationTimeout);
    timeout_spinbox->setSuffix(tr(" seconds"));
    nolimit_checkbox = new QCheckBox(tr("No limit"));
    nolimit_checkbox->setChecked(Config.OperationNoLimit);
    connect(nolimit_checkbox, SIGNAL(toggled(bool)), timeout_spinbox, SLOT(setDisabled(bool)));

    // add 1v1 banlist edit button
    QPushButton *edit_button = new QPushButton(tr("Banlist ..."));
    edit_button->setFixedWidth(100);
    connect(edit_button, SIGNAL(clicked()), this, SLOT(edit1v1Banlist()));

    QFormLayout *form_layout = new QFormLayout;
    form_layout->addRow(tr("Server name"), server_name_edit);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->addWidget(timeout_spinbox);
    lay->addWidget(nolimit_checkbox);
    lay->addWidget(edit_button);
    form_layout->addRow(tr("Operation timeout"), lay);
    form_layout->addRow(createGameModeBox());

    QWidget *widget = new QWidget;
    widget->setLayout(form_layout);
    return widget;
}

QWidget *ServerDialog::createPackageTab() {
    disable_lua_checkbox = new QCheckBox(tr("Disable Lua"));
    disable_lua_checkbox->setChecked(Config.DisableLua);
    disable_lua_checkbox->setToolTip(tr("The setting takes effect after reboot"));

    extension_group = new QButtonGroup;
    extension_group->setExclusive(false);

    QStringList extensions = Sanguosha->getExtensions();
    QSet<QString> ban_packages = Config.BanPackages.toSet();

    QGroupBox *box1 = new QGroupBox(tr("General package"));
    QGroupBox *box2 = new QGroupBox(tr("Card package"));

    QGridLayout *layout1 = new QGridLayout;
    QGridLayout *layout2 = new QGridLayout;
    box1->setLayout(layout1);
    box2->setLayout(layout2);

    int i = 0, j = 0;
    int row = 0, column = 0;
    foreach (QString extension, extensions) {
        const Package *package = Sanguosha->findChild<const Package *>(extension);
        if (package == NULL)
            continue;

        bool forbid_package = Config.value("ForbidPackages").toStringList().contains(extension);
        QCheckBox *checkbox = new QCheckBox;
        checkbox->setObjectName(extension);
        checkbox->setText(Sanguosha->translate(extension));
        checkbox->setChecked(!ban_packages.contains(extension) && !forbid_package);
        checkbox->setEnabled(!forbid_package);

        extension_group->addButton(checkbox);

        switch (package->getType()) {
        case Package::GeneralPack: {
                row = i / 5;
                column = i % 5;
                i++;

                layout1->addWidget(checkbox, row, column + 1);
                break;
            }
        case Package::CardPack: {
                row = j / 5;
                column = j % 5;
                j++;

                layout2->addWidget(checkbox, row, column + 1);
                break;
            }
        default:
                break;
        }
    }

    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(disable_lua_checkbox);
    layout->addWidget(box1);
    layout->addWidget(box2);

    widget->setLayout(layout);
    return widget;
}

void ServerDialog::setMaxHpSchemeBox() {
    if (!second_general_checkbox->isChecked()) {
        prevent_awaken_below3_checkbox->setVisible(false);

        scheme0_subtraction_label->setVisible(false);
        scheme0_subtraction_spinbox->setVisible(false);

        return;
    }
    int index = max_hp_scheme_ComboBox->currentIndex();
    if (index == 0) {
        prevent_awaken_below3_checkbox->setVisible(false);

        scheme0_subtraction_label->setVisible(true);
        scheme0_subtraction_spinbox->setVisible(true);
        scheme0_subtraction_spinbox->setValue(Config.value("Scheme0Subtraction", 3).toInt());
        scheme0_subtraction_spinbox->setEnabled(true);
    } else {
        prevent_awaken_below3_checkbox->setVisible(true);
        prevent_awaken_below3_checkbox->setChecked(Config.value("PreventAwakenBelow3", false).toBool());
        prevent_awaken_below3_checkbox->setEnabled(true);

        scheme0_subtraction_label->setVisible(false);
        scheme0_subtraction_spinbox->setVisible(false);
    }
}

QWidget *ServerDialog::createAdvancedTab() {
    QVBoxLayout *layout = new QVBoxLayout;

    random_seat_checkbox = new QCheckBox(tr("Arrange the seats randomly"));
    random_seat_checkbox->setChecked(Config.RandomSeat);

    enable_cheat_checkbox = new QCheckBox(tr("Enable cheat"));
    enable_cheat_checkbox->setToolTip(tr("This option enables the cheat menu"));
    enable_cheat_checkbox->setChecked(Config.EnableCheat);

    free_choose_checkbox = new QCheckBox(tr("Choose generals and cards freely"));
    free_choose_checkbox->setChecked(Config.FreeChoose);
    free_choose_checkbox->setVisible(Config.EnableCheat);

    free_assign_checkbox = new QCheckBox(tr("Assign role and seat freely"));
    free_assign_checkbox->setChecked(Config.value("FreeAssign").toBool());
    free_assign_checkbox->setVisible(Config.EnableCheat);

    free_assign_self_checkbox = new QCheckBox(tr("Assign only your own role"));
    free_assign_self_checkbox->setChecked(Config.FreeAssignSelf);
    free_assign_self_checkbox->setEnabled(free_assign_checkbox->isChecked());
    free_assign_self_checkbox->setVisible(Config.EnableCheat);

    connect(enable_cheat_checkbox, SIGNAL(toggled(bool)), free_choose_checkbox, SLOT(setVisible(bool)));
    connect(enable_cheat_checkbox, SIGNAL(toggled(bool)), free_assign_checkbox, SLOT(setVisible(bool)));
    connect(enable_cheat_checkbox, SIGNAL(toggled(bool)), free_assign_self_checkbox, SLOT(setVisible(bool)));
    connect(free_assign_checkbox, SIGNAL(toggled(bool)), free_assign_self_checkbox, SLOT(setEnabled(bool)));

    pile_swapping_label = new QLabel(tr("Pile-swapping limitation"));
    pile_swapping_label->setToolTip(tr("-1 means no limitations"));
    pile_swapping_spinbox = new QSpinBox;
    pile_swapping_spinbox->setRange(-1, 15);
    pile_swapping_spinbox->setValue(Config.value("PileSwappingLimitation", 5).toInt());

    without_lordskill_checkbox = new QCheckBox(tr("Without Lordskill"));
    without_lordskill_checkbox->setChecked(Config.value("WithoutLordskill", false).toBool());

    sp_convert_checkbox = new QCheckBox(tr("Enable SP Convert"));
    sp_convert_checkbox->setChecked(Config.value("EnableSPConvert", true).toBool());

    maxchoice_spinbox = new QSpinBox;
    maxchoice_spinbox->setRange(3, 21);
    maxchoice_spinbox->setValue(Config.value("MaxChoice", 5).toInt());

    lord_maxchoice_label = new QLabel(tr("Upperlimit for lord"));
    lord_maxchoice_label->setToolTip(tr("-1 means that all lords are available"));
    lord_maxchoice_spinbox = new QSpinBox;
    lord_maxchoice_spinbox->setRange(-1, 15);
    lord_maxchoice_spinbox->setValue(Config.value("LordMaxChoice", -1).toInt());

    nonlord_maxchoice_spinbox = new QSpinBox;
    nonlord_maxchoice_spinbox->setRange(0, 15);
    nonlord_maxchoice_spinbox->setValue(Config.value("NonLordMaxChoice", 2).toInt());

    forbid_same_ip_checkbox = new QCheckBox(tr("Forbid same IP with multiple connection"));
    forbid_same_ip_checkbox->setChecked(Config.ForbidSIMC);

    disable_chat_checkbox = new QCheckBox(tr("Disable chat"));
    disable_chat_checkbox->setChecked(Config.DisableChat);

    second_general_checkbox = new QCheckBox(tr("Enable second general"));
    second_general_checkbox->setChecked(Config.Enable2ndGeneral);

    scene_checkbox  = new QCheckBox(tr("Enable Scene"));
    scene_checkbox->setChecked(Config.EnableScene);    //changjing

    same_checkbox = new QCheckBox(tr("Enable Same"));
    same_checkbox->setChecked(Config.EnableSame);

    max_hp_label = new QLabel(tr("Max HP scheme"));
    max_hp_scheme_ComboBox = new QComboBox;
    max_hp_scheme_ComboBox->addItem(tr("Sum - X"));
    max_hp_scheme_ComboBox->addItem(tr("Minimum"));
    max_hp_scheme_ComboBox->addItem(tr("Maximum"));
    max_hp_scheme_ComboBox->addItem(tr("Average"));
    max_hp_scheme_ComboBox->setCurrentIndex(Config.MaxHpScheme);

    prevent_awaken_below3_checkbox = new QCheckBox(tr("Prevent maxhp being less than 3 for awaken skills"));
    prevent_awaken_below3_checkbox->setChecked(Config.PreventAwakenBelow3);
    prevent_awaken_below3_checkbox->setEnabled(max_hp_scheme_ComboBox->currentIndex() != 0);

    scheme0_subtraction_label = new QLabel(tr("Subtraction for scheme 0"));
    scheme0_subtraction_label->setVisible(max_hp_scheme_ComboBox->currentIndex() == 0);
    scheme0_subtraction_spinbox = new QSpinBox;
    scheme0_subtraction_spinbox->setRange(-5, 12);
    scheme0_subtraction_spinbox->setValue(Config.Scheme0Subtraction);
    scheme0_subtraction_spinbox->setVisible(max_hp_scheme_ComboBox->currentIndex() == 0);

    connect(max_hp_scheme_ComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setMaxHpSchemeBox()));

    basara_checkbox = new QCheckBox(tr("Enable Basara"));
    basara_checkbox->setChecked(Config.EnableBasara);
    updateButtonEnablility(mode_group->checkedButton());
    connect(mode_group, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(updateButtonEnablility(QAbstractButton *)));

    hegemony_checkbox = new QCheckBox(tr("Enable Hegemony"));
    hegemony_checkbox->setChecked(Config.EnableBasara && Config.EnableHegemony);
    hegemony_checkbox->setEnabled(basara_checkbox->isChecked());
    connect(basara_checkbox, SIGNAL(toggled(bool)), hegemony_checkbox, SLOT(setChecked(bool)));
    connect(basara_checkbox, SIGNAL(toggled(bool)), hegemony_checkbox, SLOT(setEnabled(bool)));

    hegemony_maxchoice_label = new QLabel(tr("Upperlimit for hegemony"));
    hegemony_maxchoice_spinbox = new QSpinBox;
    hegemony_maxchoice_spinbox->setRange(5, 21);
    hegemony_maxchoice_spinbox->setValue(Config.value("HegemonyMaxChoice", 7).toInt());

    hegemony_maxshown_label = new QLabel(tr("Max shown num for hegemony"));
    hegemony_maxshown_spinbox = new QSpinBox;
    hegemony_maxshown_spinbox->setRange(1, 11);
    hegemony_maxshown_spinbox->setValue(Config.value("HegemonyMaxShown", 2).toInt());

    address_edit = new QLineEdit;
    address_edit->setText(Config.Address);
#if QT_VERSION >= 0x040700
    address_edit->setPlaceholderText(tr("Public IP or domain"));
#endif

    QPushButton *detect_button = new QPushButton(tr("Detect my WAN IP"));
    connect(detect_button, SIGNAL(clicked()), this, SLOT(onDetectButtonClicked()));

    port_edit = new QLineEdit;
    port_edit->setText(QString::number(Config.ServerPort));
    port_edit->setValidator(new QIntValidator(1, 9999, port_edit));

    layout->addWidget(forbid_same_ip_checkbox);
    layout->addWidget(disable_chat_checkbox);
    layout->addWidget(random_seat_checkbox);
    layout->addWidget(enable_cheat_checkbox);
    layout->addWidget(free_choose_checkbox);
    layout->addLayout(HLay(free_assign_checkbox, free_assign_self_checkbox));
    layout->addLayout(HLay(pile_swapping_label, pile_swapping_spinbox));
    layout->addLayout(HLay(without_lordskill_checkbox, sp_convert_checkbox));
    layout->addLayout(HLay(new QLabel(tr("Upperlimit for general")), maxchoice_spinbox));
    layout->addLayout(HLay(lord_maxchoice_label, lord_maxchoice_spinbox));
    layout->addLayout(HLay(new QLabel(tr("Upperlimit for non-lord")), nonlord_maxchoice_spinbox));
    layout->addWidget(second_general_checkbox);
    layout->addLayout(HLay(max_hp_label, max_hp_scheme_ComboBox));
    layout->addLayout(HLay(scheme0_subtraction_label, scheme0_subtraction_spinbox));
    layout->addWidget(prevent_awaken_below3_checkbox);
    layout->addLayout(HLay(basara_checkbox, hegemony_checkbox));
    layout->addLayout(HLay(hegemony_maxchoice_label, hegemony_maxchoice_spinbox));
    layout->addLayout(HLay(hegemony_maxshown_label, hegemony_maxshown_spinbox));
    layout->addWidget(same_checkbox);
    layout->addLayout(HLay(new QLabel(tr("Address")), address_edit));
    layout->addWidget(detect_button);
    layout->addLayout(HLay(new QLabel(tr("Port")), port_edit));
    layout->addStretch();

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    max_hp_label->setVisible(Config.Enable2ndGeneral);
    connect(second_general_checkbox, SIGNAL(toggled(bool)), max_hp_label, SLOT(setVisible(bool)));
    max_hp_scheme_ComboBox->setVisible(Config.Enable2ndGeneral);
    connect(second_general_checkbox, SIGNAL(toggled(bool)), max_hp_scheme_ComboBox, SLOT(setVisible(bool)));

    if (Config.Enable2ndGeneral) {
        prevent_awaken_below3_checkbox->setVisible(max_hp_scheme_ComboBox->currentIndex() != 0);
        scheme0_subtraction_label->setVisible(max_hp_scheme_ComboBox->currentIndex() == 0);
        scheme0_subtraction_spinbox->setVisible(max_hp_scheme_ComboBox->currentIndex() == 0);
    } else {
        prevent_awaken_below3_checkbox->setVisible(false);
        scheme0_subtraction_label->setVisible(false);
        scheme0_subtraction_spinbox->setVisible(false);
    }
    connect(second_general_checkbox, SIGNAL(toggled(bool)), this, SLOT(setMaxHpSchemeBox()));

    hegemony_maxchoice_label->setVisible(Config.EnableHegemony);
    connect(hegemony_checkbox, SIGNAL(toggled(bool)), hegemony_maxchoice_label, SLOT(setVisible(bool)));
    hegemony_maxchoice_spinbox->setVisible(Config.EnableHegemony);
    connect(hegemony_checkbox, SIGNAL(toggled(bool)), hegemony_maxchoice_spinbox, SLOT(setVisible(bool)));

    hegemony_maxshown_label->setVisible(Config.EnableHegemony);
    connect(hegemony_checkbox, SIGNAL(toggled(bool)), hegemony_maxshown_label, SLOT(setVisible(bool)));
    hegemony_maxshown_spinbox->setVisible(Config.EnableHegemony);
    connect(hegemony_checkbox, SIGNAL(toggled(bool)), hegemony_maxshown_spinbox, SLOT(setVisible(bool)));

    return widget;
}

QWidget *ServerDialog::createMiscTab() {
    game_start_spinbox = new QSpinBox;
    game_start_spinbox->setRange(0, 10);
    game_start_spinbox->setValue(Config.CountDownSeconds);
    game_start_spinbox->setSuffix(tr(" seconds"));

    nullification_spinbox = new QSpinBox;
    nullification_spinbox->setRange(5, 15);
    nullification_spinbox->setValue(Config.NullificationCountDown);
    nullification_spinbox->setSuffix(tr(" seconds"));

    minimize_dialog_checkbox = new QCheckBox(tr("Minimize the dialog when server runs"));
    minimize_dialog_checkbox->setChecked(Config.EnableMinimizeDialog);

    surrender_at_death_checkbox = new QCheckBox(tr("Surrender at the time of Death"));
    surrender_at_death_checkbox->setChecked(Config.SurrenderAtDeath);

    luck_card_checkbox = new QCheckBox(tr("Enable the luck card"));
    luck_card_checkbox->setChecked(Config.EnableLuckCard);

    QGroupBox *ai_groupbox = new QGroupBox(tr("Artificial intelligence"));
    ai_groupbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout *layout = new QVBoxLayout;

    ai_enable_checkbox = new QCheckBox(tr("Enable AI"));
    ai_enable_checkbox->setChecked(Config.EnableAI);
    ai_enable_checkbox->setEnabled(false); // Force to enable AI for disabling it causes crashes!!

    ai_delay_spinbox = new QSpinBox;
    ai_delay_spinbox->setMinimum(0);
    ai_delay_spinbox->setMaximum(5000);
    ai_delay_spinbox->setValue(Config.OriginAIDelay);
    ai_delay_spinbox->setSuffix(tr(" millisecond"));

    ai_delay_altered_checkbox = new QCheckBox(tr("Alter AI Delay After Death"));
    ai_delay_altered_checkbox->setChecked(Config.AlterAIDelayAD);

    ai_delay_ad_spinbox = new QSpinBox;
    ai_delay_ad_spinbox->setMinimum(0);
    ai_delay_ad_spinbox->setMaximum(5000);
    ai_delay_ad_spinbox->setValue(Config.AIDelayAD);
    ai_delay_ad_spinbox->setSuffix(tr(" millisecond"));
    ai_delay_ad_spinbox->setEnabled(ai_delay_altered_checkbox->isChecked());
    connect(ai_delay_altered_checkbox, SIGNAL(toggled(bool)), ai_delay_ad_spinbox, SLOT(setEnabled(bool)));

    layout->addWidget(ai_enable_checkbox);
    layout->addLayout(HLay(new QLabel(tr("AI delay")), ai_delay_spinbox));
    layout->addWidget(ai_delay_altered_checkbox);
    layout->addLayout(HLay(new QLabel(tr("AI delay After Death")), ai_delay_ad_spinbox));

    ai_groupbox->setLayout(layout);

    QVBoxLayout *tablayout = new QVBoxLayout;
    tablayout->addLayout(HLay(new QLabel(tr("Game start count down")), game_start_spinbox));
    tablayout->addLayout(HLay(new QLabel(tr("Nullification count down")), nullification_spinbox));
    tablayout->addWidget(minimize_dialog_checkbox);
    tablayout->addWidget(surrender_at_death_checkbox);
    tablayout->addWidget(luck_card_checkbox);
    tablayout->addWidget(ai_groupbox);
    tablayout->addStretch();

    QWidget *widget = new QWidget;
    widget->setLayout(tablayout);
    return widget;
}

void ServerDialog::ensureEnableAI() {
    ai_enable_checkbox->setChecked(true);
}

void ServerDialog::updateButtonEnablility(QAbstractButton *button) {
    if (!button) return;
    if (button->objectName().contains("scenario")
        || button->objectName().contains("mini")
        || button->objectName().contains("1v1")
        || button->objectName().contains("1v3")) {
        basara_checkbox->setChecked(false);
        basara_checkbox->setEnabled(false);
    } else {
        basara_checkbox->setEnabled(true);
    }

    if (button->objectName().contains("mini")) {
        mini_scene_button->setEnabled(true);
        second_general_checkbox->setChecked(false);
        second_general_checkbox->setEnabled(false);
    } else {
        second_general_checkbox->setEnabled(true);
        mini_scene_button->setEnabled(false);
    }
}

void BanlistDialog::switchTo(int item) {
    this->item = item;
    list = lists.at(item);
    if (add2nd) add2nd->setVisible((list->objectName()=="Pairs"));
}

BanlistDialog::BanlistDialog(QWidget *parent, bool view)
    : QDialog(parent), add2nd(NULL)
{
    setWindowTitle(tr("Select generals that are excluded"));

    if (ban_list.isEmpty())
        ban_list << "Roles" << "1v1" << "HulaoPass" << "XMode" << "Basara" << "Hegemony" << "Pairs";
    QVBoxLayout *layout = new QVBoxLayout;

    QTabWidget *tab = new QTabWidget;
    layout->addWidget(tab);
    connect(tab, SIGNAL(currentChanged(int)), this, SLOT(switchTo(int)));

    foreach (QString item, ban_list) {
        if (item == "Pairs") continue;
        QWidget *apage = new QWidget;

        list = new QListWidget;
        list->setObjectName(item);

        QStringList banlist = Config.value(QString("Banlist/%1").arg(item)).toStringList();
        foreach (QString name, banlist)
            addGeneral(name);

        lists << list;

        QVBoxLayout *vlay = new QVBoxLayout;
        vlay->addWidget(list);
        apage->setLayout(vlay);

        tab->addTab(apage, Sanguosha->translate(item));
    }

    QWidget *apage = new QWidget;

    list = new QListWidget;
    list->setObjectName("Pairs");
    this->list = list;
    foreach (QString banned, BanPair::getAllBanSet().toList())
        addGeneral(banned);
    foreach (QString banned, BanPair::getSecondBanSet().toList())
        add2ndGeneral(banned);
    foreach (BanPair pair, BanPair::getBanPairSet().toList())
        addPair(pair.first, pair.second);

    QVBoxLayout *vlay = new QVBoxLayout;
    vlay->addWidget(list);
    apage->setLayout(vlay);
    tab->addTab(apage, Sanguosha->translate("Pairs"));
    lists << list;

    QPushButton *add = new QPushButton(tr("Add ..."));
    QPushButton *remove = new QPushButton(tr("Remove"));
    if (!view) add2nd = new QPushButton(tr("Add 2nd general ..."));
    QPushButton *ok = new QPushButton(tr("OK"));

    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    connect(this, SIGNAL(accepted()), this, SLOT(saveAll()));
    connect(remove, SIGNAL(clicked()), this, SLOT(doRemoveButton()));
    connect(add, SIGNAL(clicked()), this, SLOT(doAddButton()));
    if (!view) connect(add2nd, SIGNAL(clicked()), this, SLOT(doAdd2ndButton()));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    if (!view) {
        hlayout->addWidget(add2nd);
        add2nd->hide();
        hlayout->addWidget(add);
        hlayout->addWidget(remove);
        list = lists.first();
    }

    hlayout->addWidget(ok);
    layout->addLayout(hlayout);

    setLayout(layout);

    foreach (QListWidget *alist, lists) {
        if (alist->objectName() == "Pairs") continue;
        alist->setViewMode(QListView::IconMode);
        alist->setDragDropMode(QListView::NoDragDrop);
    }
}

void BanlistDialog::addGeneral(const QString &name) {
    if (list->objectName() == "Pairs") {
        if (banned_items["Pairs"].contains(name)) return;
        banned_items["Pairs"].append(name);
        QString text = QString(tr("Banned for all: %1")).arg(Sanguosha->translate(name));
        QListWidgetItem *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, QVariant::fromValue(name));
        list->addItem(item);
    } else {
        foreach (QString general_name, name.split("+")) {
            if (banned_items[list->objectName()].contains(general_name)) continue;
            banned_items[list->objectName()].append(general_name);
            QIcon icon(G_ROOM_SKIN.getGeneralPixmap(general_name, QSanRoomSkin::S_GENERAL_ICON_SIZE_TINY));
            QString text = Sanguosha->translate(general_name);
            QListWidgetItem *item = new QListWidgetItem(icon, text, list);
            item->setSizeHint(QSize(60, 60));
            item->setData(Qt::UserRole, general_name);
        }
    }
}

void BanlistDialog::add2ndGeneral(const QString &name) {
    foreach (QString general_name, name.split("+")) {
        if (banned_items["Pairs"].contains("+" + general_name)) continue;
        banned_items["Pairs"].append("+" + general_name);
        QString text = QString(tr("Banned for second general: %1")).arg(Sanguosha->translate(general_name));
        QListWidgetItem *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, QVariant::fromValue(QString("+%1").arg(general_name)));
        list->addItem(item);
    }
}

void BanlistDialog::addPair(const QString &first, const QString &second) {
    if (banned_items["Pairs"].contains(QString("%1+%2").arg(first, second))
        || banned_items["Pairs"].contains(QString("%1+%2").arg(second, first))) return;
    banned_items["Pairs"].append(QString("%1+%2").arg(first, second));
    QString trfirst = Sanguosha->translate(first);
    QString trsecond = Sanguosha->translate(second);
    QListWidgetItem *item = new QListWidgetItem(QString("%1 + %2").arg(trfirst, trsecond));
    item->setData(Qt::UserRole, QVariant::fromValue(QString("%1+%2").arg(first, second)));
    list->addItem(item);
}

void BanlistDialog::doAddButton() {
    FreeChooseDialog *chooser = new FreeChooseDialog(this,
                                                     (list->objectName() == "Pairs") ? FreeChooseDialog::Pair : FreeChooseDialog::Multi);
    connect(chooser, SIGNAL(general_chosen(QString)), this, SLOT(addGeneral(QString)));
    connect(chooser, SIGNAL(pair_chosen(QString, QString)), this, SLOT(addPair(QString, QString)));
    chooser->exec();
}

void BanlistDialog::doAdd2ndButton() {
    FreeChooseDialog *chooser = new FreeChooseDialog(this, FreeChooseDialog::Multi);
    connect(chooser, SIGNAL(general_chosen(QString)), this, SLOT(add2ndGeneral(QString)));
    chooser->exec();
}

void BanlistDialog::doRemoveButton() {
    int row = list->currentRow();
    if (row != -1)
        delete list->takeItem(row);
}

void BanlistDialog::save() {
    QSet<QString> banset;

    for (int i = 0; i < list->count(); i++)
        banset << list->item(i)->data(Qt::UserRole).toString();

    QStringList banlist = banset.toList();
    Config.setValue(QString("Banlist/%1").arg(ban_list.at(item)), QVariant::fromValue(banlist));
}

void BanlistDialog::saveAll() {
    for (int i = 0; i < lists.length(); i++) {
        switchTo(i);
        save();
    }
    BanPair::loadBanPairs();
}

void ServerDialog::edit1v1Banlist() {
    BanlistDialog *dialog = new BanlistDialog(this);
    dialog->exec();
}

QGroupBox *ServerDialog::create1v1Box() {
    QGroupBox *box = new QGroupBox(tr("1v1 options"));
    box->setEnabled(Config.GameMode == "02_1v1");
    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout *vlayout = new QVBoxLayout;

    QComboBox *officialComboBox = new QComboBox;
    officialComboBox->addItem(tr("Classical"), "Classical");
    officialComboBox->addItem("2013", "2013");
    officialComboBox->addItem("OL", "OL");

    official_1v1_ComboBox = officialComboBox;

    QString rule = Config.value("1v1/Rule", "2013").toString();
    if (rule == "2013")
        officialComboBox->setCurrentIndex(1);
    else if (rule == "OL")
        officialComboBox->setCurrentIndex(2);

    kof_using_extension_checkbox = new QCheckBox(tr("General extensions"));
    kof_using_extension_checkbox->setChecked(Config.value("1v1/UsingExtension", false).toBool());

    kof_card_extension_checkbox = new QCheckBox(tr("Card extensions"));
    kof_card_extension_checkbox->setChecked(Config.value("1v1/UsingCardExtension", false).toBool());

    vlayout->addLayout(HLay(new QLabel(tr("Rule option")), official_1v1_ComboBox));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(new QLabel(tr("Extension setting")));
    hlayout->addStretch();
    hlayout->addWidget(kof_using_extension_checkbox);
    hlayout->addWidget(kof_card_extension_checkbox);

    vlayout->addLayout(hlayout);
    box->setLayout(vlayout);

    return box;
}

QGroupBox *ServerDialog::create3v3Box() {
    QGroupBox *box = new QGroupBox(tr("3v3 options"));
    box->setEnabled(Config.GameMode == "06_3v3");
    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout *vlayout = new QVBoxLayout;

    official_3v3_radiobutton = new QRadioButton(tr("Official mode"));

    QComboBox *officialComboBox = new QComboBox;
    officialComboBox->addItem(tr("Classical"), "Classical");
    officialComboBox->addItem("2012", "2012");
    officialComboBox->addItem("2013", "2013");

    official_3v3_ComboBox = officialComboBox;

    QString rule = Config.value("3v3/OfficialRule", "2013").toString();
    if (rule == "2012")
        officialComboBox->setCurrentIndex(1);
    else if (rule == "2013")
        officialComboBox->setCurrentIndex(2);

    QRadioButton *extend = new QRadioButton(tr("Extension mode"));
    QPushButton *extend_edit_button = new QPushButton(tr("General selection ..."));
    extend_edit_button->setEnabled(false);
    connect(extend, SIGNAL(toggled(bool)), extend_edit_button, SLOT(setEnabled(bool)));
    connect(extend_edit_button, SIGNAL(clicked()), this, SLOT(select3v3Generals()));

    exclude_disaster_checkbox = new QCheckBox(tr("Exclude disasters"));
    exclude_disaster_checkbox->setChecked(Config.value("3v3/ExcludeDisasters", true).toBool());

    QComboBox *roleChooseComboBox = new QComboBox;
    roleChooseComboBox->addItem(tr("Normal"), "Normal");
    roleChooseComboBox->addItem(tr("Random"), "Random");
    roleChooseComboBox->addItem(tr("All roles"), "AllRoles");

    role_choose_ComboBox = roleChooseComboBox;

    QString scheme = Config.value("3v3/RoleChoose", "Normal").toString();
    if (scheme == "Random")
        roleChooseComboBox->setCurrentIndex(1);
    else if (scheme == "AllRoles")
        roleChooseComboBox->setCurrentIndex(2);

    vlayout->addLayout(HLay(official_3v3_radiobutton, official_3v3_ComboBox));
    vlayout->addLayout(HLay(extend, extend_edit_button));
    vlayout->addWidget(exclude_disaster_checkbox);
    vlayout->addLayout(HLay(new QLabel(tr("Role choose")), role_choose_ComboBox));
    box->setLayout(vlayout);

    bool using_extension = Config.value("3v3/UsingExtension", false).toBool();
    if (using_extension)
        extend->setChecked(true);
    else
        official_3v3_radiobutton->setChecked(true);

    return box;
}

QGroupBox *ServerDialog::createXModeBox() {
    QGroupBox *box = new QGroupBox(tr("XMode options"));
    box->setEnabled(Config.GameMode == "06_XMode");
    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QComboBox *roleChooseComboBox = new QComboBox;
    roleChooseComboBox->addItem(tr("Normal"), "Normal");
    roleChooseComboBox->addItem(tr("Random"), "Random");
    roleChooseComboBox->addItem(tr("All roles"), "AllRoles");

    role_choose_xmode_ComboBox = roleChooseComboBox;

    QString scheme = Config.value("XMode/RoleChooseX", "Normal").toString();
    if (scheme == "Random")
        roleChooseComboBox->setCurrentIndex(1);
    else if (scheme == "AllRoles")
        roleChooseComboBox->setCurrentIndex(2);

    box->setLayout(HLay(new QLabel(tr("Role choose")), role_choose_xmode_ComboBox));
    return box;
}

QGroupBox *ServerDialog::createGameModeBox() {
    QGroupBox *mode_box = new QGroupBox(tr("Game mode"));
    mode_group = new QButtonGroup;

    QObjectList item_list;

    // normal modes
    QMap<QString, QString> modes = Sanguosha->getAvailableModes();
    QMapIterator<QString, QString> itor(modes);
    while (itor.hasNext()) {
        itor.next();

        QRadioButton *button = new QRadioButton(itor.value());
        button->setObjectName(itor.key());
        mode_group->addButton(button);

        if (itor.key() == "02_1v1") {
            QGroupBox *box = create1v1Box();
            connect(button, SIGNAL(toggled(bool)), box, SLOT(setEnabled(bool)));

            item_list << button << box;
        } else if (itor.key() == "06_3v3") {
            QGroupBox *box = create3v3Box();
            connect(button, SIGNAL(toggled(bool)), box, SLOT(setEnabled(bool)));

            item_list << button << box;
        } else if (itor.key() == "06_XMode") {
            QGroupBox *box = createXModeBox();
            connect(button, SIGNAL(toggled(bool)), box, SLOT(setEnabled(bool)));

            item_list << button << box;
        } else {
            item_list << button;
        }

        if (itor.key() == Config.GameMode)
            button->setChecked(true);
    }

    // add scenario modes
    QRadioButton *scenario_button = new QRadioButton(tr("Scenario mode"));
    scenario_button->setObjectName("scenario");
    mode_group->addButton(scenario_button);

    scenario_ComboBox = new QComboBox;
    QStringList names = Sanguosha->getModScenarioNames();
    foreach (QString name, names) {
        QString scenario_name = Sanguosha->translate(name);
        const Scenario *scenario = Sanguosha->getScenario(name);
        int count = scenario->getPlayerCount();
        QString text = tr("%1 (%2 persons)").arg(scenario_name).arg(count);
        scenario_ComboBox->addItem(text, name);
    }

    if (mode_group->checkedButton() == NULL) {
        int index = names.indexOf(Config.GameMode);
        if (index != -1) {
            scenario_button->setChecked(true);
            scenario_ComboBox->setCurrentIndex(index);
        }
    }

    //mini scenes
    QRadioButton *mini_scenes = new QRadioButton(tr("Mini Scenes"));
    mini_scenes->setObjectName("mini");
    mode_group->addButton(mini_scenes);

    mini_scene_ComboBox = new QComboBox;
    int index = -1;
    int stage = qMin(Sanguosha->getMiniSceneCounts(), Config.value("MiniSceneStage", 1).toInt());
        
    for (int i = 1; i <= stage; i++) {
        QString name = QString(MiniScene::S_KEY_MINISCENE).arg(QString::number(i));
        QString scenario_name = Sanguosha->translate(name);
        const Scenario *scenario = Sanguosha->getScenario(name);
        int count = scenario->getPlayerCount();
        QString text = tr("%1 (%2 persons)").arg(scenario_name).arg(count);
        mini_scene_ComboBox->addItem(text, name);

        if (name == Config.GameMode) index = i - 1;
    }

    if (index >= 0) {
        mini_scene_ComboBox->setCurrentIndex(index);
        mini_scenes->setChecked(true);
    } else if (Config.GameMode == "custom_scenario")
        mini_scenes->setChecked(true);

    mini_scene_button = new QPushButton(tr("Custom Mini Scene"));
    connect(mini_scene_button, SIGNAL(clicked()), this, SLOT(doCustomAssign()));

    mini_scene_button->setEnabled(mode_group->checkedButton() ?
                                      mode_group->checkedButton()->objectName() == "mini" :
                                      false);

    item_list << HLay(scenario_button, scenario_ComboBox);
    item_list << HLay(mini_scenes, mini_scene_ComboBox);
    item_list << HLay(mini_scenes, mini_scene_button);

    // ============

    QVBoxLayout *left = new QVBoxLayout;
    QVBoxLayout *right = new QVBoxLayout;

    for (int i = 0; i < item_list.length(); i++) {
        QObject *item = item_list.at(i);

        QVBoxLayout *side = i <= item_list.length() / 2 - 4 ? left : right;

        if (item->isWidgetType()) {
            QWidget *widget = qobject_cast<QWidget *>(item);
            side->addWidget(widget);
        } else {
            QLayout *item_layout = qobject_cast<QLayout *>(item);
            side->addLayout(item_layout);
        }
        if (i == item_list.length() / 2 - 4)
            side->addStretch();
    }

    right->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addLayout(left);
    layout->addLayout(right);

    mode_box->setLayout(layout);

    return mode_box;
}

QLayout *ServerDialog::createButtonLayout() {
    QHBoxLayout *button_layout = new QHBoxLayout;
    button_layout->addStretch();

    QPushButton *ok_button = new QPushButton(tr("OK"));
    QPushButton *cancel_button = new QPushButton(tr("Cancel"));

    button_layout->addWidget(ok_button);
    button_layout->addWidget(cancel_button);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(onOkButtonClicked()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

    return button_layout;
}

void ServerDialog::onDetectButtonClicked() {
    QHostInfo vHostInfo = QHostInfo::fromName(QHostInfo::localHostName());
    QList<QHostAddress> vAddressList = vHostInfo.addresses();
    foreach (QHostAddress address, vAddressList) {
        if (!address.isNull() && address != QHostAddress::LocalHost
            && address.protocol() ==  QAbstractSocket::IPv4Protocol) {
            address_edit->setText(address.toString());
            return;
        }
    }
}

void ServerDialog::onOkButtonClicked() {
    accept();
}

Select3v3GeneralDialog::Select3v3GeneralDialog(QDialog *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Select generals in extend 3v3 mode"));
    ex_generals = Config.value("3v3/ExtensionGenerals").toStringList().toSet();
    QVBoxLayout *layout = new QVBoxLayout;
    tab_widget = new QTabWidget;
    fillTabWidget();

    QPushButton *ok_button = new QPushButton(tr("OK"));
    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(ok_button);

    layout->addWidget(tab_widget);
    layout->addLayout(hlayout);
    setLayout(layout);
    setMinimumWidth(550);

    connect(this, SIGNAL(accepted()), this, SLOT(save3v3Generals()));
}

void Select3v3GeneralDialog::fillTabWidget() {
    QList<const Package *> packages = Sanguosha->findChildren<const Package *>();
    foreach (const Package *package, packages) {
        if (package->getType() == Package::GeneralPack) {
            QListWidget *list = new QListWidget;
            list->setViewMode(QListView::IconMode);
            list->setDragDropMode(QListView::NoDragDrop);
            fillListWidget(list, package);

            tab_widget->addTab(list, Sanguosha->translate(package->objectName()));
        }
    }
}

void Select3v3GeneralDialog::fillListWidget(QListWidget *list, const Package *pack) {
    QList<const General *> generals = pack->findChildren<const General *>();
    foreach (const General *general, generals) {
        if (Sanguosha->isGeneralHidden(general->objectName())) continue;

        QListWidgetItem *item = new QListWidgetItem(list);
        item->setData(Qt::UserRole, general->objectName());
        item->setIcon(QIcon(G_ROOM_SKIN.getGeneralPixmap(general->objectName(), QSanRoomSkin::S_GENERAL_ICON_SIZE_TINY)));

        bool checked = false;
        if (ex_generals.isEmpty()) {
            checked = (pack->objectName() == "standard" || pack->objectName() == "wind")
                      && general->objectName() != "yuji";
        } else
            checked = ex_generals.contains(general->objectName());

        if (checked)
            item->setCheckState(Qt::Checked);
        else
            item->setCheckState(Qt::Unchecked);
    }

    QAction *action = new QAction(tr("Check/Uncheck all"), list);
    list->addAction(action);
    list->setContextMenuPolicy(Qt::ActionsContextMenu);
    list->setResizeMode(QListView::Adjust);

    connect(action, SIGNAL(triggered()), this, SLOT(toggleCheck()));
}

void ServerDialog::doCustomAssign() {
    CustomAssignDialog *dialog = new CustomAssignDialog(this);

    connect(dialog, SIGNAL(scenario_changed()), this, SLOT(setMiniCheckBox()));
    dialog->exec();
}

void ServerDialog::setMiniCheckBox() {
    mini_scene_ComboBox->setEnabled(false);
}

void Select3v3GeneralDialog::toggleCheck() {
    QWidget *widget = tab_widget->currentWidget();
    QListWidget *list = qobject_cast<QListWidget *>(widget);

    if (list == NULL || list->item(0) == NULL) return;

    bool checked = list->item(0)->checkState() != Qt::Checked;

    for (int i = 0; i < list->count(); i++)
        list->item(i)->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

void Select3v3GeneralDialog::save3v3Generals() {
    ex_generals.clear();

    for (int i = 0; i < tab_widget->count(); i++) {
        QWidget *widget = tab_widget->widget(i);
        QListWidget *list = qobject_cast<QListWidget *>(widget);
        if (list) {
            for (int j = 0; j < list->count(); j++) {
                QListWidgetItem *item = list->item(j);
                if (item->checkState() == Qt::Checked)
                    ex_generals << item->data(Qt::UserRole).toString();
            }
        }
    }

    QStringList list = ex_generals.toList();
    QVariant data = QVariant::fromValue(list);
    Config.setValue("3v3/ExtensionGenerals", data);
}

void ServerDialog::select3v3Generals() {
    QDialog *dialog = new Select3v3GeneralDialog(this);
    dialog->exec();
}

bool ServerDialog::config() {
    exec();

    if (result() != Accepted)
        return false;

    Config.ServerName = server_name_edit->text();
    Config.OperationTimeout = timeout_spinbox->value();
    Config.OperationNoLimit = nolimit_checkbox->isChecked();
    Config.RandomSeat = random_seat_checkbox->isChecked();
    Config.EnableCheat = enable_cheat_checkbox->isChecked();
    Config.FreeChoose = Config.EnableCheat && free_choose_checkbox->isChecked();
    Config.FreeAssignSelf = Config.EnableCheat && free_assign_self_checkbox->isChecked() && free_assign_checkbox->isEnabled();
    Config.ForbidSIMC = forbid_same_ip_checkbox->isChecked();
    Config.DisableChat = disable_chat_checkbox->isChecked();
    Config.Enable2ndGeneral = second_general_checkbox->isChecked();
    Config.EnableScene = scene_checkbox->isChecked();        //changjing
    Config.EnableSame = same_checkbox->isChecked();
    Config.EnableBasara = basara_checkbox->isChecked() && basara_checkbox->isEnabled();
    Config.EnableHegemony = hegemony_checkbox->isChecked() && hegemony_checkbox->isEnabled();
    Config.MaxHpScheme = max_hp_scheme_ComboBox->currentIndex();
    if (Config.MaxHpScheme == 0) {
        Config.Scheme0Subtraction = scheme0_subtraction_spinbox->value();
        Config.PreventAwakenBelow3 = false;
    } else {
        Config.Scheme0Subtraction = 3;
        Config.PreventAwakenBelow3 = prevent_awaken_below3_checkbox->isChecked();
    }
    Config.Address = address_edit->text();
    Config.CountDownSeconds = game_start_spinbox->value();
    Config.NullificationCountDown = nullification_spinbox->value();
    Config.EnableMinimizeDialog = minimize_dialog_checkbox->isChecked();
    Config.EnableAI = ai_enable_checkbox->isChecked();
    Config.OriginAIDelay = ai_delay_spinbox->value();
    Config.AIDelay = Config.OriginAIDelay;
    Config.AIDelayAD = ai_delay_ad_spinbox->value();
    Config.AlterAIDelayAD = ai_delay_altered_checkbox->isChecked();
    Config.ServerPort = port_edit->text().toInt();
    Config.DisableLua = disable_lua_checkbox->isChecked();
    Config.SurrenderAtDeath = surrender_at_death_checkbox->isChecked();
    Config.EnableLuckCard = luck_card_checkbox->isChecked();

    // game mode
    QString objname = mode_group->checkedButton()->objectName();
    if (objname == "scenario")
        Config.GameMode = scenario_ComboBox->itemData(scenario_ComboBox->currentIndex()).toString();
    else if (objname == "mini") {
        if (mini_scene_ComboBox->isEnabled())
            Config.GameMode = mini_scene_ComboBox->itemData(mini_scene_ComboBox->currentIndex()).toString();
        else
            Config.GameMode = "custom_scenario";
    } else
        Config.GameMode = objname;

    Config.setValue("ServerName", Config.ServerName);
    Config.setValue("GameMode", Config.GameMode);
    Config.setValue("OperationTimeout", Config.OperationTimeout);
    Config.setValue("OperationNoLimit", Config.OperationNoLimit);
    Config.setValue("RandomSeat", Config.RandomSeat);
    Config.setValue("EnableCheat", Config.EnableCheat);
    Config.setValue("FreeChoose", Config.FreeChoose);
    Config.setValue("FreeAssign", Config.EnableCheat && free_assign_checkbox->isChecked());
    Config.setValue("FreeAssignSelf", Config.FreeAssignSelf);
    Config.setValue("PileSwappingLimitation", pile_swapping_spinbox->value());
    Config.setValue("WithoutLordskill", without_lordskill_checkbox->isChecked());
    Config.setValue("EnableSPConvert", sp_convert_checkbox->isChecked());
    Config.setValue("MaxChoice", maxchoice_spinbox->value());
    Config.setValue("LordMaxChoice", lord_maxchoice_spinbox->value());
    Config.setValue("NonLordMaxChoice", nonlord_maxchoice_spinbox->value());
    Config.setValue("ForbidSIMC", Config.ForbidSIMC);
    Config.setValue("DisableChat", Config.DisableChat);
    Config.setValue("Enable2ndGeneral", Config.Enable2ndGeneral);
    Config.setValue("EnableScene", Config.EnableScene);   //changjing
    Config.setValue("EnableSame", Config.EnableSame);
    Config.setValue("EnableBasara", Config.EnableBasara);
    Config.setValue("EnableHegemony", Config.EnableHegemony);
    Config.setValue("HegemonyMaxChoice", hegemony_maxchoice_spinbox->value());
    Config.setValue("HegemonyMaxShown", hegemony_maxshown_spinbox->value());
    Config.setValue("MaxHpScheme", Config.MaxHpScheme);
    Config.setValue("Scheme0Subtraction", Config.Scheme0Subtraction);
    Config.setValue("PreventAwakenBelow3", Config.PreventAwakenBelow3);
    Config.setValue("CountDownSeconds", game_start_spinbox->value());
    Config.setValue("NullificationCountDown", nullification_spinbox->value());
    Config.setValue("EnableMinimizeDialog", Config.EnableMinimizeDialog);
    Config.setValue("EnableAI", Config.EnableAI);
    Config.setValue("OriginAIDelay", Config.OriginAIDelay);
    Config.setValue("AlterAIDelayAD", ai_delay_altered_checkbox->isChecked());
    Config.setValue("AIDelayAD", Config.AIDelayAD);
    Config.setValue("SurrenderAtDeath", Config.SurrenderAtDeath);
    Config.setValue("EnableLuckCard", Config.EnableLuckCard);
    Config.setValue("ServerPort", Config.ServerPort);
    Config.setValue("Address", Config.Address);
    Config.setValue("DisableLua", disable_lua_checkbox->isChecked());

    Config.beginGroup("3v3");
    Config.setValue("UsingExtension", !official_3v3_radiobutton->isChecked());
    Config.setValue("RoleChoose", role_choose_ComboBox->itemData(role_choose_ComboBox->currentIndex()).toString());
    Config.setValue("ExcludeDisaster", exclude_disaster_checkbox->isChecked());
    Config.setValue("OfficialRule", official_3v3_ComboBox->itemData(official_3v3_ComboBox->currentIndex()).toString());
    Config.endGroup();

    Config.beginGroup("1v1");
    Config.setValue("Rule", official_1v1_ComboBox->itemData(official_1v1_ComboBox->currentIndex()).toString());
    Config.setValue("UsingExtension", kof_using_extension_checkbox->isChecked());
    Config.setValue("UsingCardExtension", kof_card_extension_checkbox->isChecked());
    Config.endGroup();

    Config.beginGroup("XMode");
    Config.setValue("RoleChooseX", role_choose_xmode_ComboBox->itemData(role_choose_xmode_ComboBox->currentIndex()).toString());
    Config.endGroup();

    QSet<QString> ban_packages;
    QList<QAbstractButton *> checkboxes = extension_group->buttons();
    foreach (QAbstractButton *checkbox, checkboxes) {
        if (!checkbox->isChecked()) {
            QString package_name = checkbox->objectName();
            Sanguosha->addBanPackage(package_name);
            ban_packages.insert(package_name);
        }
    }

    Config.BanPackages = ban_packages.toList();
    Config.setValue("BanPackages", Config.BanPackages);

    return true;
}

Server::Server(QObject *parent)
    : QObject(parent)
{
    server = new NativeServerSocket;
    server->setParent(this);

    //synchronize ServerInfo on the server side to avoid ambiguous usage of Config and ServerInfo
    ServerInfo.parse(Sanguosha->getSetupString());

    current = NULL;
    createNewRoom();

    connect(server, SIGNAL(new_connection(ClientSocket *)), this, SLOT(processNewConnection(ClientSocket *)));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
}

void Server::broadcast(const QString &msg) {
    QString to_sent = msg.toUtf8().toBase64();
    to_sent = ".:" + to_sent;
    foreach (Room *room, rooms)
        room->broadcastInvoke("speak", to_sent);
}

bool Server::listen() {
    return server->listen();
}

void Server::daemonize() {
    server->daemonize();
}

Room *Server::createNewRoom() {
    Room *new_room = new Room(this, Config.GameMode);
    current = new_room;
    rooms.insert(current);

    connect(current, SIGNAL(room_message(QString)), this, SIGNAL(server_message(QString)));
    connect(current, SIGNAL(game_over(QString)), this, SLOT(gameOver()));

    return current;
}

void Server::processNewConnection(ClientSocket *socket) {
    if (Config.ForbidSIMC) {
        QString addr = socket->peerAddress();
        if (addresses.contains(addr)) {
            socket->disconnectFromHost();
            emit server_message(tr("Forbid the connection of address %1").arg(addr));
            return;
        } else
            addresses.insert(addr);
    }

    connect(socket, SIGNAL(disconnected()), this, SLOT(cleanup()));
    socket->send("checkVersion " + Sanguosha->getVersion());
    socket->send("setup " + Sanguosha->getSetupString());
    emit server_message(tr("%1 connected").arg(socket->peerName()));

    connect(socket, SIGNAL(message_got(const char *)), this, SLOT(processRequest(const char *)));
}

static inline QString ConvertFromBase64(const QString &base64) {
    QByteArray data = QByteArray::fromBase64(base64.toAscii());
    return QString::fromUtf8(data);
}

void Server::processRequest(const char *request) {
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());
    socket->disconnect(this, SLOT(processRequest(const char *)));

    QRegExp rx("(signupr?) (.+):(.+)(:.+)?\n");
    if (!rx.exactMatch(request)) {
        emit server_message(tr("Invalid signup string: %1").arg(request));
        socket->send("warn INVALID_FORMAT");
        socket->disconnectFromHost();
        return;
    }

    QStringList texts = rx.capturedTexts();
    QString command = texts.at(1);
    QString screen_name = ConvertFromBase64(texts.at(2));
    QString avatar = texts.at(3);

    if (command == "signupr") {
        foreach (QString objname, name2objname.values(screen_name)) {
            ServerPlayer *player = players.value(objname);
            if (player && player->getState() == "offline" && !player->getRoom()->isFinished()) {
                player->getRoom()->reconnect(player, socket);
                return;
            }
        }
    }

    if (current == NULL || current->isFull() || current->isFinished())
        createNewRoom();

    ServerPlayer *player = current->addSocket(socket);
    current->signup(player, screen_name, avatar, false);
}

void Server::cleanup() {
    const ClientSocket *socket = qobject_cast<const ClientSocket *>(sender());
    if (Config.ForbidSIMC)
        addresses.remove(socket->peerAddress());
}

void Server::signupPlayer(ServerPlayer *player) {
    name2objname.insert(player->screenName(), player->objectName());
    players.insert(player->objectName(), player);
}

void Server::gameOver() {
    Room *room = qobject_cast<Room *>(sender());
    rooms.remove(room);

    foreach (ServerPlayer *player, room->findChildren<ServerPlayer *>()) {
        name2objname.remove(player->screenName(), player->objectName());
        players.remove(player->objectName());
    }
}
