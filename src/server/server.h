#ifndef SERVER_H
#define SERVER_H

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

class Package;

class Select3v3GeneralDialog: public QDialog{
    Q_OBJECT

public:
    Select3v3GeneralDialog(QDialog *parent);

private:
    QTabWidget *tab_widget;
    QSet<QString> ex_generals;

    void fillTabWidget();
    void fillListWidget(QListWidget *list, const Package *pack);

private slots:
    void save3v3Generals();
    void toggleCheck();
};

class BanlistDialog: public QDialog{
    Q_OBJECT

public:
    BanlistDialog(QWidget *parent, bool view = false);

private:
    QList<QListWidget *>lists;
    QListWidget * list;
    int item;
    QStringList ban_list;
    QPushButton* add2nd;

private slots:
    void addGeneral(const QString &name);
    void add2ndGeneral(const QString &name);
    void addPair(const QString &first, const QString& second);
    void doAdd2ndButton();
    void doAddButton();
    void doRemoveButton();
    void save();
    void saveAll();
    void switchTo(int item);
};

class QNetworkReply;

class ServerDialog: public QDialog{
    Q_OBJECT

public:
    ServerDialog(QWidget *parent);
    void ensureEnableAI();
    bool config();
    bool isPCConsole() {return pc_console;}

private:
    QWidget *createBasicTab();
    QWidget *createPackageTab();
    QWidget *createAdvancedTab();
    QWidget *createCheatTab();
    QWidget *createAITab();
    QLayout *createButtonLayout();

    QGroupBox *createGameModeBox();
    QGroupBox *createChangbanSlopeBox();
    QGroupBox *create3v3Box();
    QPushButton *ok_button;
    QCheckBox *minimize_checkbox;

    QLineEdit *server_name_edit;
    QSpinBox *timeout_spinbox;
    QCheckBox *nolimit_checkbox;
    QCheckBox *contest_mode_checkbox;
    QCheckBox *random_seat_checkbox;
    QSpinBox *swap_spinbox;
    QSpinBox *maxchoice_spinbox;
    QCheckBox *forbid_same_ip_checkbox;
    QCheckBox *disable_chat_checkbox;
    QCheckBox *second_general_checkbox;
    QCheckBox *nolordskill_checkbox;
    QCheckBox *reincarnation_checkbox;
    QCheckBox *reinca_unchange_checkbox;
    QCheckBox *scene_checkbox;	//changjing
    QCheckBox *same_checkbox;
    QCheckBox *endless_checkbox;
    QSpinBox *endless_timebox;
    QCheckBox *basara_checkbox;
    QCheckBox *hegemony_checkbox;
    QLabel *max_hp_label;
    QComboBox *max_hp_scheme_combobox;
    QCheckBox *announce_ip_checkbox;
    QComboBox *scenario_combobox;
    QComboBox *mini_scene_combobox;
    QPushButton *mini_scene_button;
    QLineEdit *address_edit;
    QLineEdit *port_edit;
    QCheckBox *cheat_enable_checkbox;
    QCheckBox *free_choose_generals_checkbox;
    QCheckBox *free_choose_cards_checkbox;
    QCheckBox *free_assign_checkbox;
    QCheckBox *free_assign_self_checkbox;
    QCheckBox *free_discard_checkbox;
    QCheckBox *gambling_cards_checkbox;
    QCheckBox *free_change_general_checkbox;
    QCheckBox *free_undead_checkbox;
    QCheckBox *hands_up_checkbox;
    QCheckBox *ai_enable_checkbox;
    QCheckBox *role_predictable_checkbox;
    QCheckBox *ai_nickname_checkbox, *ai_chat_checkbox;
    QSpinBox *ai_delay_spinbox;

    QCheckBox *RandomKingdoms_checkbox;
    QRadioButton *standard_3v3_radiobutton;
    QRadioButton *new_3v3_radiobutton;
    QComboBox *role_choose_combobox;
    QCheckBox *exclude_disaster_checkbox;

    QButtonGroup *extension_group;
    QButtonGroup *mode_group;
    bool pc_console;

private slots:
    void onOkButtonClicked();
    void onSerButtonClicked();
    void onPCCButtonClicked();
    void onDetectButtonClicked();
    void onNetworkReplyGot(QNetworkReply *reply);

    void select3v3Generals();
    void edit1v1Banlist();
    void updateButtonEnablility(QAbstractButton* button);
    void updateCheckBoxState(bool toggled);
    void doCheat(bool enable);

    void doCustomAssign();
    void setMiniCheckBox();
};

class Scenario;
class ServerPlayer;

class Server : public QObject{
    Q_OBJECT

public:
    explicit Server(QObject *parent);

    void broadcast(const QString &msg);
    bool listen();
    void daemonize();
    Room *createNewRoom();
    void signupPlayer(ServerPlayer *player);
    void gamesOver();

private:
    ServerSocket *server;
    Room *current;
    QSet<Room *> rooms;
    QHash<QString, ServerPlayer*> players;
    QSet<QString> addresses;
    QMultiHash<QString, QString> name2objname;

private slots:
    void processNewConnection(ClientSocket *socket);
    void processRequest(char *request);
    void cleanup();
    void gameOver();

signals:
    void server_message(const QString &);
};

#endif // SERVER_H
