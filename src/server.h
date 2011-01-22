#ifndef SERVER_H
#define SERVER_H

class Room;
class QGroupBox;
class QLabel;

#include "socket.h"
#include "detector.h"

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QButtonGroup>
#include <QComboBox>
#include <QLayoutItem>

class ServerDialog: public QDialog{
    Q_OBJECT

public:
    ServerDialog(QWidget *parent);
    bool config();

private:
    QLayout *createLeft();
    QLayout *createRight();
    QLayout *createButtonLayout();

    QGroupBox *createGameModeBox();

    QLineEdit *server_name_edit;
    QSpinBox *timeout_spinbox;
    QCheckBox *nolimit_checkbox;
    QCheckBox *free_choose_checkbox;
    QCheckBox *forbid_same_ip_checkbox;
    QCheckBox *second_general_checkbox;
    QComboBox *max_hp_scheme_combobox;
    QCheckBox *announce_ip_checkbox;
    QComboBox *scenario_combobox;
    QComboBox *challenge_combobox;
    QList<QLabel *> challenge_avatars;
    QLineEdit *address_edit;
    QLineEdit *port_edit;
    QCheckBox *ai_enable_checkbox;
    QSpinBox *ai_delay_spinbox;

    QButtonGroup *extension_group;
    QButtonGroup *mode_group;

private slots:
    void onOkButtonClicked();
    void onDetectButtonClicked();
    void onHttpDone(bool error);
    void updateChallengeLabel(int index);
};

class Scenario;

class Server : public QObject{
    Q_OBJECT

public:
    explicit Server(QObject *parent);

    bool listen();
    void daemonize();

private:
    ServerSocket *server;
    Room *current;
    QSet<QString> addresses;

    void createNewRoom();

private slots:
    void processNewConnection(ClientSocket *socket);
    void cleanup();

signals:
    void server_message(const QString &);
};

#endif // SERVER_H
