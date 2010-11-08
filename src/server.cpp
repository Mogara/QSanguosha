#include "server.h"
#include "settings.h"
#include "room.h"
#include "engine.h"
#include "nativesocket.h"

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
    server_name_lineedit = new QLineEdit;
    server_name_lineedit->setText(Config.ServerName);

    player_count_spinbox = new QSpinBox;
    player_count_spinbox->setMinimum(2);
    player_count_spinbox->setMaximum(8);
    player_count_spinbox->setValue(Config.PlayerCount);
    player_count_spinbox->setSuffix(tr(" persons"));

    timeout_spinbox = new QSpinBox;
    timeout_spinbox->setMinimum(5);
    timeout_spinbox->setMaximum(30);
    timeout_spinbox->setValue(Config.OperationTimeout);
    timeout_spinbox->setSuffix(tr(" seconds"));

    nolimit_checkbox = new QCheckBox(tr("No limit"));
    nolimit_checkbox->setChecked(false);
    connect(nolimit_checkbox, SIGNAL(toggled(bool)), timeout_spinbox, SLOT(setDisabled(bool)));
    nolimit_checkbox->setChecked(Config.OperationNoLimit);

    QGroupBox *extension_box = new QGroupBox;
    extension_box->setTitle(tr("Extension package selection"));
    QGridLayout *extension_layout = new QGridLayout;
    extension_box->setLayout(extension_layout);
    extension_group = new QButtonGroup;
    extension_group->setExclusive(false);

    static QStringList extensions;
    if(extensions.isEmpty())
        extensions << "wind" << "fire" << "thicket" << "maneuvering" << "god" << "yitian";
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

    QGroupBox *scenario_box = new QGroupBox;

    {
        scenario_box->setTitle(tr("Scenario mode"));
        QVBoxLayout *layout = new QVBoxLayout;
        scenario_box->setLayout(layout);
        QCheckBox *scenario_checkbox = new QCheckBox(tr("Enable scenario mode"));
        layout->addWidget(scenario_checkbox);

        scenario_combobox = new QComboBox;
        QStringList names = Sanguosha->getScenarioNames();
        foreach(QString name, names){
            QVariant data = name;
            QString text = Sanguosha->translate(name);
            scenario_combobox->addItem(text, data);
        }

        layout->addWidget(scenario_combobox);

        connect(scenario_checkbox, SIGNAL(toggled(bool)), scenario_combobox, SLOT(setEnabled(bool)));

        int index = names.indexOf(Config.Scenario);
        if(index == -1){
            scenario_checkbox->setChecked(false);
            scenario_combobox->setEnabled(false);
        }else{
            scenario_checkbox->setChecked(true);
            scenario_combobox->setCurrentIndex(index);
        }
    }

    QFormLayout *form_layout = new QFormLayout;
    form_layout->addRow(tr("Server name"), server_name_lineedit);
    form_layout->addRow(tr("Player count"), player_count_spinbox);
    form_layout->addRow(tr("Operation timeout"), timeout_spinbox);
    form_layout->addWidget(nolimit_checkbox);
    form_layout->addRow(extension_box);
    form_layout->addRow(scenario_box);

    return form_layout;
}

QLayout *ServerDialog::createRight(){
    QGroupBox *advanced_box = new QGroupBox;

    {
        advanced_box->setTitle(tr("Advanced"));
        QVBoxLayout *layout = new QVBoxLayout;
        advanced_box->setLayout(layout);

        free_choose_checkbox = new QCheckBox(tr("Free choose generals"));
        free_choose_checkbox->setToolTip(tr("Enable this will make the clients choose generals freely"));
        free_choose_checkbox->setChecked(Config.FreeChoose);
        layout->addWidget(free_choose_checkbox);

        forbid_same_ip_checkbox = new QCheckBox(tr("Forbid same IP with multiple connection"));
        forbid_same_ip_checkbox->setChecked(Config.ForbidSIMC);
        layout->addWidget(forbid_same_ip_checkbox);

        second_general_checkbox = new QCheckBox(tr("Enable second general"));
        QPushButton *banpair_button = new QPushButton(tr("Ban pairs table ..."));

        connect(second_general_checkbox, SIGNAL(toggled(bool)), banpair_button, SLOT(setEnabled(bool)));
        second_general_checkbox->setChecked(Config.Enable2ndGeneral);

        layout->addLayout(HLay(second_general_checkbox, banpair_button));

        announce_ip_checkbox = new QCheckBox(tr("Annouce my IP in WAN"));
        announce_ip_checkbox->setChecked(Config.AnnounceIP);
        layout->addWidget(announce_ip_checkbox);

        port_lineedit = new QLineEdit;
        port_lineedit->setText(QString::number(Config.ServerPort));
        port_lineedit->setValidator(new QIntValidator(1, 9999, port_lineedit));
        layout->addLayout(HLay(new QLabel(tr("Port")), port_lineedit));
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
        ai_box->setEnabled(false);
    }

    QVBoxLayout *vlayout = new QVBoxLayout;
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

    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

    return button_layout;
}

