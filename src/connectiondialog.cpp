#include "connectiondialog.h"
#include "ui_connectiondialog.h"
#include "settings.h"
#include "engine.h"

static const int ShrinkWidth = 230;
static const int ExpandWidth = 744;

#include <QMessageBox>
#include <QRegExpValidator>

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);

    ui->nameLineEdit->setText(Config.UserName);
    ui->nameLineEdit->setMaxLength(32);

    ui->hostComboBox->addItems(Config.HistoryIPs);
    ui->hostComboBox->lineEdit()->setText(Config.HostAddress);

    ui->portLineEdit->setValidator(new QIntValidator(0, USHRT_MAX, ui->portLineEdit));
    ui->portLineEdit->setText(QString::number(Config.Port));

    ui->connectButton->setFocus();

    const General *avatar_general = Sanguosha->getGeneral(Config.UserAvatar);
    if(avatar_general){
        QPixmap avatar(avatar_general->getPixmapPath("big"));
        ui->avatarPixmap->setPixmap(avatar);
    }

    QList<const General*> generals = Sanguosha->findChildren<const General*>();
    foreach(const General *general, generals){
        QIcon icon(general->getPixmapPath("big"));
        QString text = Sanguosha->translate(general->objectName());
        QListWidgetItem *item = new QListWidgetItem(icon, text, ui->avatarList);
        item->setData(Qt::UserRole, general->objectName());
    }

    ui->avatarList->hide();

    setFixedHeight(height());
    setFixedWidth(ShrinkWidth);
}

ConnectionDialog::~ConnectionDialog()
{
    delete ui;
}

void ConnectionDialog::on_connectButton_clicked()
{
    QString username = ui->nameLineEdit->text();

    if(username.isEmpty()){
        QMessageBox::warning(this, tr("Warning"), tr("The user name can not be empty!"));
        return;
    }

    Config.setValue("UserName", Config.UserName = username);
    Config.setValue("HostAddress", Config.HostAddress = ui->hostComboBox->lineEdit()->text());
    Config.setValue("Port", Config.Port = ui->portLineEdit->text().toUShort());

    accept();
}

void ConnectionDialog::on_changeAvatarButton_clicked()
{
    if(ui->avatarList->isVisible()){
        QListWidgetItem *selected = ui->avatarList->currentItem();
        if(selected)
            on_avatarList_itemDoubleClicked(selected);
        else{
            ui->avatarList->hide();
            setFixedWidth(ShrinkWidth);
        }
    }else{
        ui->avatarList->show();
        setFixedWidth(ExpandWidth);
    }
}

void ConnectionDialog::on_avatarList_itemDoubleClicked(QListWidgetItem* item)
{    
    QString general_name = item->data(Qt::UserRole).toString();
    const General *general = Sanguosha->getGeneral(general_name);
    if(general){
        QPixmap avatar(general->getPixmapPath("big"));
        ui->avatarPixmap->setPixmap(avatar);
        Config.UserAvatar = general_name;
        Config.setValue("UserAvatar", general_name);
        ui->avatarList->hide();

        setFixedWidth(ShrinkWidth);
    }
}

void ConnectionDialog::on_clearHistoryButton_clicked()
{
    ui->hostComboBox->clear();
    ui->hostComboBox->lineEdit()->clear();

    Config.HistoryIPs.clear();
    Config.remove("HistoryIPs");
}
