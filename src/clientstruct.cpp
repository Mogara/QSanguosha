#include "clientstruct.h"
#include "engine.h"
#include "client.h"

ServerInfoStruct ServerInfo;

bool ServerInfoStruct::parse(const QString &str){
    QRegExp rx("(.*):(\\d+):(\\d+):([+\\w]*):(\\w*):([FS]*)");
    if(!rx.exactMatch(str))
        return false;

    QStringList texts = rx.capturedTexts();

    QString server_name = texts.at(1);
    Name = QString::fromUtf8(QByteArray::fromBase64(server_name.toAscii()));

    PlayerCount = texts.at(2).toInt();
    OperationTimeout = texts.at(3).toInt();

    QStringList ban_packages = texts.at(4).split("+");
    QList<const Package *> packages = Sanguosha->findChildren<const Package *>();
    foreach(const Package *package, packages){
        if(package->inherits("Scenario"))
            continue;

        QString package_name = package->objectName();
        Extensions.insert(package_name, ! ban_packages.contains(package_name));
    }

    Scenario = texts.at(5);

    QString flags = texts.at(6);

    FreeChoose = flags.contains("F");
    Enable2ndGeneral = flags.contains("S");

    return true;
}

bool CardMoveStructForClient::parse(const QString &str){
    static QMap<QString, Player::Place> place_map;
    if(place_map.isEmpty()){
        place_map["hand"] = Player::Hand;
        place_map["equip"] = Player::Equip;
        place_map["judging"] = Player::Judging;
        place_map["special"] = Player::Special;
        place_map["_"] = Player::DiscardedPile;
        place_map["="] = Player::DrawPile;
    }

    // example: 12:tenshi@equip->moligaloo@hand
    QRegExp pattern("(-?\\d+):(.+)@(.+)->(.+)@(.+)");
    if(!pattern.exactMatch(str)){
        return false;
    }

    QStringList words = pattern.capturedTexts();

    card_id = words.at(1).toInt();

    if(words.at(2) == "_")
        from = NULL;
    else
        from = ClientInstance->getPlayer(words.at(2));
    from_place = place_map.value(words.at(3), Player::DiscardedPile);

    if(words.at(4) == "_")
        to = NULL;
    else
        to = ClientInstance->getPlayer(words.at(4));
    to_place = place_map.value(words.at(5), Player::DiscardedPile);

    return true;
}