bool ServerDialog::config(){
    exec();

    if(result() != Accepted)
        return false;

    Config.ServerName = server_name_lineedit->text();
    Config.PlayerCount = player_count_spinbox->value();
    Config.OperationTimeout = timeout_spinbox->value();
    Config.OperationNoLimit = nolimit_checkbox->isChecked();
    Config.FreeChoose = free_choose_checkbox->isChecked();
    Config.ForbidSIMC = forbid_same_ip_checkbox->isChecked();
    Config.Enable2ndGeneral = second_general_checkbox->isChecked();
    Config.AnnounceIP = announce_ip_checkbox->isChecked();
    Config.AILevel = ai_group->checkedId();
    if(scenario_combobox->isEnabled())
        Config.Scenario = scenario_combobox->itemData(scenario_combobox->currentIndex()).toString();
    else
        Config.Scenario.clear();
    Config.ServerPort = port_lineedit->text().toInt();   

    Config.setValue("ServerName", Config.ServerName);
    Config.setValue("PlayerCount", Config.PlayerCount);
    Config.setValue("OperationTimeout", Config.OperationTimeout);
    Config.setValue("OperationNoLimit", Config.OperationNoLimit);
    Config.setValue("FreeChoose", Config.FreeChoose);
    Config.setValue("ForbidSIMC", Config.ForbidSIMC);
    Config.setValue("Enable2ndGeneral", Config.Enable2ndGeneral);
    Config.setValue("AILevel", Config.AILevel);
    Config.setValue("Scenario", Config.Scenario);
    Config.setValue("ServerPort", Config.ServerPort);
    Config.setValue("AnnounceIP", Config.AnnounceIP);

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

    session = NULL;
}

Server::~Server(){
    if(session){
        irc_disconnect(session);
        irc_destroy_session(session);
    }
}

bool Server::listen(){
    return server->listen();
}

static void server_connect(irc_session_t *session,
                           const char *event,
                           const char *origin,
                           const char **params,
                           unsigned int count)
{
    char channel[255];
    qstrcpy(channel, Config.IrcChannel.toAscii().constData());

    irc_cmd_join(session, channel, NULL);

    Server *server = static_cast<Server *>(irc_get_ctx(session));
    server->emitDetectableMessage();
}

static void dummy_callback(irc_session_t *session,
                           irc_dcc_t id,
                           int status,
                           void *ctx,
                           const char *data,
                           unsigned int length)
{

}

static void server_channel(irc_session_t *session,
                           const char *event,
                           const char *origin,
                           const char **params,
                           unsigned int count)
{
    const char *content = params[1];
    if(qstrcmp(content, "whoIsServer") == 0){
        char server_info[1024];
        qstrcpy(server_info, Sanguosha->getSetupString().toAscii().constData());

        irc_dcc_t dcc;
        irc_dcc_chat(session, NULL, origin, dummy_callback, &dcc);
        irc_cmd_msg(session, origin, server_info);
    }
}

void Server::daemonize(){
    server->daemonize();

    if(Config.AnnounceIP){
        // winsock initialize
        WORD wVersionRequested = MAKEWORD (1, 1);
        WSADATA wsaData;

        WSAStartup (wVersionRequested, &wsaData);

        irc_callbacks_t callbacks;
        memset(&callbacks, 0, sizeof(callbacks));
        callbacks.event_connect = server_connect;
        callbacks.event_channel = server_channel;
        session = irc_create_session(&callbacks);

        irc_set_ctx(session, this);

        irc_option_set(session, LIBIRC_OPTION_STRIPNICKS);

        char server[255], nick[255];

        qstrcpy(server, Config.IrcHost.toAscii().constData());
        qstrcpy(nick, Config.IrcNick.toAscii().constData());
        ushort port = Config.IrcPort;

        int result = irc_connect(session, server, port, NULL, nick, NULL, NULL);
        if(result == 0){
            IrcRunner *runner = new IrcRunner(this, session);
            runner->start();
        }
    }
}

void Server::processNewConnection(ClientSocket *socket){
    if(Config.ForbidSIMC){
        QString addr = socket->peerAddress();
        if(addresses.contains(addr)){
            socket->disconnectFromHost();
            emit server_message(tr("Forbid the connection of address %1").arg(addr));
            return;
        }else{
            addresses.insert(addr);
            connect(socket, SIGNAL(disconnected()), this, SLOT(removeAddress()));
        }
    }

    // remove the game over room first
    QMutableListIterator<Room *> itor(rooms);
    while(itor.hasNext()){
        Room *room = itor.next();
        if(room->isFinished()){
            delete room;
            itor.remove();
        }
    }

    // find the free room for the new connection
    Room *free_room = NULL;
    foreach(Room *room, rooms){
        if(!room->isFull()){
            free_room = room;
            break;
        }
    }

    // if no free room is found, create a new room for him
    if(free_room == NULL){
        const Scenario *scenario = NULL;
        if(!Config.Scenario.isEmpty())
            scenario = Sanguosha->getScenario(Config.Scenario);

        free_room = new Room(this, scenario);
        rooms << free_room;
        connect(free_room, SIGNAL(room_message(QString)), this, SIGNAL(server_message(QString)));
    }

    free_room->addSocket(socket);    

    emit server_message(tr("%1 connected").arg(socket->peerName()));
}

void Server::removeAddress(){
    ClientSocket *socket = qobject_cast<ClientSocket *>(sender());

    if(socket){
        addresses.remove(socket->peerAddress());
    }
}

void Server::emitDetectableMessage(){
    emit server_message(tr("Server can be detected at WAN"));
}
