#include "detector.h"
#include "settings.h"

#include <QApplication>

UdpDetector::UdpDetector()
{
    socket = new QUdpSocket(this);

    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadReady()));
}

void UdpDetector::detect(){
    socket->bind(Config.DetectorPort, QUdpSocket::ShareAddress);

    const char *ask_str = "whoIsServer";

    socket->writeDatagram(ask_str,
                          strlen(ask_str) + 1,
                          QHostAddress::Broadcast,
                          Config.ServerPort);
}

void UdpDetector::stop(){
    socket->close();
}

void UdpDetector::onReadReady(){
    while(socket->hasPendingDatagrams()){
        QHostAddress from;
        QByteArray data;
        data.resize(socket->pendingDatagramSize());
        socket->readDatagram(data.data(), data.size(), &from);

        QString server_name = QString::fromUtf8(data);
        emit detected(server_name, from.toString());
    }
}

// ----------------------------------

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

static IrcDetector *IrcDetectorInstance = NULL;

IrcDetector *IrcDetector::GetInstance(){
    if(IrcDetectorInstance == NULL)
        IrcDetectorInstance = new IrcDetector;

    return IrcDetectorInstance;
}

IrcDetector::IrcDetector(){
    WORD wVersionRequested = MAKEWORD (1, 1);
    WSADATA wsaData;

    WSAStartup (wVersionRequested, &wsaData);

    irc_callbacks_t callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.event_connect = detector_connect;
    callbacks.event_dcc_chat_req = detector_dcc_callback;
    callbacks.event_privmsg = detector_privmsg;

    session = irc_create_session(&callbacks);
    irc_set_ctx(session, this);
    irc_option_set(session, LIBIRC_OPTION_STRIPNICKS);

    connect(qApp, SIGNAL(lastWindowClosed()), this, SLOT(stop()));
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
    irc_destroy_session(session);
}

void IrcDetector::setAddrMap(const char *nick, const char *addr){
    nick2addr.insert(nick, addr);
}

void IrcDetector::setInfoMap(const char *nick, const char *server_info){
    ServerInfoStruct info;
    if(info.parse(server_info)){
        nick2info.insert(nick, info);

        if(nick2addr.contains(nick)){
            QString addr = nick2addr.value(nick);
            emit detected(info.Name, addr);
        }
    }
}

void IrcDetector::clearMap(){
    nick2addr.clear();
    nick2info.clear();
}

void IrcDetector::emitConnected(){
    emit server_connected();
}
