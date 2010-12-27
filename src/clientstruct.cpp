#include "clientstruct.h"
#include "engine.h"
#include "client.h"
#include "settings.h"

ServerInfoStruct ServerInfo;

#include <QFormLayout>
#include <QLabel>
#include <QListWidget>
#include <QCheckBox>

bool ServerInfoStruct::parse(const QString &str){
    QRegExp rx("(.*):(@?\\w+):(\\d+):([+\\w]*):([FS]*)");
    if(!rx.exactMatch(str)){
        // older version, just take the player count
        int count = str.split(":").at(1).toInt();
        GameMode = QString("%1p").arg(count, 2, 10, QChar('0'));

        return false;
    }

    QStringList texts = rx.capturedTexts();

    QString server_name = texts.at(1);
    Name = QString::fromUtf8(QByteArray::fromBase64(server_name.toAscii()));

    GameMode = texts.at(2);
    OperationTimeout = texts.at(3).toInt();

    QStringList ban_packages = texts.at(4).split("+");
    QList<const Package *> packages = Sanguosha->findChildren<const Package *>();
    foreach(const Package *package, packages){
        if(package->inherits("Scenario") || package->inherits("ChallengeModeSet"))
            continue;

        QString package_name = package->objectName();
        Extensions.insert(package_name, ! ban_packages.contains(package_name));
    }

    QString flags = texts.at(5);

    FreeChoose = flags.contains("F");
    Enable2ndGeneral = flags.contains("S");

    return true;
}

ServerInfoWidget::ServerInfoWidget(bool show_lack)
{
    name_label = new QLabel;
    address_label = new QLabel;
    port_label = new QLabel;
    game_mode_label = new QLabel;
    player_count_label = new QLabel;
    two_general_label = new QLabel;
    free_choose_label = new QLabel;
    time_limit_label = new QLabel;

    list_widget = new QListWidget;

    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Server name"), name_label);
    layout->addRow(tr("Address"), address_label);
    layout->addRow(tr("Port"), port_label);
    layout->addRow(tr("Game mode"), game_mode_label);
    layout->addRow(tr("Player count"), player_count_label);
    layout->addRow(tr("2nd general mode"), two_general_label);
    layout->addRow(tr("Free choose"), free_choose_label);
    layout->addRow(tr("Operation time"), time_limit_label);
    layout->addRow(tr("Extension packages"), list_widget);

    if(show_lack){
        lack_label = new QLabel;
        layout->addRow(tr("Lack"), lack_label);
    }else
        lack_label = NULL;

    setLayout(layout);
}

void ServerInfoWidget::fill(const ServerInfoStruct &info, const QString &address){
    name_label->setText(info.Name);
    address_label->setText(address);
    game_mode_label->setText(Sanguosha->getModeName(info.GameMode));
    int player_count = Sanguosha->getPlayerCount(info.GameMode);
    player_count_label->setText(QString::number(player_count));
    port_label->setText(QString::number(Config.ServerPort));
    two_general_label->setText(info.Enable2ndGeneral ? tr("Enabled") : tr("Disabled"));
    free_choose_label->setText(info.FreeChoose ? tr("Enabled") : tr("Disabled"));

    if(info.OperationTimeout == 0)
        time_limit_label->setText(tr("No limit"));
    else
        time_limit_label->setText(tr("%1 seconds").arg(info.OperationTimeout));

    list_widget->clear();

    static QIcon enabled_icon("image/system/enabled.png");
    static QIcon disabled_icon("image/system/disabled.png");

    QMap<QString, bool> extensions = info.Extensions;
    QMapIterator<QString, bool> itor(extensions);
    while(itor.hasNext()){
        itor.next();

        QString package_name = Sanguosha->translate(itor.key());
        bool checked = itor.value();

        QCheckBox *checkbox = new QCheckBox(package_name);
        checkbox->setChecked(checked);

        new QListWidgetItem(checked ? enabled_icon : disabled_icon, package_name, list_widget);
    }
}

void ServerInfoWidget::updateLack(int count){
    if(lack_label){
        QString path = QString("image/system/number/%1.png").arg(count);
        lack_label->setPixmap(QPixmap(path));
    }
}

void ServerInfoWidget::clear(){
    name_label->clear();
    address_label->clear();
    port_label->clear();
    game_mode_label->clear();
    player_count_label->clear();
    two_general_label->clear();
    free_choose_label->clear();
    time_limit_label->clear();
    list_widget->clear();
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
