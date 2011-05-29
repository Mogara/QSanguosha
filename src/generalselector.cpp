#include "generalselector.h"
#include "engine.h"
#include "serverplayer.h"

#include <QFile>
#include <QTextStream>

GeneralSelector *GeneralSelector::GetInstance(){
    static GeneralSelector *selector;
    if(selector == NULL){
        selector = new GeneralSelector;
        selector->setParent(Sanguosha);
    }

    return selector;
}

GeneralSelector::GeneralSelector()
{
    loadFirstGeneralTable();
    loadSecondGeneralTable();
}

QString GeneralSelector::selectFirst(ServerPlayer *player, const QStringList &candidates){
    QString lord_kingdom = player->getRoom()->getLord()->getKingdom();
    QString role = player->getRole();
    int seat = player->getSeat();
    int player_count = Sanguosha->getPlayerCount(player->getRoom()->getMode());
    int index = seat;
    if(player_count < 8 && seat == player_count)
        index = 8;
    else if(player_count > 8 && seat > 8)
        index = 8;

    int max = -1;
    QString max_general;

    foreach(QString candidate, candidates){
        QString key = QString("%1:%2:%3").arg(candidate).arg(role).arg(index);
        qreal value = first_general_table.value(key, 5);

        if(role == "loyalist" || role == "renegade"){
            const General *general = Sanguosha->getGeneral(candidate);
            if(general->getKingdom() == lord_kingdom)
                value += 1.5;
        }

        if(value > max){
            max = value;
            max_general = candidate;
        }
    }

    Q_ASSERT(!max_general.isEmpty());

    return max_general;
}

QString GeneralSelector::selectSecond(ServerPlayer *player, const QStringList &candidates){
    QString first = player->getGeneralName();

    int max = -1;
    QString max_general;

    foreach(QString candidate, candidates){
        QString key = QString("%1+%2").arg(first).arg(candidate);
        int value = second_general_table.value(key, 2);

        if(value > max){
            max = value;
            max_general = candidate;
        }
    }

    Q_ASSERT(!max_general.isEmpty());

    return max_general;
}

void GeneralSelector::loadFirstGeneralTable(){
    loadFirstGeneralTable("loyalist");
    loadFirstGeneralTable("rebel");
    loadFirstGeneralTable("renegade");
}

void GeneralSelector::loadFirstGeneralTable(const QString &role){
    QFile file(QString("etc/%1.txt").arg(role));
    if(file.open(QIODevice::ReadOnly)){
        QTextStream stream(&file);
        while(!stream.atEnd()){
            QString name;
            stream >> name;

            int i;
            for(i=0; i<7; i++){
                qreal value;
                stream >> value;

                QString key = QString("%1:%2:%3").arg(name).arg(role).arg(i+2);
                first_general_table.insert(key, value);
            }
        }
    }
}

void GeneralSelector::loadSecondGeneralTable(){
    QRegExp rx("(\\w+)\\s+(\\w+)\\s+(\\d+)");
    QFile file("etc/double-generals.txt");
    if(file.open(QIODevice::ReadOnly)){
        QTextStream stream(&file);
        while(!stream.atEnd()){
            QString line = stream.readLine();
            if(!rx.exactMatch(line))
                continue;

            QStringList texts = rx.capturedTexts();
            QString first = texts.at(1);
            QString second = texts.at(2);
            int value = texts.at(3).toInt();

            QString key = QString("%1+%2").arg(first).arg(second);
            second_general_table.insert(key, value);
        }
    }
}
