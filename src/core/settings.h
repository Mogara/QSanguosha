#ifndef SETTINGS_H
#define SETTINGS_H

#include "protocol.h"
#include <QSettings>
#include <QFont>
#include <QRectF>
#include <QPixmap>
#include <QBrush>

class Settings : public QSettings{
    Q_OBJECT

public:
    explicit Settings();
    void init();
    //Get the timeout allowance for a command. Server countdown is more lenient than the client.
    //@param command: type of command
    //@return countdown for command in milliseconds.
    inline time_t getCommandTimeout(QSanProtocol::CommandType command, QSanProtocol::ProcessInstanceType instance)
    {
        time_t timeOut;
        if (OperationNoLimit) return UINT_MAX;
        else if (command == QSanProtocol::S_COMMAND_CHOOSE_GENERAL)
        {
            timeOut = S_CHOOSE_GENERAL_TIMEOUT * 1000;
        }
        else if (command == QSanProtocol::S_COMMAND_SKILL_GUANXING)
        {
            timeOut = S_GUANXING_TIMEOUT * 1000;
        }
        else
        {
            timeOut = OperationTimeout * 1000;
        }
        if (instance = QSanProtocol::S_SERVER_INSTANCE)
            timeOut += S_SERVER_TIMEOUT_GRACIOUS_PERIOD;
        return timeOut;
    }

    const QRectF Rect;
    QFont BigFont;
    QFont SmallFont;
    QFont TinyFont;

    QFont AppFont;
    QFont UIFont;
    QColor TextEditColor;

    // server side
    QString ServerName;
    int CountDownSeconds;
    QString GameMode;
    QStringList BanPackages;
    bool ContestMode;
    bool FreeChoose;
    bool ForbidSIMC;
    bool DisableChat;
    bool FreeAssignSelf;
    bool Enable2ndGeneral;
    bool EnableScene;	//changjing
    bool EnableBasara;
    bool EnableHegemony;
    int MaxHpScheme;
    bool AnnounceIP;
    QString Address;
    bool EnableAI;
    int AIDelay;
    ushort ServerPort;

    // client side
    QString HostAddress;
    QString UserName;
    QString UserAvatar;
    QString Password;
    QStringList HistoryIPs;
    ushort DetectorPort;
    int MaxCards;

    bool FitInView;
    bool EnableHotKey;
    bool EnableMinimizeDialog;
    bool NeverNullifyMyTrick;
    bool EnableAutoTarget;
    int NullificationCountDown;
    int OperationTimeout;
    bool OperationNoLimit;
    bool EnableEffects;
    bool EnableLastWord;
    bool EnableBgMusic;
    float BGMVolume;
    float EffectVolume;

    QString BackgroundBrush;

    // consts
    static const int S_CHOOSE_GENERAL_TIMEOUT;
    static const int S_GUANXING_TIMEOUT;
    static const int S_SURRNDER_REQUEST_MIN_INTERVAL;
    static const int S_PROGRESS_BAR_UPDATE_INTERVAL;
    static const int S_SERVER_TIMEOUT_GRACIOUS_PERIOD;
};

extern Settings Config;

#endif // SETTINGS_H
