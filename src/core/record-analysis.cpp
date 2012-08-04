#include "record-analysis.h"
#include "recorder.h"
#include "settings.h"

#include <QFile>
#include <QMessageBox>

RecAnalysis::RecAnalysis(QString dir){
    initialize(dir);
}

void RecAnalysis::initialize(QString dir){
    QList<QString> records_line;
    if(dir.isEmpty())
        records_line = ClientInstance->getRecords();
    else if(dir.endsWith(".png")){
        QByteArray data = Replayer::PNG2TXT(dir);
        QString record_data(data);
        records_line = record_data.split("\n");
    }
    else if(dir.endsWith(".txt")){
        QFile file(dir);
        if(file.open(QIODevice::ReadOnly)){
            QTextStream stream(&file);
            while(!stream.atEnd()){
                QString line = stream.readLine();
                records_line << line;
            }
        }
    }
    else{
        //Weird, I cannot find the header file of tr function
    //    QMessageBox::warning(NULL, tr("Warning"), tr("The file is unreadable"));
    }

    m_recordMap["sgs1"] = new PlayerRecordStruct;
    m_recordMap["sgs1"]->m_screenName = Config.UserName;
    foreach(QString line, records_line){
        if(line.contains("setup")){
            QRegExp rx("(.*):(\\w+):(\\w+):(.*):(\\w+)");
            if(!rx.exactMatch(line))
                continue;

            QStringList texts = rx.capturedTexts();
            m_recordPackages = texts.at(4).split("+");
            continue;
        }

        if(line.contains("addPlayer")){
            PlayerRecordStruct *player_rec = new PlayerRecordStruct;
            QList<QString> info_assemble = line.split(" ").last().split(":");
            player_rec->m_screenName = QString::fromUtf8(QByteArray::fromBase64(info_assemble.at(1).toAscii()));
            player_rec->m_generalName = info_assemble.at(2);
            m_recordMap[info_assemble.at(0)] = player_rec;

            continue;
        }

        if(line.contains("general")){
            foreach(QString object, m_recordMap.keys()){
                if(line.contains(object)){
                    QString general = line.split(",").last();
                    m_recordMap[object]->m_generalName = general.remove("\"");
                    m_recordMap[object]->m_generalName = general.remove("]");
                }
            }
        }

        if(line.contains("state") && line.contains("robot")){
            foreach(QString object_name, m_recordMap.keys()){
                if(line.contains(object_name)){
                    m_recordMap[object_name]->m_statue = "robot";
                    break;
                }
            }

            continue;
        }

        if(line.contains("hpChange")){
            QList<QString> info_assemble = line.split(" ").last().split(":");
            QString hp = info_assemble.at(1);
            bool ok = false;
            int hp_change = hp.remove(QRegExp("[TF]")).toInt(&ok);
            if(!ok)
                continue;

            if(hp_change < 0)
                m_recordMap[info_assemble.at(0)]->m_damage += -hp_change;
            else
                m_recordMap[info_assemble.at(0)]->m_recover += hp_change;

            continue;
        }

        if(line.contains("role")){
            foreach(QString object, m_recordMap.keys()){
                if(line.contains(object)){
                    if(line.contains("lord")) m_recordMap[object]->m_role = "lord";
                    else if(line.contains("loyalist")) m_recordMap[object]->m_role = "loyalist";
                    else if(line.contains("rebel")) m_recordMap[object]->m_role = "rebel";
                    else if(line.contains("renegade")) m_recordMap[object]->m_role = "renegade";

                    break;
                }
            }

            continue;
        }

        if(line.contains("#Murder")){
            QString object = line.split(":").at(1).split("->").first();
            m_recordMap[object]->m_kill++;

            continue;
        }
    }
}

PlayerRecordStruct *RecAnalysis::getPlayerRecord(const Player *player) const{
    if(m_recordMap.keys().contains(player->objectName()))
        return m_recordMap[player->objectName()];
    else
        return NULL;
}

QMap<QString, PlayerRecordStruct *> RecAnalysis::getRecordMap() const{
    return m_recordMap;
}


PlayerRecordStruct::PlayerRecordStruct()
    :m_statue("online"), m_recover(0), m_damage(0), m_kill(0)
{}
