#include "server.h"
#include "settings.h"
#include "room.h"
#include "engine.h"
#include "nativesocket.h"
#include "banpairdialog.h"
#include "scenario.h"
#include "challengemode.h"
#include "contestdb.h"
#include "choosegeneraldialog.h"

#include <QInputDialog>
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
#include <QHttp>
#include <QAction>

static QLayout *HLay(QWidget *left, QWidget *right){
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(left);
    layout->addWidget(right);

    return layout;
}

ServerDialog::ServerDialog(QWidget *parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Start server"));

    QLayout *left = createLeft();
    QLayout *right = createRight();

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addLayout(left);
    hlayout->addLayout(right);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(hlayout);
    layout->addLayout(createButtonLayout());

    setLayout(layout);
}

void ServerDialog::ensureEnableAI(){
    ai_enable_checkbox->setChecked(true);
}


QLayout *ServerDialog::createLeft(){
    server_name_edit = new QLineEdit;
    server_name_edit->setText(Config.ServerName);

    timeout_spinbox = new QSpinBox;
    timeout_spinbox->setMinimum(5);
    timeout_spinbox->setMaximum(30);
    timeout_spinbox->setValue(Config.OperationTimeout);
    timeout_spinbox->setSuffix(tr(" seconds"));

    nolimit_checkbox = new QCheckBox(tr("No limit"));
    nolimit_checkbox->setChecked(false);
    connect(nolimit_checkbox, SIGNAL(toggled(bool)), timeout_spinbox, SLOT(setDisabled(bool)));
    nolimit_checkbox->setChecked(Config.OperationNoLimit);

    QFormLayout *form_layout = new QFormLayout;
    form_layout->addRow(tr("Server name"), server_name_edit);
    form_layout->addRow(tr("Operation timeout"), HLay(timeout_spinbox, nolimit_checkbox));
    form_layout->addRow(createGameModeBox());

    return form_layout;
}

KOFBanlistDialog::KOFBanlistDialog(QDialog *parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Select generals that are excluded in 1v1 mode"));

    QVBoxLayout *layout = new QVBoxLayout;

    list = new QListWidget;
    list->setIconSize(General::TinyIconSize);
    list->setViewMode(QListView::IconMode);
    list->setDragDropMode(QListView::NoDragDrop);

    QStringList banlist = Config.value("1v1/Banlist").toStringList();
    foreach(QString name, banlist){
        addGeneral(name);
    }

    QPushButton *add = new QPushButton(tr("Add ..."));
    QPushButton *remove = new QPushButton(tr("Remove"));
    QPushButton *ok = new QPushButton(tr("OK"));

    connect(remove, SIGNAL(clicked()), this, SLOT(removeGeneral()));
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    connect(this, SIGNAL(accepted()), this, SLOT(save()));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(add);
    hlayout->addWidget(remove);
    hlayout->addWidget(ok);

    layout->addWidget(list);
    layout->addLayout(hlayout);
    setLayout(layout);

    FreeChooseDialog *chooser = new FreeChooseDialog(this, false);
    connect(add, SIGNAL(clicked()), chooser, SLOT(exec()));
    connect(chooser, SIGNAL(general_chosen(QString)), this, SLOT(addGeneral(QString)));
}

void KOFBanlistDialog::addGeneral(const QString &name){
    const General *general = Sanguosha->getGeneral(name);
    QIcon icon(general->getPixmapPath("tiny"));
    QString text = Sanguosha->translate(name);
    QListWidgetItem *item = new QListWidgetItem(icon, text, list);
    item->setData(Qt::UserRole, name);
}

void KOFBanlistDialog::removeGeneral(){
    int row = list->currentRow();
    if(row != -1)
        delete list->takeItem(row);
}

void KOFBanlistDialog::save(){
    QSet<QString> banset;

    int i;
    for(i=0; i<list->count(); i++){
        banset << list->item(i)->data(Qt::UserRole).toString();
    }

    QStringList banlist = banset.toList();
    Config.setValue("1v1/Banlist", QVariant::fromValue(banlist));
}

void ServerDialog::edit1v1Banlist(){
    KOFBanlistDialog *dialog = new KOFBanlistDialog(this);
    dialog->exec();
}

