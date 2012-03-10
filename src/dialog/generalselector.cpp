#include "generalselector.h"
#include "engine.h"
#include "serverplayer.h"

#include <QFile>
#include <QTextStream>

static GeneralSelector *Selector;

GeneralSelector *GeneralSelector::GetInstance(){
    if(Selector == NULL){
        Selector = new GeneralSelector;
        Selector->setParent(Sanguosha);
    }

    return Selector;
}

GeneralSelector::GeneralSelector()
{
    loadFirstGeneralTable();
    loadSecondGeneralTable();
    load3v3Table();
    load1v1Table();
}

QString GeneralSelector::selectFirst(ServerPlayer *player, const QStringList &candidates){
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

    qreal default_value;
    if(role == "loyalist")
        default_value = 6.5;
    else if(role == "rebel")
        default_value = 7.3;
    else
        default_value = 6.3;

    ServerPlayer *lord = player->getRoom()->getLord();
    QString lord_kingdom, suffix = QString();
    if(lord->getGeneral() && lord->getGeneral()->isLord()){
        lord_kingdom = lord->getKingdom();
        suffix = lord->getGeneralName();
    }

    foreach(QString candidate, candidates){
        QString key = QString("%1:%2:%3:%4").arg(candidate).arg(role).arg(index).arg(suffix);
        qreal value = first_general_table.value(key, default_value);

        if(!lord_kingdom.isNull() && (role == "loyalist" || role == "renegade")){
            const General *general = Sanguosha->getGeneral(candidate);
            if(general->getKingdom() == lord_kingdom)
                value += 0.5;
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
        int value = second_general_table.value(key, 3);

        if(value > max){
            max = value;
            max_general = candidate;
        }
    }

    Q_ASSERT(!max_general.isEmpty());

    return max_general;
}

QString GeneralSelector::select3v3(ServerPlayer *player, const QStringList &candidates){
    return selectHighest(priority_3v3_table, candidates, 0);
}

QString GeneralSelector::select1v1(const QStringList &candidates){
    return selectHighest(priority_1v1_table, candidates, 5);
}

QString GeneralSelector::selectHighest(const QHash<QString, int> &table, const QStringList &candidates, int default_value){
    int max = -1;
    QString max_general;

    foreach(QString candidate, candidates){
        int value = table.value(candidate, default_value);

        if(value > max){
            max = value;
            max_general = candidate;
        }
    }

    Q_ASSERT(!max_general.isEmpty());

    return max_general;
}

static bool CompareByMaxHp(const QString &a, const QString &b){
    const General *g1 = Sanguosha->getGeneral(a);
    const General *g2 = Sanguosha->getGeneral(b);

    return g1->getMaxHp() < g2->getMaxHp();
}

QStringList GeneralSelector::arrange3v3(ServerPlayer *player){
    QStringList arranged = player->getSelected();
    qShuffle(arranged);
    arranged = arranged.mid(0, 3);

    qSort(arranged.begin(), arranged.end(), CompareByMaxHp);
    arranged.swap(0, 1);

    return arranged;
}

static bool CompareFunction(const QString &first, const QString &second){
    return Selector->get1v1ArrangeValue(first) < Selector->get1v1ArrangeValue(second);
}

int GeneralSelector::get1v1ArrangeValue(const QString &name){
    int value = priority_1v1_table.value(name, 5);
    if(sacrifice.contains(name))
        value += 100;
    return value;
}

QStringList GeneralSelector::arrange1v1(ServerPlayer *player){
    QStringList arranged = player->getSelected();
    qSort(arranged.begin(), arranged.end(), CompareFunction);
    return arranged.mid(0, 3);
}

void GeneralSelector::loadFirstGeneralTable(){
    loadFirstGeneralTable("loyalist");
    loadFirstGeneralTable("rebel");
    loadFirstGeneralTable("renegade");
}

void GeneralSelector::loadFirstGeneralTable(const QString &role){
    QStringList prefix = Sanguosha->getLords();
    prefix << QString();
    foreach(QString lord, prefix){
        QFile file(QString("etc/%1%2%3.txt").arg(lord).arg((lord.isEmpty()?"":"_")).arg(role));
        if(file.open(QIODevice::ReadOnly)){
            QTextStream stream(&file);
            while(!stream.atEnd()){
                QString name;
                stream >> name;

                int i;
                for(i=0; i<7; i++){
                    qreal value;
                    stream >> value;

                    QString key = QString("%1:%2:%3:%4").arg(name).arg(role).arg(i+2).arg(lord);
                    first_general_table.insert(key, value);
                }
            }

            file.close();
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

        file.close();
    }
}

void GeneralSelector::load3v3Table(){
    QFile file("etc/3v3-priority.txt");
    if(file.open(QIODevice::ReadOnly)){
        QTextStream stream(&file);
        while(!stream.atEnd()){
            QString name;
            int priority;

            stream >> name >> priority;

            priority_3v3_table.insert(name, priority);
        }

        file.close();
    }
}

void GeneralSelector::load1v1Table(){
    QRegExp rx("(\\w+)\\s+(\\d+)\\s*(\\*)?");
    QFile file("etc/1v1-priority.txt");
    if(file.open(QIODevice::ReadOnly)){
        QTextStream stream(&file);
        while(!stream.atEnd()){
            QString line = stream.readLine();
            if(!rx.exactMatch(line))
                continue;

            QStringList texts = rx.capturedTexts();
            QString name = texts.at(1);
            int priority = texts.at(2).toInt();

            priority_1v1_table.insert(name, priority);

            if(!texts.at(3).isEmpty())
                sacrifice << name;
        }

        file.close();
    }
}
