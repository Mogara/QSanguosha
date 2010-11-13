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

static void detector_join(irc_session_t *session,
                          const char *event,
                          const char *origin,
                          const char **params,
                          unsigned int count)
{
    const char *nick = origin;
    if(Config.IrcNick != nick){
        irc_cmd_notice(session, nick, "giveYourInfo");
    }
}

static void detector_notice(irc_session_t *session,
                            const char *event,
                            const char *origin,
                            const char **params,
                            unsigned int count)
{
    const char *notice = params[1];
    const char *nick = origin;

    IrcDetector *detector = static_cast<IrcDetector*>(irc_get_ctx(session));
    if(notice[0] == '@'){
        int count = atoi(notice + 1);
        detector->askPerson(nick, count);
    }else
        detector->setInfoMap(nick, notice);
}

static void detector_part(irc_session_t *session,
                          const char *event,
                          const char *origin,
                          const char **params,
                          unsigned int count)
{
    IrcDetector *detector = static_cast<IrcDetector*>(irc_get_ctx(session));
    detector->emitParted(origin);
}

IrcDetector::IrcDetector(){
    WORD wVersionRequested = MAKEWORD (1, 1);
    WSADATA wsaData;

    WSAStartup (wVersionRequested, &wsaData);

    irc_callbacks_t callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.event_connect = detector_connect;
    callbacks.event_dcc_chat_req = detector_dcc_callback;
    callbacks.event_join = detector_join;
    callbacks.event_notice = detector_notice;
    callbacks.event_part = detector_part;
    callbacks.event_quit = detector_part;

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
    nick2info[nick].address = addr;
}

void IrcDetector::setInfoMap(const char *nick, const char *server_info){
    ServerFullInfo &info = nick2info[nick];
    if(info.parse(server_info)){
        nick2info.insert(nick, info);

        if(!info.address.isEmpty()){
            emit detected(nick);
        }
    }
}

const ServerFullInfo &IrcDetector::getInfo(const QString &nick){
    return nick2info[nick];
}

void IrcDetector::clearMap(){
    nick2info.clear();
}

void IrcDetector::emitConnected(){
    emit server_connected();
}

void IrcDetector::emitParted(const char *nick){
    if(nick2info.contains(nick)){
        nick2info.remove(nick);
        emit parted(nick);
    }
}

void IrcDetector::askPerson(const char *nick, int count){
    nick2info[nick].lack = count;

    emit person_asked(nick, count);
}

IrcDetectorDialog::IrcDetectorDialog(){
    setWindowTitle(tr("Detect available server's addresses at WAN"));

    detector = NULL;

    info_label = new QLabel(tr("Please click the start button to detect"));

    list = new QListWidget;
    info_widget = new ServerInfoWidget(true);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(list);
    hlayout->addWidget(info_widget);

    progress_bar = new QProgressBar;
    progress_bar->setMinimum(0);
    progress_bar->setMaximum(0);
    progress_bar->hide();

    detect_button = new QPushButton(tr("Start"));

    QHBoxLayout *button_layout = new QHBoxLayout;
    button_layout->addStretch();
    button_layout->addWidget(progress_bar);
    button_layout->addWidget(detect_button);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(info_label);
    layout->addLayout(hlayout);
    layout->addLayout(button_layout);

    setLayout(layout);

    connect(detect_button, SIGNAL(clicked()), this, SLOT(startDetection()));
    connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(copyAddress(QListWidgetItem*)));
    connect(list, SIGNAL(itemSelectionChanged()), this, SLOT(updateServerInfo()));
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
    connect(detector, SIGNAL(parted(QString)), this, SLOT(removeNick(QString)));
    connect(detector, SIGNAL(person_asked(QString,int)), this, SLOT(updateLack(QString,int)));

    detector->detect();
}

void IrcDetectorDialog::onServerConnected(){
    progress_bar->hide();
    info_label->setText(tr("Server connected, double click the item can copy the address to clipboard"));
}

void IrcDetectorDialog::copyAddress(QListWidgetItem *item){
    QString nick = item->data(Qt::UserRole).toString();
    const ServerFullInfo &info = detector->getInfo(nick);
    QString address = info.address;
    QApplication::clipboard()->setText(address);

    info_label->setText(tr("Address %1 is copied to clipboard now").arg(address));
}

void IrcDetectorDialog::updateServerInfo(){
    QListWidgetItem *item = list->currentItem();
    if(item){
        QString nick = item->data(Qt::UserRole).toString();
        const ServerFullInfo &info = detector->getInfo(nick);
        info_widget->fill(info, info.address);
        if(info.lack != 0){
            info_widget->updateLack(info.lack);
        }
    }else
        info_widget->clear();
}

void IrcDetectorDialog::addNick(const QString &nick){
    const ServerFullInfo &info = detector->getInfo(nick);

    QString address = info.address;
    QString label = QString("%1 [%2]").arg(info.Name).arg(address);
    QListWidgetItem *item = new QListWidgetItem(label);
    item->setData(Qt::UserRole, nick);

    list->addItem(item);
}

void IrcDetectorDialog::removeNick(const QString &nick){
    int i, n = list->count();
    for(i=0; i<n; i++){
        QListWidgetItem *item = list->item(i);
        QString item_nick = item->data(Qt::UserRole).toString();
        if(item_nick == nick){
            list->takeItem(i);
            break;
        }
    }
}

void IrcDetectorDialog::updateLack(const QString &nick, int lack){
    QListWidgetItem *item = list->currentItem();    
    if(item){
        QString item_nick = item->data(Qt::UserRole).toString();
        if(item_nick == nick)
            info_widget->updateLack(lack);
    }
}