QGroupBox *ServerDialog::create3v3Box(){
    QGroupBox *box = new QGroupBox(tr("3v3 options"));
    box->setEnabled(Config.GameMode == "06_3v3");

    QVBoxLayout *vlayout = new QVBoxLayout;

    standard_3v3_radiobutton = new QRadioButton(tr("Standard mode"));
    QRadioButton *extend = new QRadioButton(tr("Extension mode"));
    QPushButton *extend_edit_button = new QPushButton(tr("General selection ..."));
    extend_edit_button->setEnabled(false);
    connect(extend, SIGNAL(toggled(bool)), extend_edit_button, SLOT(setEnabled(bool)));
    connect(extend_edit_button, SIGNAL(clicked()), this, SLOT(select3v3Generals()));

    exclude_disaster_checkbox = new QCheckBox(tr("Exclude disasters"));
    exclude_disaster_checkbox->setChecked(Config.value("3v3/ExcludeDisasters", true).toBool());

    {
        QComboBox *combobox = new QComboBox;
        combobox->addItem(tr("Normal"), "Normal");
        combobox->addItem(tr("Random"), "Random");
        combobox->addItem(tr("All roles"), "AllRoles");

        role_choose_combobox = combobox;

        QString scheme = Config.value("3v3/RoleChoose", "Normal").toString();
        if(scheme == "Random")
            combobox->setCurrentIndex(1);
        else if(scheme == "AllRoles")
            combobox->setCurrentIndex(2);
    }

    vlayout->addWidget(standard_3v3_radiobutton);
    vlayout->addLayout(HLay(extend, extend_edit_button));
    vlayout->addWidget(exclude_disaster_checkbox);
    vlayout->addLayout(HLay(new QLabel(tr("Role choose")), role_choose_combobox));
    box->setLayout(vlayout);

    bool using_extension = Config.value("3v3/UsingExtension", false).toBool();
    if(using_extension)
        extend->setChecked(true);
    else
        standard_3v3_radiobutton->setChecked(true);

    return box;
}

QGroupBox *ServerDialog::createGameModeBox(){
    QGroupBox *mode_box = new QGroupBox(tr("Game mode"));
    mode_group = new QButtonGroup;

    QVBoxLayout *layout = new QVBoxLayout;

    {
        // normal modes
        QMap<QString, QString> modes = Sanguosha->getAvailableModes();
        QMapIterator<QString, QString> itor(modes);
        while(itor.hasNext()){
            itor.next();

            QRadioButton *button = new QRadioButton(itor.value());
            button->setObjectName(itor.key());
            mode_group->addButton(button);

            if(itor.key() == "02_1v1"){
                // add 1v1 banlist edit button
                QPushButton *edit_button = new QPushButton(tr("Banlist ..."));
                connect(edit_button, SIGNAL(clicked()), this, SLOT(edit1v1Banlist()));
                layout->addLayout(HLay(button, edit_button));

            }else if(itor.key() == "06_3v3"){
                // add 3v3 options
                QGroupBox *box = create3v3Box();
                layout->addWidget(button);
                layout->addWidget(box);
                connect(button, SIGNAL(toggled(bool)), box, SLOT(setEnabled(bool)));
            }else{
                layout->addWidget(button);
            }

            if(itor.key() == Config.GameMode)
                button->setChecked(true);
        }
    }

    {
        // add scenario modes
        QRadioButton *scenario_button = new QRadioButton(tr("Scenario mode"));
        scenario_button->setObjectName("scenario");
        mode_group->addButton(scenario_button);

        scenario_combobox = new QComboBox;
        QStringList names = Sanguosha->getScenarioNames();
        foreach(QString name, names){
            QString scenario_name = Sanguosha->translate(name);
            const Scenario *scenario = Sanguosha->getScenario(name);
            int count = scenario->getPlayerCount();
            QString text = tr("%1 (%2 persons)").arg(scenario_name).arg(count);
            scenario_combobox->addItem(text, name);
        }

        if(mode_group->checkedButton() == NULL){
            int index = names.indexOf(Config.GameMode);
            if(index != -1){
                scenario_button->setChecked(true);
                scenario_combobox->setCurrentIndex(index);
            }
        }

        layout->addLayout(HLay(scenario_button, scenario_combobox));
    }

#if 0

    {
        // add challenge modes
        QRadioButton *challenge_button = new QRadioButton(tr("Challenge mode"));
        challenge_button->setObjectName("challenge");
        mode_group->addButton(challenge_button);

        challenge_combobox = new QComboBox;

        const ChallengeModeSet *set = Sanguosha->getChallengeModeSet();
        QList<const ChallengeMode *> modes = set->allModes();
        QStringList names;
        foreach(const ChallengeMode *mode, modes)
            names << mode->objectName();

        foreach(QString name, names){
            QString text = Sanguosha->translate(name);
            challenge_combobox->addItem(text, name);
        }

        QHBoxLayout *challenge_layout = new QHBoxLayout;
        int i;
        for(i=0; i<4; i++){
            QLabel *avatar = new QLabel;
            challenge_avatars << avatar;
            challenge_layout->addWidget(avatar);
        }

        connect(challenge_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateChallengeLabel(int)));

        if(mode_group->checkedButton() == NULL){
            int index = names.indexOf(Config.GameMode);
            if(index != -1){
                challenge_button->setChecked(true);
                challenge_combobox->setCurrentIndex(index);
                updateChallengeLabel(index);
            }
        }else
            updateChallengeLabel(0);

        //layout->addWidget(challenge_button);
        //layout->addWidget(challenge_combobox);
        //layout->addLayout(challenge_layout);
    }

