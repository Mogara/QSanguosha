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

ServerInfoWidget::ServerInfoWidget(const ServerInfoStruct &info, const QString &address)
{
    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Server name"), new QLabel(info.Name));
    layout->addRow(tr("Address"), new QLabel(address));
    layout->addRow(tr("Port"), new QLabel(QString::number(Config.ServerPort)));
    layout->addRow(tr("Player count"), new QLabel(QString::number(info.PlayerCount)));
    layout->addRow(tr("2nd general mode"), new QLabel(info.Enable2ndGeneral ? tr("Enabled") : tr("Disabled")));
    layout->addRow(tr("Free choose"), new QLabel(info.FreeChoose ? tr("Enabled") : tr("Disabled")));

    QString scenario_label;
    if(Config.Scenario.isEmpty())
        scenario_label = tr("Disabled");
    else
        scenario_label = Sanguosha->translate(Config.Scenario);
    layout->addRow(tr("Scenario mode"), new QLabel(scenario_label));

    QLabel *time_limit = new QLabel;
    if(info.OperationTimeout == 0)
        time_limit->setText(tr("No limit"));
    else
        time_limit->setText(tr("%1 seconds").arg(info.OperationTimeout));
    layout->addRow(tr("Operation time"), time_limit);

    QListWidget *list_widget = new QListWidget;
    QIcon enabled_icon(":/enabled.png");
    QIcon disabled_icon(":/disabled.png");

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

    layout->addRow(tr("Extension packages"), list_widget);

    setLayout(layout);
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
