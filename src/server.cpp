#include "server.h"
#include "settings.h"
#include "room.h"
#include "engine.h"
#include "nativesocket.h"
#include "banpairdialog.h"
#include "scenario.h"
#include "challengemode.h"

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

            layout->addWidget(button);
            mode_group->addButton(button);

            if(itor.key() == Config.GameMode)
                button->setChecked(true);
        }
    }

    {
        // add scenario modes
        QRadioButton *scenario_button = new QRadioButton(tr("Scenario mode"));
        scenario_button->setObjectName("scenario");

        layout->addWidget(scenario_button);
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
        layout->addWidget(scenario_combobox);

        if(mode_group->checkedButton() == NULL){
            int index = names.indexOf(Config.GameMode);
            if(index != -1){
                scenario_button->setChecked(true);
                scenario_combobox->setCurrentIndex(index);
            }
        }
    }


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
        extension_box->setTitle(tr("Extension package selection"));
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

        free_choose_checkbox = new QCheckBox(tr("Choose generals and cards freely"));
        free_choose_checkbox->setChecked(Config.FreeChoose);
        layout->addWidget(free_choose_checkbox);

        forbid_same_ip_checkbox = new QCheckBox(tr("Forbid same IP with multiple connection"));
        forbid_same_ip_checkbox->setChecked(Config.ForbidSIMC);
        layout->addWidget(forbid_same_ip_checkbox);

        second_general_checkbox = new QCheckBox(tr("Enable second general"));        
        layout->addWidget(second_general_checkbox);

        max_hp_scheme_combobox = new QComboBox;
        max_hp_scheme_combobox->addItem(tr("Sum - 3"));
        max_hp_scheme_combobox->addItem(tr("Minimum"));
        max_hp_scheme_combobox->setCurrentIndex(Config.MaxHpScheme);
        max_hp_scheme_combobox->setEnabled(Config.Enable2ndGeneral);
        connect(second_general_checkbox, SIGNAL(toggled(bool)), max_hp_scheme_combobox, SLOT(setEnabled(bool)));

        second_general_checkbox->setChecked(Config.Enable2ndGeneral);


        layout->addLayout(HLay(new QLabel(tr("Max HP scheme")), max_hp_scheme_combobox));


        /*
        QPushButton *banpair_button = new QPushButton(tr("Ban pairs table ..."));
        BanPairDialog *banpair_dialog = new BanPairDialog(this);
        connect(banpair_button, SIGNAL(clicked()), banpair_dialog, SLOT(exec()));

        connect(second_general_checkbox, SIGNAL(toggled(bool)), banpair_button, SLOT(setEnabled(bool)));


        layout->addLayout(HLay(second_general_checkbox, banpair_button));
        */

        announce_ip_checkbox = new QCheckBox(tr("Annouce my IP in WAN"));
        announce_ip_checkbox->setChecked(Config.AnnounceIP);
        announce_ip_checkbox->setEnabled(false);
        layout->addWidget(announce_ip_checkbox);

        address_edit = new QLineEdit;
        address_edit->setText(Config.Address);
        address_edit->setPlaceholderText(tr("Public IP or domain"));
        layout->addLayout(HLay(new QLabel(tr("Address")), address_edit));

        QPushButton *detect_button = new QPushButton(tr("Detect my WAN IP"));
        connect(detect_button, SIGNAL(clicked()), this, SLOT(onDetectButtonClicked()));
        layout->addWidget(detect_button);

        //address_edit->setEnabled(announce_ip_checkbox->isChecked());
        // connect(announce_ip_checkbox, SIGNAL(toggled(bool)), address_edit, SLOT(setEnabled(bool)));

        port_edit = new QLineEdit;
        port_edit->setText(QString::number(Config.ServerPort));
        port_edit->setValidator(new QIntValidator(1, 9999, port_edit));
        layout->addLayout(HLay(new QLabel(tr("Port")), port_edit));
    }

    QGroupBox *ai_box = new QGroupBox;

    {
        ai_box->setTitle(tr("Artificial intelligence"));
        QVBoxLayout *layout = new QVBoxLayout;
        ai_box->setLayout(layout);

        ai_group = new QButtonGroup;

        QRadioButton *stupid = new QRadioButton(tr("Stupid"));
        QRadioButton *normal = new QRadioButton(tr("Normal"));
        QRadioButton *smart = new QRadioButton(tr("Smart"));

        ai_group->addButton(stupid, 0);
        ai_group->addButton(normal, 1);
        ai_group->addButton(smart, 2);

        layout->addWidget(stupid);
        layout->addWidget(normal);
        layout->addWidget(smart);

        smart->setChecked(true);
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

        delete http;
    }
}

void ServerDialog::onOkButtonClicked(){
    if(announce_ip_checkbox->isChecked() && address_edit->text().isEmpty()){
        QMessageBox::warning(this, tr("Warning"), tr("Please fill address when you want to annouce your server's IP"));
    }else
        accept();
}

bool ServerDialog::config(){
    exec();

    if(result() != Accepted)
        return false;

    Config.ServerName = server_name_edit->text();
    Config.OperationTimeout = timeout_spinbox->value();
    Config.OperationNoLimit = nolimit_checkbox->isChecked();
    Config.FreeChoose = free_choose_checkbox->isChecked();
    Config.ForbidSIMC = forbid_same_ip_checkbox->isChecked();
    Config.Enable2ndGeneral = second_general_checkbox->isChecked();
    Config.MaxHpScheme = max_hp_scheme_combobox->currentIndex();
    Config.AnnounceIP = announce_ip_checkbox->isChecked();
    Config.Address = address_edit->text();
    Config.AILevel = ai_group->checkedId();
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
    Config.setValue("FreeChoose", Config.FreeChoose);
    Config.setValue("ForbidSIMC", Config.ForbidSIMC);
    Config.setValue("Enable2ndGeneral", Config.Enable2ndGeneral);
    Config.setValue("MaxHpScheme", Config.MaxHpScheme);
    Config.setValue("AILevel", Config.AILevel);
    Config.setValue("ServerPort", Config.ServerPort);
    Config.setValue("AnnounceIP", Config.AnnounceIP);
    Config.setValue("Address", Config.Address);

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

bool Server::listen(){
    // the server side will load the AI module
    Sanguosha->loadAIs();

    return server->listen();
}

void Server::daemonize(){
    server->daemonize();

    current = new Room(this, Config.GameMode);
    connect(current, SIGNAL(room_message(QString)), this, SIGNAL(server_message(QString)));
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

    if(current->isFull()){
        current = new Room(this, Config.GameMode);
        connect(current, SIGNAL(room_message(QString)), this, SIGNAL(server_message(QString)));
    }

    current->addSocket(socket);
    connect(socket, SIGNAL(disconnected()), this, SLOT(cleanup()));

    emit server_message(tr("%1 connected").arg(socket->peerName()));
}

void Server::cleanup(){
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());

    if(Config.ForbidSIMC)
        addresses.remove(socket->peerAddress());
}