#endif

    mode_box->setLayout(layout);

    return mode_box;
}

void ServerDialog::updateChallengeLabel(int index){
    QString name = challenge_combobox->itemData(index).toString();
    const ChallengeMode *mode = Sanguosha->getChallengeMode(name);

    if(mode == NULL)
        return;

    QStringList generals = mode->getGenerals();

    if(challenge_avatars.length() != generals.length())
        return;

    int i;
    for(i=0; i<generals.length(); i++){
        const General *general = Sanguosha->getGeneral(generals.at(i));

        QPixmap avatar_pixmap(general->getPixmapPath("tiny"));
        QLabel *avatar = challenge_avatars.at(i);
        avatar->setPixmap(avatar_pixmap);
        avatar->setToolTip(general->getSkillDescription());
    }
}

QLayout *ServerDialog::createRight(){
    QGroupBox *extension_box = new QGroupBox;
    {
        extension_box->setTitle(tr("Game package selection"));
        QGridLayout *extension_layout = new QGridLayout;
        extension_box->setLayout(extension_layout);
        extension_group = new QButtonGroup;
        extension_group->setExclusive(false);

        QStringList extensions = Sanguosha->getExtensions();
        QSet<QString> ban_packages = Config.BanPackages.toSet();

        int i;
        for(i=0; i<extensions.length(); i++){
            QString extension = extensions.at(i);
            QCheckBox *checkbox = new QCheckBox;
            checkbox->setObjectName(extension);
            checkbox->setText(Sanguosha->translate(extension));
            checkbox->setChecked(! ban_packages.contains(extension));

            extension_group->addButton(checkbox);

            int row = i / 2;
            int column = i % 2;
            extension_layout->addWidget(checkbox, row, column);
        }
    }

    QGroupBox *advanced_box = new QGroupBox;

    {
        advanced_box->setTitle(tr("Advanced"));
        QVBoxLayout *layout = new QVBoxLayout;
        advanced_box->setLayout(layout);

        contest_mode_checkbox = new QCheckBox(tr("Contest mode"));
        contest_mode_checkbox->setChecked(Config.ContestMode);
        contest_mode_checkbox->setToolTip(tr("Requires password to login, hide screen name and disable kicking"));

        free_choose_checkbox = new QCheckBox(tr("Choose generals and cards freely"));
        free_choose_checkbox->setChecked(Config.FreeChoose);

        free_assign_checkbox = new QCheckBox(tr("Assign role and seat freely"));
        free_assign_checkbox->setChecked(Config.value("FreeAssign").toBool());

        maxchoice_spinbox = new QSpinBox;
        maxchoice_spinbox->setRange(5, 10);
        maxchoice_spinbox->setValue(Config.value("MaxChoice", 5).toInt());

        forbid_same_ip_checkbox = new QCheckBox(tr("Forbid same IP with multiple connection"));
        forbid_same_ip_checkbox->setChecked(Config.ForbidSIMC);

        disable_chat_checkbox = new QCheckBox(tr("Disable chat"));
        disable_chat_checkbox->setChecked(Config.DisableChat);

        second_general_checkbox = new QCheckBox(tr("Enable second general"));

        scene_checkbox  = new QCheckBox(tr("Enable Scene"));
        //changjing

        max_hp_scheme_combobox = new QComboBox;
        max_hp_scheme_combobox->addItem(tr("Sum - 3"));
        max_hp_scheme_combobox->addItem(tr("Minimum"));
        max_hp_scheme_combobox->addItem(tr("Average"));
        max_hp_scheme_combobox->setCurrentIndex(Config.MaxHpScheme);
        max_hp_scheme_combobox->setEnabled(Config.Enable2ndGeneral);
        connect(second_general_checkbox, SIGNAL(toggled(bool)), max_hp_scheme_combobox, SLOT(setEnabled(bool)));

        second_general_checkbox->setChecked(Config.Enable2ndGeneral);

        scene_checkbox->setChecked(Config.EnableScene);	//changjing

        QPushButton *banpair_button = new QPushButton(tr("Ban pairs table ..."));
        BanPairDialog *banpair_dialog = new BanPairDialog(this);
        connect(banpair_button, SIGNAL(clicked()), banpair_dialog, SLOT(exec()));

        connect(second_general_checkbox, SIGNAL(toggled(bool)), banpair_button, SLOT(setEnabled(bool)));

        announce_ip_checkbox = new QCheckBox(tr("Annouce my IP in WAN"));
        announce_ip_checkbox->setChecked(Config.AnnounceIP);
        announce_ip_checkbox->setEnabled(false); // not support now

        address_edit = new QLineEdit;
        address_edit->setText(Config.Address);

        #if QT_VERSION >= 0x040700
        address_edit->setPlaceholderText(tr("Public IP or domain"));
        #endif  

        QPushButton *detect_button = new QPushButton(tr("Detect my WAN IP"));
        connect(detect_button, SIGNAL(clicked()), this, SLOT(onDetectButtonClicked()));

        //address_edit->setEnabled(announce_ip_checkbox->isChecked());
        // connect(announce_ip_checkbox, SIGNAL(toggled(bool)), address_edit, SLOT(setEnabled(bool)));

        port_edit = new QLineEdit;
        port_edit->setText(QString::number(Config.ServerPort));
        port_edit->setValidator(new QIntValidator(1, 9999, port_edit));

        layout->addWidget(contest_mode_checkbox);
        layout->addWidget(forbid_same_ip_checkbox);
        layout->addWidget(disable_chat_checkbox);
        layout->addWidget(free_choose_checkbox);
        //layout->addWidget(free_assign_checkbox);
        layout->addLayout(HLay(new QLabel(tr("Upperlimit for general")), maxchoice_spinbox));
        layout->addLayout(HLay(second_general_checkbox, banpair_button));
        layout->addLayout(HLay(new QLabel(tr("Max HP scheme")), max_hp_scheme_combobox));
        layout->addWidget(scene_checkbox);		//changjing
        layout->addWidget(announce_ip_checkbox);
        layout->addLayout(HLay(new QLabel(tr("Address")), address_edit));
        layout->addWidget(detect_button);
        layout->addLayout(HLay(new QLabel(tr("Port")), port_edit));
    }

    QGroupBox *ai_box = new QGroupBox;

    {
        ai_box->setTitle(tr("Artificial intelligence"));
        QVBoxLayout *layout = new QVBoxLayout;
        ai_box->setLayout(layout);

        ai_enable_checkbox = new QCheckBox(tr("Enable AI"));
        ai_enable_checkbox->setChecked(Config.EnableAI);

        role_predictable_checkbox = new QCheckBox(tr("Role predictable"));
        role_predictable_checkbox->setChecked(Config.value("RolePredictable", true).toBool());

        ai_chat_checkbox = new QCheckBox(tr("AI Chat"));
        ai_chat_checkbox->setChecked(Config.value("AIChat", true).toBool());

        ai_delay_spinbox = new QSpinBox;
        ai_delay_spinbox->setMinimum(0);
        ai_delay_spinbox->setMaximum(5000);
        ai_delay_spinbox->setValue(Config.AIDelay);
        ai_delay_spinbox->setSuffix(tr(" millisecond"));

        layout->addWidget(ai_enable_checkbox);
        layout->addWidget(role_predictable_checkbox);
        layout->addWidget(ai_chat_checkbox);
        layout->addLayout(HLay(new QLabel(tr("AI delay")), ai_delay_spinbox));
    }

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(extension_box);
    vlayout->addWidget(advanced_box);
    vlayout->addWidget(ai_box);
    vlayout->addStretch();

    return vlayout;
}

