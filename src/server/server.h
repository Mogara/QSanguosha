#ifndef _SERVER_H
#define _SERVER_H

class Room;
class QGroupBox;
class QLabel;
class QRadioButton;

#include "socket.h"
#include "detector.h"
#include "clientstruct.h"

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QButtonGroup>
#include <QComboBox>
#include <QLayoutItem>
#include <QListWidget>
#include <QSplitter>
#include <QTabWidget>
#include <QMultiHash>
#include <QTableWidget>

class Package;

class BanlistDialog: public QDialog {
    Q_OBJECT

public:
    BanlistDialog(QWidget *parent, bool view = false);

private:
    QList<QListWidget *>lists;
    QListWidget *list;
    int item;
    QStringList ban_list;
    QPushButton *add2nd;

private slots:
    void addGeneral(const QString &name);
    void add2ndGeneral(const QString &name);
    void addPair(const QString &first, const QString &second);
    void doAdd2ndButton();
    void doAddButton();
    void doRemoveButton();
    void save();
    void saveAll();
    void switchTo(int item);
};

class ServerDialog: public QDialog {
    Q_OBJECT

public:
    ServerDialog(QWidget *parent);
    void ensureEnableAI();
    bool config();

private:
    QWidget *createBasicTab();
    QWidget *createPackageTab();
    QWidget *createAdvancedTab();
    QWidget *createMiscTab();
    QLayout *createButtonLayout();

    QGroupBox *createGameModeBox();

    QLineEdit *server_name_edit;
    QSpinBox *timeout_spinbox;
    QCheckBox *nolimit_checkbox;
    QCheckBox *random_seat_checkbox;
    QCheckBox *enable_cheat_checkbox;
    QCheckBox *free_choose_checkbox;
    QLabel *pile_swapping_label;
    QSpinBox *pile_swapping_spinbox;
    QCheckBox *lord_convert_checkbox;
    QCheckBox *forbid_same_ip_checkbox;
    QCheckBox *disable_chat_checkbox;
    QLabel *hegemony_maxchoice_label;
    QSpinBox *hegemony_maxchoice_spinbox;
    QLineEdit *address_edit;
    QLineEdit *port_edit;
    QSpinBox *game_start_spinbox;
    QSpinBox *nullification_spinbox;
    QCheckBox *minimize_dialog_checkbox;
    QCheckBox *ai_enable_checkbox;
    QSpinBox *ai_delay_spinbox;
    QCheckBox *ai_delay_altered_checkbox;
    QSpinBox *ai_delay_ad_spinbox;
    QCheckBox *surrender_at_death_checkbox;
    QLabel *luck_card_label;
    QSpinBox *luck_card_spinbox;
    QCheckBox *disable_lua_checkbox;

    QButtonGroup *extension_group;
    QButtonGroup *mode_group;

private slots:

    void onOkButtonClicked();
    void onDetectButtonClicked();

    void doCustomAssign();
};

class Scenario;
class ServerPlayer;
class BanIPDialog;

class Server: public QObject {
    Q_OBJECT

public:
    explicit Server(QObject *parent);

    void broadcast(const QString &msg);
    bool listen();
    void daemonize();
    Room *createNewRoom();
    void signupPlayer(ServerPlayer *player);

    friend class BanIPDialog;

private:
    ServerSocket *server;
    Room *current;
    QSet<Room *> rooms;
    QHash<QString, ServerPlayer *> players;
    QSet<QString> addresses;
    QMultiHash<QString, QString> name2objname;

private slots:
    void processNewConnection(ClientSocket *socket);
    void processRequest(const char *request);
    void cleanup();
    void gameOver();

signals:
    void server_message(const QString &);
};

class BanIPDialog: public QDialog {
    Q_OBJECT

public:
    BanIPDialog(QWidget *parent, Server *theserver);

private:
    QListWidget *left;
    QListWidget *right;

    Server *server;
    QList<ServerPlayer *> sp_list;

    void loadIPList();
    void loadBannedList();

private slots:
    void insertClicked();
    void removeClicked();
    void kickClicked();

    void save();
};

#endif

