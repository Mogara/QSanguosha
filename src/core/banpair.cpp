#include "banpair.h"
#include "settings.h"

static QSet<BanPair> BanPairSet;
static QSet<QString> AllBanSet;
static QSet<QString> SecondBanSet;

BanPair::BanPair(){

}

BanPair::BanPair(const QString &first, const QString &second)
    :QPair<QString, QString>(first, second)
{
    if(first > second){
        qSwap(this->first, this->second);
    }
}

Q_DECLARE_METATYPE(BanPair)

bool BanPair::isBanned(const QString &general){
    return AllBanSet.contains(general);
}

bool BanPair::isBanned(const QString &first, const QString &second){
    if(SecondBanSet.contains(second))
        return true;

    if(AllBanSet.contains(first) || AllBanSet.contains(second))
        return true;

    BanPair pair(first, second);
    return BanPairSet.contains(pair);    
}

void BanPair::loadBanPairs(){
    // special cases
    QStringList banlist = Config.value("Banlist/Pairs","").toStringList();

    foreach(QString line, banlist){
        QStringList names = line.split("+");
        if(names.isEmpty())
            continue;

        QString first = names.at(0).trimmed();
        if(names.length() == 2){
            QString second = names.at(1).trimmed();
            if(first.isEmpty())
                SecondBanSet.insert(second);
            else{
                BanPair pair(first, second);
                BanPairSet.insert(pair);
            }
        }
        else if(names.length()==1){
            AllBanSet.insert(first);
        }
    }
}

void BanPair::saveBanPairs(){
    QStringList stream;
    foreach(QString banned, AllBanSet)
        stream << banned;
    foreach(QString banned, SecondBanSet)
        stream << QString("+%1").arg(banned);
    foreach(BanPair pair, BanPairSet)
        stream << QString("%1+%2").arg(pair.first, pair.second);
    Config.setValue("Banlist/Pairs", stream);
}

const QSet<QString> BanPair::getAllBanSet(){
    return AllBanSet;
}

const QSet<QString> BanPair::getSecondBanSet(){
    return SecondBanSet;
}

const QSet<BanPair> BanPair::getBanPairSet(){
    return BanPairSet;
}
