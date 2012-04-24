#include "recorder.h"
#include "client.h"

#include <cstdlib>
#include <cmath>

#include <QFile>
#include <QBuffer>
#include <QMessageBox>
using namespace QSanProtocol;

Recorder::Recorder(QObject *parent)
    :QObject(parent)
{
    watch.start();
}

void Recorder::record(char *line)
{
    recordLine(line);
}

void Recorder::recordLine(const QString &line){
    int elapsed = watch.elapsed();
    if(line.endsWith("\n"))
        data.append(QString("%1 %2").arg(elapsed).arg(line));
    else
        data.append(QString("%1 %2\n").arg(elapsed).arg(line));
}

bool Recorder::save(const QString &filename) const{
    if(filename.endsWith(".txt")){
        QFile file(filename);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text))
            return file.write(data) != -1;
        else
            return false;
    }else if(filename.endsWith(".png")){
        return TXT2PNG(data).save(filename);
    }else
        return false;
}

QImage Recorder::TXT2PNG(QByteArray txtData){
    QByteArray data = qCompress(txtData, 9);
    qint32 actual_size = data.size();
    data.prepend((const char *)&actual_size, sizeof(qint32));

    // actual data = width * height - padding
    int width = ceil(sqrt((double)data.size()));
    int height = width;
    int padding = width * height - data.size();
    QByteArray paddingData;
    paddingData.fill('\0', padding);
    data.append(paddingData);

    QImage image((const uchar *)data.constData(), width, height, QImage::Format_ARGB32);
    return image;
}