QLayout *ServerDialog::createButtonLayout(){
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

void ServerDialog::onDetectButtonClicked(){
    QString host = "www.net.cn";
    QString path = "/static/customercare/yourIP.asp";
    QHttp *http = new QHttp(this);
    http->setHost(host);

    connect(http, SIGNAL(done(bool)), this, SLOT(onHttpDone(bool)));
    http->get(path);
}

void ServerDialog::onHttpDone(bool error){
    QHttp *http = qobject_cast<QHttp *>(sender());

    if(error){
        QMessageBox::warning(this, tr("Warning"), http->errorString());
    }else{
        QRegExp rx("(\\d+\\.\\d+\\.\\d+\\.\\d+)");
        int index = rx.indexIn(http->readAll());
        if(index != -1){
            QString addr = rx.capturedTexts().at(0);
            address_edit->setText(addr);
        }

        http->deleteLater();
    }
}

void ServerDialog::onOkButtonClicked(){
    if(announce_ip_checkbox->isChecked() && address_edit->text().isEmpty()){
        QMessageBox::warning(this, tr("Warning"), tr("Please fill address when you want to annouce your server's IP"));
    }else
        accept();
}

Select3v3GeneralDialog::Select3v3GeneralDialog(QDialog *parent)
    :QDialog(parent)
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

void Select3v3GeneralDialog::fillTabWidget(){
    QList<const Package *> packages = Sanguosha->findChildren<const Package *>();
    foreach(const Package *package, packages){
        switch(package->getType()){
        case Package::GeneralPack:
        case Package::MixedPack: {
                QListWidget *list = new QListWidget;
                list->setIconSize(General::TinyIconSize);
                list->setViewMode(QListView::IconMode);
                list->setDragDropMode(QListView::NoDragDrop);
                fillListWidget(list, package);

                tab_widget->addTab(list, Sanguosha->translate(package->objectName()));
            }
        default:
            break;
        }
    }
}

void Select3v3GeneralDialog::fillListWidget(QListWidget *list, const Package *pack){
    QList<const General *> generals = pack->findChildren<const General *>();
    foreach(const General *general, generals){
        if(general->isHidden())
            continue;

        QListWidgetItem *item = new QListWidgetItem(list);
        item->setData(Qt::UserRole, general->objectName());
        item->setIcon(QIcon(general->getPixmapPath("tiny")));

        bool checked = false;
        if(ex_generals.isEmpty()){
            checked = pack->objectName() == "standard" || pack->objectName() == "wind"
                      || general->objectName() == "xiaoqiao";
        }else
            checked = ex_generals.contains(general->objectName());

        if(checked)
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

void Select3v3GeneralDialog::toggleCheck(){
    QWidget *widget = tab_widget->currentWidget();
    QListWidget *list = qobject_cast<QListWidget *>(widget);

    if(list == NULL || list->item(0) == NULL)
        return;

    bool checked = list->item(0)->checkState() != Qt::Checked;

    int i;
    for(i=0; i<list->count(); i++)
        list->item(i)->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

void Select3v3GeneralDialog::save3v3Generals(){
    ex_generals.clear();

    int i;
    for(i=0; i<tab_widget->count(); i++){
        QWidget *widget = tab_widget->widget(i);
        QListWidget *list = qobject_cast<QListWidget *>(widget);
        if(list){
            int i;
            for(i=0; i<list->count(); i++){
                QListWidgetItem *item = list->item(i);
                if(item->checkState() == Qt::Checked)
                    ex_generals << item->data(Qt::UserRole).toString();
            }
        }
    }

    QStringList list = ex_generals.toList();
    QVariant data = QVariant::fromValue(list);
    Config.setValue("3v3/ExtensionGenerals", data);
}

void ServerDialog::select3v3Generals(){
    QDialog *dialog = new Select3v3GeneralDialog(this);
    dialog->exec();
}

bool ServerDialog::config(){
    exec();

    if(result() != Accepted)
        return false;

    Config.ServerName = server_name_edit->text();
    Config.OperationTimeout = timeout_spinbox->value();
    Config.OperationNoLimit = nolimit_checkbox->isChecked();
    Config.ContestMode = contest_mode_checkbox->isChecked();
    Config.FreeChoose = free_choose_checkbox->isChecked();
    Config.ForbidSIMC = forbid_same_ip_checkbox->isChecked();
    Config.DisableChat = disable_chat_checkbox->isChecked();
    Config.Enable2ndGeneral = second_general_checkbox->isChecked();
    Config.EnableScene = scene_checkbox->isChecked();		//changjing
    Config.MaxHpScheme = max_hp_scheme_combobox->currentIndex();
    Config.AnnounceIP = announce_ip_checkbox->isChecked();
    Config.Address = address_edit->text();
    Config.EnableAI = ai_enable_checkbox->isChecked();
    Config.AIDelay = ai_delay_spinbox->value();
    Config.ServerPort = port_edit->text().toInt();

    // game mode
    QString objname = mode_group->checkedButton()->objectName();
    if(objname == "scenario")
        Config.GameMode = scenario_combobox->itemData(scenario_combobox->currentIndex()).toString();
    else if(objname == "challenge")
        Config.GameMode = challenge_combobox->itemData(challenge_combobox->currentIndex()).toString();
    else
        Config.GameMode = objname;

    Config.setValue("ServerName", Config.ServerName);
    Config.setValue("GameMode", Config.GameMode);
    Config.setValue("OperationTimeout", Config.OperationTimeout);
    Config.setValue("OperationNoLimit", Config.OperationNoLimit);
    Config.setValue("ContestMode", Config.ContestMode);
    Config.setValue("FreeChoose", Config.FreeChoose);
    Config.setValue("FreeAssign", free_assign_checkbox->isChecked());
    Config.setValue("MaxChoice", maxchoice_spinbox->value());
    Config.setValue("ForbidSIMC", Config.ForbidSIMC);
    Config.setValue("DisableChat", Config.DisableChat);
    Config.setValue("Enable2ndGeneral", Config.Enable2ndGeneral);
    Config.setValue("EnableScene", Config.EnableScene);	//changjing
    Config.setValue("MaxHpScheme", Config.MaxHpScheme);
    Config.setValue("EnableAI", Config.EnableAI);
    Config.setValue("RolePredictable", role_predictable_checkbox->isChecked());
    Config.setValue("AIChat", ai_chat_checkbox->isChecked());
    Config.setValue("AIDelay", Config.AIDelay);
    Config.setValue("ServerPort", Config.ServerPort);
    Config.setValue("AnnounceIP", Config.AnnounceIP);
    Config.setValue("Address", Config.Address);

    Config.beginGroup("3v3");
    Config.setValue("UsingExtension", ! standard_3v3_radiobutton->isChecked());
    Config.setValue("RoleChoose", role_choose_combobox->itemData(role_choose_combobox->currentIndex()).toString());
    Config.setValue("ExcludeDisaster", exclude_disaster_checkbox->isChecked());
    Config.endGroup();

    QSet<QString> ban_packages;
    QList<QAbstractButton *> checkboxes = extension_group->buttons();
    foreach(QAbstractButton *checkbox, checkboxes){
        if(!checkbox->isChecked()){
            QString package_name = checkbox->objectName();
            Sanguosha->addBanPackage(package_name);
            ban_packages.insert(package_name);
        }
    }

    Config.BanPackages = ban_packages.toList();
    Config.setValue("BanPackages", Config.BanPackages);

    if(Config.ContestMode){
        ContestDB *db = ContestDB::GetInstance();
        return db->loadMembers();
    }

    return true;
}

Server::Server(QObject *parent)
    :QObject(parent)
{
    server = new NativeServerSocket;
    server->setParent(this);

    connect(server, SIGNAL(new_connection(ClientSocket*)), this, SLOT(processNewConnection(ClientSocket*)));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));

    current = NULL;
}

void Server::broadcast(const QString &msg){
    QString to_sent = msg.toUtf8().toBase64();
    to_sent = ".:" + to_sent;
    foreach(Room *room, rooms)
        room->broadcastInvoke("speak", to_sent);
}

bool Server::listen(){
    return server->listen();
}

void Server::daemonize(){
    server->daemonize();

    createNewRoom();
}

Room *Server::createNewRoom(){
    Room *new_room = new Room(this, Config.GameMode);
    QString error_msg = new_room->createLuaState();

    if(!error_msg.isEmpty()){
        QMessageBox::information(NULL, tr("Lua scripts error"), error_msg);
        return NULL;
    }

    current = new_room;
    rooms.insert(current);

    connect(current, SIGNAL(room_message(QString)), this, SIGNAL(server_message(QString)));
    connect(current, SIGNAL(game_over(QString)), this, SLOT(gameOver()));

    return current;
}

void Server::processNewConnection(ClientSocket *socket){
    if(Config.ForbidSIMC){
        QString addr = socket->peerAddress();
        if(addresses.contains(addr)){
            socket->disconnectFromHost();
            emit server_message(tr("Forbid the connection of address %1").arg(addr));
            return;
        }else
            addresses.insert(addr);
    }

    connect(socket, SIGNAL(disconnected()), this, SLOT(cleanup()));
    socket->send("checkVersion " + Sanguosha->getVersion());
    socket->send("setup " + Sanguosha->getSetupString());
    emit server_message(tr("%1 connected").arg(socket->peerName()));

    connect(socket, SIGNAL(message_got(char*)), this, SLOT(processRequest(char*)));
}

static inline QString ConvertFromBase64(const QString &base64){
    QByteArray data = QByteArray::fromBase64(base64.toAscii());
    return QString::fromUtf8(data);
}

void Server::processRequest(char *request){
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());
    socket->disconnect(this, SLOT(processRequest(char*)));

    QRegExp rx("(signupr?) (.+):(.+)(:.+)?\n");
    if(!rx.exactMatch(request)){
        emit server_message(tr("Invalid signup string: %1").arg(request));
        socket->send("warn INVALID_FORMAT");
        socket->disconnectFromHost();
        return;
    }

    QStringList texts = rx.capturedTexts();
    QString command = texts.at(1);
    QString screen_name = ConvertFromBase64(texts.at(2));
    QString avatar = texts.at(3);

    if(Config.ContestMode){
        QString password = texts.value(4);
        if(password.isEmpty()){
            socket->send("warn REQUIRE_PASSWORD");
            socket->disconnectFromHost();
            return;
        }

        password.remove(QChar(':'));
        ContestDB *db = ContestDB::GetInstance();
        if(!db->checkPassword(screen_name, password)){
            socket->send("warn WRONG_PASSWORD");
            socket->disconnectFromHost();
            return;
        }
    }

    if(command == "signupr"){
        foreach(QString objname, name2objname.values(screen_name)){
            ServerPlayer *player = players.value(objname);
            if(player && player->getState() == "offline"){
                player->getRoom()->reconnect(player, socket);
                return;
            }
        }
    }

    if(current->isFull())
        createNewRoom();

    ServerPlayer *player = current->addSocket(socket);
    current->signup(player, screen_name, avatar, false);
}

void Server::cleanup(){
    const ClientSocket *socket = qobject_cast<const ClientSocket *>(sender());

    if(Config.ForbidSIMC)
        addresses.remove(socket->peerAddress());
}

void Server::signupPlayer(ServerPlayer *player){
    name2objname.insert(player->screenName(), player->objectName());
    players.insert(player->objectName(), player);
}

void Server::gameOver(){
    Room *room = qobject_cast<Room *>(sender());
    rooms.remove(room);

    foreach(ServerPlayer *player, room->findChildren<ServerPlayer *>()){
        name2objname.remove(player->screenName(), player->objectName());
        players.remove(player->objectName());
    }
}
