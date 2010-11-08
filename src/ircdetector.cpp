#include "ircdetector.h"
#include "settings.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QClipboard>
#include <QApplication>

IrcRunner::IrcRunner(QObject *parent, irc_session_t *session)
    :QThread(parent), session(session)
{

}

void IrcRunner::run()
{
    int result = irc_run(session);
    QThread::exit(result);
}

static void detector_connect(irc_session_t *session,
                             const char *event,
                             const char *origin,
                             const char **params,
                             unsigned int count)
{
    char channel[255];
    qstrcpy(channel, Config.IrcChannel.toAscii().constData());

    irc_cmd_join(session, channel, NULL);
    irc_cmd_msg(session, channel, "whoIsServer");

    IrcDetector *detector = static_cast<IrcDetector*>(irc_get_ctx(session));
    detector->emitConnected();
}

static void detector_dcc_callback(irc_session_t *session,
                                  const char *nick,
                                  const char *addr,
                                  irc_dcc_t dccid)
{
    irc_dcc_decline(session, dccid);

    IrcDetector *detector = static_cast<IrcDetector*>(irc_get_ctx(session));
    detector->setAddrMap(nick, addr);
}

/*
static void detector_privmsg(irc_session_t *session,
                             const char *event,
                             const char *origin,
                             const char **params,
                             unsigned int count)
{
    const char *server_info = params[1];
    const char *nick = origin;

    IrcDetector *detector = static_cast<IrcDetector*>(irc_get_ctx(session));
    detector->setInfoMap(nick, server_info);
}
*/

static void detector_notice(irc_session_t *session,
                            const char *event,
                            const char *origin,
                            const char **params,
                            unsigned int count)
{
    const char *server_info = params[1];
    const char *nick = origin;

    IrcDetector *detector = static_cast<IrcDetector*>(irc_get_ctx(session));
    detector->setInfoMap(nick, server_info);
}

IrcDetector::IrcDetector(){
    WORD wVersionRequested = MAKEWORD (1, 1);
    WSADATA wsaData;

    WSAStartup (wVersionRequested, &wsaData);

    irc_callbacks_t callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.event_connect = detector_connect;
    callbacks.event_dcc_chat_req = detector_dcc_callback;
    //callbacks.event_privmsg = detector_privmsg;
    callbacks.event_notice = detector_notice;

    session = irc_create_session(&callbacks);
    irc_set_ctx(session, this);
    irc_option_set(session, LIBIRC_OPTION_STRIPNICKS);
}

IrcDetector::~IrcDetector(){
    stop();
}

void IrcDetector::detect(){
    if(irc_is_connected(session)){
        emit server_connected();

        char channel[255];
        qstrcpy(channel, Config.IrcChannel.toAscii().constData());
        irc_cmd_msg(session, channel, "whoIsServer");

        return;
    }

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

void IrcDetector::stop(){
    if(session){
        irc_destroy_session(session);
        session = NULL;
    }
}

void IrcDetector::setAddrMap(const char *nick, const char *addr){
    nick2addr.insert(nick, addr);
}

void IrcDetector::setInfoMap(const char *nick, const char *server_info){
    ServerInfoStruct info;
    if(info.parse(server_info)){
        nick2info.insert(nick, info);

        if(nick2addr.contains(nick)){
            emit detected(nick);
        }
    }
}

QString IrcDetector::getAddr(const QString &nick) const{
    return nick2addr.value(nick);
}

bool IrcDetector::getInfo(const QString &nick, ServerInfoStruct &info) const{
    if(nick2info.contains(nick)){
        info = nick2info.value(nick);
        return true;
    }else
        return false;
}

void IrcDetector::clearMap(){
    nick2addr.clear();
    nick2info.clear();
}

void IrcDetector::emitConnected(){
    emit server_connected();
}

IrcDetectorDialog::IrcDetectorDialog(){
    setWindowTitle(tr("Detect available server's addresses at WAN"));

    detector = NULL;

    info_label = new QLabel(tr("Please click the start button to detect"));

    list = new QListWidget;

    progress_bar = new QProgressBar;
    progress_bar->setMinimum(0);
    progress_bar->setMaximum(0);
    progress_bar->hide();

    detect_button = new QPushButton(tr("Start"));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(progress_bar);
    hlayout->addWidget(detect_button);


    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(info_label);
    layout->addWidget(list);
    layout->addLayout(hlayout);

    setLayout(layout);

    connect(detect_button, SIGNAL(clicked()), this, SLOT(startDetection()));
    connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(chooseAddress(QListWidgetItem*)));
}

void IrcDetectorDialog::startDetection(){
    detect_button->setEnabled(false);
    info_label->setText(tr("There will be 10+ seconds when connect to the remote host, please wait"));
    progress_bar->show();

    detector = new IrcDetector;
    detector->setParent(this);

    connect(this, SIGNAL(rejected()), detector, SLOT(stop()));
    connect(detector, SIGNAL(server_connected()), this, SLOT(onServerConnected()));
    connect(detector, SIGNAL(detected(QString)), this, SLOT(addNick(QString)));

    detector->detect();
}

void IrcDetectorDialog::onServerConnected(){
    progress_bar->hide();
    info_label->setText(tr("Server connected, double click the item can copy the address to clipboard"));
}

void IrcDetectorDialog::chooseAddress(QListWidgetItem *item){   
    QString nick = item->data(Qt::UserRole).toString();
    QString address = detector->getAddr(nick);
    QApplication::clipboard()->setText(address);

    info_label->setText(tr("Address %1 is copied to clipboard now").arg(address));
}

void IrcDetectorDialog::addNick(const QString &nick){
    ServerInfoStruct info;
    if(detector->getInfo(nick, info)){
        QString address = detector->getAddr(nick);

        QString label = QString("%1 [%2]").arg(info.Name).arg(address);
        QListWidgetItem *item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, nick);

        list->addItem(item);
    }
}
