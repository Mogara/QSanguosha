#include "detector.h"
#include "settings.h"

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

