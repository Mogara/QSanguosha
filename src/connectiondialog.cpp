#include "connectiondialog.h"
#include "ui_connectiondialog.h"
#include "settings.h"

#include <QTcpSocket>

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);

    ui->nameLineEdit->setText(Config.UserName);
    ui->hostLineEdit->setText("127.0.0.1");
    ui->portLineEdit->setValidator(new QIntValidator(0, 65535, ui->portLineEdit));
    ui->portLineEdit->setText(QString::number(Config.Port));

    // do connection
//    QTcpSocket *socket = new QTcpSocket(this);
//    socket->connectToHost("127.0.0.1", Config.Port);
//    connect(socket, SIGNAL(connected()), this, SLOT(enterRoom()));
//    socket->waitForConnected();
}

ConnectionDialog::~ConnectionDialog()
{
    delete ui;
}
