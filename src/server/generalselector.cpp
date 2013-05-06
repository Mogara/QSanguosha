#include "generalselector.h"
#include "engine.h"
#include "serverplayer.h"

#include <QFile>
#include <QTextStream>

static GeneralSelector *Selector;

GeneralSelector *GeneralSelector::getInstance() {
    if (Selector == NULL) {
        Selector = new GeneralSelector;
        //@todo: this setParent is illegitimate in QT and is equivalent to calling
        // setParent(NULL). So taking it off at the moment until we figure out
        // a way to do it.
        //Selector->setParent(Sanguosha);
    }

    return Selector;
}

GeneralSelector::GeneralSelector() {
    loadFirstGeneralTable();
    loadSecondGeneralTable();
    load3v3Table();
    load1v1Table();
}

QString GeneralSelector::selectFirst(ServerPlayer *player, const QStringList &candidates) {
    QString role = player->getRole();
    int seat = player->getSeat();
    Q_ASSERT(player->getRoom() != NULL);
    int player_count = Sanguosha->getPlayerCount(player->getRoom()->getMode());
    int index = seat;
    if (player_count < 8 && seat == player_count)
        index = 8;
    else if (player_count > 8 && seat > 8)
        index = 8;

    QString max_general;

    ServerPlayer *lord = player->getRoom()->getLord();
    QString suffix = QString();
    if (lord->getGeneral() && lord->getGeneral()->isLord())
        suffix = lord->getGeneralName();

    QStringList key_list;
    foreach (QString candidate, candidates) {
        QString key = QString("%1:%2:%3:%4").arg(candidate).arg(role).arg(index).arg(suffix);
        key_list << key;
    }
    QStringList choice_list;
    while (!key_list.isEmpty() && choice_list.length() < 6) {
        qreal max = -1;
        QString choice = QString();
        foreach (QString key, key_list) {
            qreal value = first_general_table.value(key, 0.0);
            if (value < 0.001) {
                QString _key = QString("%1:%2:%3:").arg(key.split(":").first()).arg(role).arg(index);
                value = first_general_table.value(_key, 0.0);
            }
            if (value < 0.001) value = 5.0;
            if (value > max) {
                max = value;
                choice = key;
            }
        }
        choice_list << choice;
        key_list.removeOne(choice);
    }

    int rnd = qrand() % 100;
    int total = choice_list.length();
    int prob[6] = {70, 85, 92, 95, 97, 99};
    for (int i = 0; i < 6; i++) {
        if (rnd <= prob[i] || total <= i + 1) {
            max_general = choice_list.at(i).split(":").at(0);
            break;
        }
    }

    Q_ASSERT(!max_general.isEmpty());

    return max_general;
}

QString GeneralSelector::selectSecond(ServerPlayer *player, const QStringList &candidates) {
    QString first = player->getGeneralName();

    int max = -1;
    QString max_general;

    foreach (QString candidate, candidates) {
        QString key = QString("%1+%2").arg(first).arg(candidate);
        int value = second_general_table.value(key, 0);
        if (value == 0) {
            key = QString("%1+%2").arg(candidate).arg(first);
            value = second_general_table.value(key, 3);
        }

        if (value > max) {
            max = value;
            max_general = candidate;
        }
    }

    Q_ASSERT(!max_general.isEmpty());

    return max_general;
}

QString GeneralSelector::select3v3(ServerPlayer *, const QStringList &candidates) {
    return selectHighest(priority_3v3_table, candidates, 5);
}

QString GeneralSelector::select1v1(const QStringList &candidates) {
    return selectHighest(priority_1v1_table, candidates, 5);
}

QString GeneralSelector::selectHighest(const QHash<QString, int> &table, const QStringList &candidates, int default_value) {
    int max = -1;
    QString max_general;

    foreach (QString candidate, candidates) {
        int value = table.value(candidate, default_value);

        if (value > max) {
            max = value;
            max_general = candidate;
        }
    }

    Q_ASSERT(!max_general.isEmpty());

    return max_general;
}

static bool CompareByMaxHp(const QString &a, const QString &b) {
    const General *g1 = Sanguosha->getGeneral(a);
    const General *g2 = Sanguosha->getGeneral(b);

    return g1->getMaxHp() < g2->getMaxHp();
}

QStringList GeneralSelector::arrange3v3(ServerPlayer *player) {
    QStringList arranged = player->getSelected();
    qShuffle(arranged);
    arranged = arranged.mid(0, 3);

    qSort(arranged.begin(), arranged.end(), CompareByMaxHp);
    arranged.swap(0, 1);

    return arranged;
}

static bool CompareFunction(const QString &first, const QString &second) {
    return Selector->get1v1ArrangeValue(first) > Selector->get1v1ArrangeValue(second);
}

int GeneralSelector::get1v1ArrangeValue(const QString &name) {
    int value = priority_1v1_table.value(name, 5);
    if (sacrifice.contains(name))
        value += 1000;
    return value;
}

QStringList GeneralSelector::arrange1v1(ServerPlayer *player) {
    QStringList arranged = player->getSelected();
    qSort(arranged.begin(), arranged.end(), CompareFunction);

    QStringList result;
    int i;
    for (i = 0; i < 3; i++) {
        if (get1v1ArrangeValue(arranged[i]) > 1000) {
            result << arranged[i];
            break;
        }
    }
    if (!result.isEmpty()) {
        int strong = (i == 0) ? 1 : 0;
        int weak = (i == 2) ? 1 : 2;
        result << arranged[weak] << arranged[strong];
    } else {
        result << arranged[1] << arranged[2] << arranged[0];
    }

    return result;
}

void GeneralSelector::loadFirstGeneralTable() {
    loadFirstGeneralTable("loyalist");
    loadFirstGeneralTable("rebel");
    loadFirstGeneralTable("renegade");
}

void GeneralSelector::loadFirstGeneralTable(const QString &role) {
    QStringList prefix = Sanguosha->getLords();
    prefix << QString();
    foreach (QString lord, prefix) {
        QFile file(QString("etc/%1%2%3.txt").arg(lord).arg((lord.isEmpty() ? "" : "_")).arg(role));
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            while (!stream.atEnd()) {
                QString name;
                stream >> name;
                for (int i = 0; i < 7; i++) {
                    qreal value;
                    stream >> value;

                    QString key = QString("%1:%2:%3:%4").arg(name).arg(role).arg(i + 2).arg(lord);
                    first_general_table.insert(key, value);
                }
            }

            file.close();
        }
    }
}

void GeneralSelector::loadSecondGeneralTable() {
    QRegExp rx("(\\w+)\\s+(\\w+)\\s+(\\d+)");
    QFile file("etc/double-generals.txt");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            if (!rx.exactMatch(line))
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

void GeneralSelector::load3v3Table() {
    QFile file("etc/3v3-priority.txt");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString name;
            int priority;

            stream >> name >> priority;

            priority_3v3_table.insert(name, priority);
        }

        file.close();
    }
}

void GeneralSelector::load1v1Table() {
    QRegExp rx("(\\w+)\\s+(\\d+)\\s*(\\*)?");
    QFile file("etc/1v1-priority.txt");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            if (!rx.exactMatch(line))
                continue;

            QStringList texts = rx.capturedTexts();
            QString name = texts.at(1);
            int priority = texts.at(2).toInt();

            priority_1v1_table.insert(name, priority);

            if (!texts.at(3).isEmpty())
                sacrifice << name;
        }

        file.close();
    }
}