Replayer::Replayer(QObject *parent, const QString &filename)
    :QThread(parent), m_isOldVersion(false), m_commandSeriesCounter(1),
      filename(filename), speed(1.0), playing(true)
{
    QIODevice *device = NULL;
    if(filename.endsWith(".png")){
        QByteArray *data = new QByteArray(PNG2TXT(filename));
        QBuffer *buffer = new QBuffer(data);
        device = buffer;
    }else if(filename.endsWith(".txt")){
        QFile *file = new QFile(filename);
        device = file;
    }

    if(device == NULL)
        return;

    if(!device->open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    typedef char buffer_t[1024];

    initCommandPair();
    while(!device->atEnd()){
        buffer_t line;
        memset(line, 0, sizeof(buffer_t));
        device->readLine(line, sizeof(buffer_t));

        char *space = strchr(line, ' ');
        if(space == NULL)
            continue;

        *space = '\0';
        QString cmd = space + 1;
        int elapsed = atoi(line);

        //@todo: There is a serious problem that the old protocol didn't has
        //any tag or type definition for the sent messages, and if the old protocol
        // wants to be translated to the new protocol, there is no information for the package type
        //and client will be confused of showing dialog or not.
        commandTranslation(cmd);

        Pair pair;
        pair.elapsed = elapsed;
        pair.cmd = cmd;

        pairs << pair;
    }

    if(m_isOldVersion){
        QMessageBox::warning(NULL, tr("Warning"), tr("The replay use old protocol"));
    }

    delete device;
}

QByteArray Replayer::PNG2TXT(const QString filename){
    QImage image(filename);
    image = image.convertToFormat(QImage::Format_ARGB32);
    const uchar *imageData = image.bits();
    qint32 actual_size = *(const qint32 *)imageData;
    QByteArray data((const char *)(imageData+4), actual_size);
    data = qUncompress(data);

    return data;
}

QString &Replayer::commandProceed(QString &cmd){
    static QStringList split_flags;
    if(split_flags.isEmpty()){
        split_flags << ":" << "+" << "_" << "->";
    }

    foreach(QString flag, split_flags){
        QStringList messages = cmd.split(flag);
        if(messages.length() > 1){
            QStringList message_analyse;
            foreach(QString message, messages){
                message_analyse << commandProceed(message);
            }
            cmd = "[" + message_analyse.join(",") + "]";
        }
        else if(!cmd.startsWith("\"") && !cmd.startsWith("["))
            cmd = "\"" + cmd +"\"";
    }

    return cmd;
}

QString &Replayer::commandTranslation(QString &cmd){
    foreach(QString command, m_commandMapping.keys()){
        if(cmd.contains(command)){
            m_isOldVersion = true;
            cmd.remove(command);
            cmd.remove(' ');

            commandProceed(cmd);

            CommandType commandEnum = m_commandMapping[command];
            int packetTypeId;
            if(m_packetTypeMapping[S_SERVER_REQUEST].contains(commandEnum))
                packetTypeId = static_cast<int>(S_SERVER_REQUEST);
            else
                packetTypeId = static_cast<int>(S_SERVER_NOTIFICATION);

            cmd = QString("[%1,0,%2,%3,%4]").arg(QString::number(m_commandSeriesCounter++))
                    .arg(QString::number(packetTypeId))
                    .arg(QString::number(static_cast<int>(commandEnum)))
                    .arg(cmd);
        }
    }

    foreach(QString name, m_nameTranslation.keys()){
        if(cmd.contains(name)){
            m_isOldVersion = true;
            cmd.replace(name, m_nameTranslation[name]);
        }
    }

    return cmd;
}

void Replayer::initCommandPair(){
    if(m_nameTranslation.isEmpty()){
        m_nameTranslation["lubu"]               = "lvbu";
        m_nameTranslation["lumeng"]             = "lvmeng";
  //      m_nameTranslation["shuangxiong"]        = "yanliangwenchou";
  //      m_nameTranslation["erzhang"]            = "zhangzhaozhanghong";
  //      m_nameTranslation["shencc"]             = "weiwudi";
    }

    if(m_commandMapping.isEmpty()){
        m_commandMapping["showCard"]            = S_COMMAND_SHOW_CARD;
        m_commandMapping["moveFocus"]           = S_COMMAND_MOVE_FOCUS;
        m_commandMapping["skillInvoked"]        = S_COMMAND_INVOKE_SKILL;
        m_commandMapping["doGongxin"]           = S_COMMAND_SKILL_GONGXIN;
        m_commandMapping["askForGeneral"]       = S_COMMAND_CHOOSE_GENERAL;
        m_commandMapping["askForPlayerChosen"]  = S_COMMAND_CHOOSE_PLAYER;
        m_commandMapping["askForAssign"]        = S_COMMAND_CHOOSE_ROLE;
        m_commandMapping["askForDirection"]     = S_COMMAND_CHOOSE_DIRECTION;
        m_commandMapping["askForExchange"]      = S_COMMAND_EXCHANGE_CARD;
        m_commandMapping["askForSinglePeach"]   = S_COMMAND_ASK_PEACH;
        m_commandMapping["doGuanxing"]          = S_COMMAND_SKILL_GUANXING;
        m_commandMapping["askForYiji"]          = S_COMMAND_SKILL_YIJI;
        m_commandMapping["activate"]            = S_COMMAND_PLAY_CARD;
        m_commandMapping["askForDiscard"]       = S_COMMAND_DISCARD_CARD;
        m_commandMapping["askForSuit"]          = S_COMMAND_CHOOSE_SUIT;
        m_commandMapping["askForKingdom"]       = S_COMMAND_CHOOSE_KINGDOM;
        m_commandMapping["askForCard"]          = S_COMMAND_RESPONSE_CARD;
        m_commandMapping["askForUseCard"]       = S_COMMAND_USE_CARD;
        m_commandMapping["askForChoice"]        = S_COMMAND_MULTIPLE_CHOICE;
        m_commandMapping["askForCardShow"]      = S_COMMAND_SHOW_CARD;
        m_commandMapping["askForAG"]            = S_COMMAND_AMAZING_GRACE;
        m_commandMapping["askForPindian"]       = S_COMMAND_PINDIAN;
        m_commandMapping["askForCardChosen"]    = S_COMMAND_CHOOSE_CARD;
        m_commandMapping["askForOrder"]         = S_COMMAND_CHOOSE_ORDER;
        m_commandMapping["askForRole"]          = S_COMMAND_CHOOSE_ROLE_3V3;
    }

    if(m_packetTypeMapping.isEmpty()){
        QList<CommandType> commands;

        commands << S_COMMAND_CHOOSE_GENERAL
                 << S_COMMAND_CHOOSE_PLAYER
                << S_COMMAND_CHOOSE_ROLE
                << S_COMMAND_CHOOSE_DIRECTION
                << S_COMMAND_EXCHANGE_CARD
                << S_COMMAND_ASK_PEACH
                << S_COMMAND_SKILL_GUANXING
                << S_COMMAND_SKILL_GONGXIN
                << S_COMMAND_SKILL_YIJI
                << S_COMMAND_PLAY_CARD
                << S_COMMAND_DISCARD_CARD
                << S_COMMAND_CHOOSE_SUIT
                << S_COMMAND_CHOOSE_KINGDOM
                << S_COMMAND_RESPONSE_CARD
                << S_COMMAND_USE_CARD
                << S_COMMAND_INVOKE_SKILL
                << S_COMMAND_MULTIPLE_CHOICE
                << S_COMMAND_NULLIFICATION
                << S_COMMAND_SHOW_CARD
                << S_COMMAND_AMAZING_GRACE
                << S_COMMAND_PINDIAN
                << S_COMMAND_CHOOSE_CARD
                << S_COMMAND_CHOOSE_ORDER
                << S_COMMAND_CHOOSE_ROLE_3V3;
        m_packetTypeMapping[S_SERVER_NOTIFICATION] = commands;

        commands.clear();
        commands << S_COMMAND_SHOW_CARD
                << S_COMMAND_MOVE_FOCUS
                << S_COMMAND_INVOKE_SKILL
                << S_COMMAND_SKILL_GONGXIN;
        m_packetTypeMapping[S_SERVER_REQUEST] = commands;
    }
}

int Replayer::getDuration() const{
    return pairs.last().elapsed / 1000.0;
}

qreal Replayer::getSpeed() {
    qreal speed;
    mutex.lock();
    speed = this->speed;
    mutex.unlock();
    return speed;
}

void Replayer::uniform(){
    mutex.lock();

    if(speed != 1.0){
        speed = 1.0;
        emit speed_changed(1.0);
    }

    mutex.unlock();
}

void Replayer::speedUp(){
    mutex.lock();

    if(speed < 6.0){
        qreal inc = speed >= 2.0 ? 1.0 : 0.5;
        speed += inc;
        emit speed_changed(speed);
    }

    mutex.unlock();
}

void Replayer::slowDown(){
    mutex.lock();

    if(speed >= 1.0){
        qreal dec = speed >= 2.0 ? 1.0 : 0.5;
        speed -= dec;
        emit speed_changed(speed);
    }

    mutex.unlock();
}

void Replayer::toggle(){
    playing = !playing;
    if(playing)
        play_sem.release(); // to play
}

void Replayer::run(){
    int last = 0;

    QStringList nondelays;
    nondelays << "addPlayer" << "removePlayer" << "speak";

    foreach(Pair pair, pairs){
        int delay = qMin(pair.elapsed - last, 2500);
        last = pair.elapsed;

        bool delayed = true;
        foreach(QString nondelay, nondelays){
            if(pair.cmd.startsWith(nondelay)){
                delayed = false;
                break;
            }
        }

        if(delayed){
            delay /= getSpeed();

            msleep(delay);
            emit elasped(pair.elapsed / 1000.0);

            if(!playing)
                play_sem.acquire();
        }

        emit command_parsed(pair.cmd);
    }
}

