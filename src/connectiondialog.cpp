#include "connectiondialog.h"
#include "ui_connectiondialog.h"
#include "settings.h"
#include "engine.h"
#include "detector.h"

#include <QMessageBox>
#include <QTimer>

static const int ShrinkWidth = 230;
static const int ExpandWidth = 744;

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);

    ui->nameLineEdit->setText(Config.UserName);
    ui->nameLineEdit->setMaxLength(32);

    ui->hostComboBox->addItems(Config.HistoryIPs);
    ui->hostComboBox->lineEdit()->setText(Config.HostAddress);

    ui->portLineEdit->setText(QString::number(Config.ServerPort));
    ui->portLineEdit->setValidator(new QIntValidator(1, 9999, ui->portLineEdit));

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

    Config.UserName = username;
    Config.HostAddress = ui->hostComboBox->lineEdit()->text();
    bool ok;
    int port = ui->portLineEdit->text().toInt(&ok);
    if(port){
        Config.ServerPort = port;
        Config.setValue("ServerPort", Config.ServerPort);
    }

    Config.setValue("UserName", Config.UserName);
    Config.setValue("HostAddress", Config.HostAddress);

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

void ConnectionDialog::on_detectButton_clicked()
{
    DetectorDialog *detector_dialog = new DetectorDialog(this);
    connect(detector_dialog, SIGNAL(address_chosen(QString)), ui->hostComboBox->lineEdit(), SLOT(setText(QString)));

    detector_dialog->exec();
}

// -----------------------------------

DetectorDialog::DetectorDialog(QDialog *parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Detect available server's addresses"));

    method_combobox = new QComboBox;
    method_combobox->addItem(tr("LAN detect"));
    method_combobox->addItem(tr("WAN detect"));
    method_combobox->addItem(tr("Battle platform detect"));

    detect_button = new QPushButton(tr("Refresh"));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(new QLabel(tr("Detect type:")));
    hlayout->addWidget(method_combobox);
    hlayout->addWidget(detect_button);

    list = new QListWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(list);
    layout->addLayout(hlayout);

    setLayout(layout);

    detector = new UdpDetector;
    connect(detect_button, SIGNAL(clicked()), this, SLOT(startDetection()));
    connect(detector, SIGNAL(detected(QString,QString)), this, SLOT(addServerAddress(QString,QString)));

    connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(chooseAddress(QListWidgetItem*)));

    detect_button->click();
}

void DetectorDialog::startDetection(){
    detect_button->setEnabled(false);
    list->clear();
    detector->detect();

    QTimer::singleShot(2000, this, SLOT(stopDetection()));
}

void DetectorDialog::stopDetection(){
    detect_button->setEnabled(true);
    detector->stop();
}

void DetectorDialog::addServerAddress(const QString &server_name, const QString &address){
    QString label = QString("%1 [%2]").arg(server_name).arg(address);
    QListWidgetItem *item = new QListWidgetItem(label);
    item->setData(Qt::UserRole, address);

    list->addItem(item);
}

void DetectorDialog::chooseAddress(QListWidgetItem *item){
    QString address = item->data(Qt::UserRole).toString();

    accept();

    emit address_chosen(address);
}
