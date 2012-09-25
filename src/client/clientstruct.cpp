#include "clientstruct.h"
#include "engine.h"
#include "client.h"
#include "settings.h"

ServerInfoStruct ServerInfo;

#include <QFormLayout>
#include <QLabel>
#include <QListWidget>
#include <QCheckBox>

time_t ServerInfoStruct::getCommandTimeout(QSanProtocol::CommandType command, QSanProtocol::ProcessInstanceType instance)
{
    time_t timeOut;
    if (OperationTimeout == 0) return 0;
    else if (command == QSanProtocol::S_COMMAND_CHOOSE_GENERAL)
    {
        timeOut = Config.S_CHOOSE_GENERAL_TIMEOUT * 1000;
    }
    else if (command == QSanProtocol::S_COMMAND_SKILL_GUANXING)
    {
        timeOut = Config.S_GUANXING_TIMEOUT * 1000;
    }
    else
    {
        timeOut = OperationTimeout * 1000;
    }
    if (instance == QSanProtocol::S_SERVER_INSTANCE)
        timeOut += Config.S_SERVER_TIMEOUT_GRACIOUS_PERIOD;
    return timeOut;
}

bool ServerInfoStruct::parse(const QString &str){
    QRegExp rx("(.*):(@?\\w+):(\\d+):([+\\w]*):([FSCTBHAM123]*)");
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
        if(package->inherits("Scenario"))
            continue;

        QString package_name = package->objectName();
        if(ban_packages.contains(package_name))
            package_name = "!" + package_name;

        Extensions << package_name;
    }

    QString flags = texts.at(5);

    FreeChoose = flags.contains("F");
    Enable2ndGeneral = flags.contains("S");
    EnableScene = flags.contains("C");
    EnableSame = flags.contains("T");
    EnableBasara= flags.contains("B");
    EnableHegemony = flags.contains("H");
    EnableAI = flags.contains("A");
    DisableChat = flags.contains("M");

    if(flags.contains("1"))
        MaxHPScheme = 1;
    else if(flags.contains("2"))
        MaxHPScheme = 2;
    else if(flags.contains("3"))
        MaxHPScheme = 3;
    else
        MaxHPScheme = 0;

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
    scene_label = new QLabel;
    same_label = new QLabel;
    basara_label = new QLabel;
    hegemony_label = new QLabel;
    free_choose_label = new QLabel;
    enable_ai_label = new QLabel;
    time_limit_label = new QLabel;
    max_hp_label = new QLabel;

    list_widget = new QListWidget;
    list_widget->setViewMode(QListView::IconMode);
    list_widget->setMovement(QListView::Static);

    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Server name"), name_label);
    layout->addRow(tr("Address"), address_label);
    layout->addRow(tr("Port"), port_label);
    layout->addRow(tr("Game mode"), game_mode_label);
    layout->addRow(tr("Player count"), player_count_label);
    layout->addRow(tr("2nd general mode"), two_general_label);
    layout->addRow(tr("Scene Mode"), scene_label);
    layout->addRow(tr("Same Mode"), same_label);
    layout->addRow(tr("Basara Mode"), basara_label);
    layout->addRow(tr("Hegemony Mode"), hegemony_label);
    layout->addRow(tr("Max HP scheme"), max_hp_label);
    layout->addRow(tr("Free choose"), free_choose_label);
    layout->addRow(tr("Enable AI"), enable_ai_label);
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
    scene_label->setText(info.EnableScene ? tr("Enabled") : tr("Disabled"));
    same_label->setText(info.EnableSame ? tr("Enabled") : tr("Disabled"));
    basara_label->setText(info.EnableBasara ? tr("Enabled") : tr("Disabled"));
    hegemony_label->setText(info.EnableHegemony ? tr("Enabled") : tr("Disabled"));

    if(info.Enable2ndGeneral){
        switch(info.MaxHPScheme){
        case 0: max_hp_label->setText(tr("Sum - 3")); break;
        case 1: max_hp_label->setText(tr("Minimum")); break;
        case 2: max_hp_label->setText(tr("Average")); break;
        }
    }else{
        max_hp_label->setText(tr("2nd general is disabled"));
        max_hp_label->setEnabled(false);
    }

    free_choose_label->setText(info.FreeChoose ? tr("Enabled") : tr("Disabled"));
    enable_ai_label->setText(info.EnableAI ? tr("Enabled") : tr("Disabled"));

    if(info.OperationTimeout == 0)
        time_limit_label->setText(tr("No limit"));
    else
        time_limit_label->setText(tr("%1 seconds").arg(info.OperationTimeout));

    list_widget->clear();

    static QIcon enabled_icon("image/system/enabled.png");
    static QIcon disabled_icon("image/system/disabled.png");

    foreach(QString extension, info.Extensions){
        bool checked = ! extension.startsWith("!");
        if(!checked)
            extension.remove("!");

        QString package_name = Sanguosha->translate(extension);
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
    scene_label->clear();
    same_label->clear();
    basara_label->clear();
    hegemony_label->clear();
    free_choose_label->clear();
    time_limit_label->clear();
    list_widget->clear();
}
