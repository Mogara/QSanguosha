#include "mainwindow.h"
#include "startscene.h"
#include "roomscene.h"
#include "server.h"
#include "client.h"
#include "generaloverview.h"
#include "cardoverview.h"
#include "ui_mainwindow.h"
#include "audiere.h"

audiere::AudioDevicePtr Device;

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QVariant>
#include <QMessageBox>
#include <QTime>
#include <QProcess>
#include <QCheckBox>

class FitView : public QGraphicsView
{
public:
    FitView(QGraphicsScene *scene) : QGraphicsView(scene) {
        setSceneRect(Config.Rect);
    }

protected:
    void resizeEvent(QResizeEvent *event)
    {
        QGraphicsView::resizeEvent(event);
        if(Config.FitInView)
            fitInView(sceneRect(), Qt::KeepAspectRatio);
    }
};

MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Device = audiere::OpenDevice();
    Sanguosha = new Engine(this);    
    Config.init();

    // initialize random seed for later use
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    connection_dialog = new ConnectionDialog(this);
    connect(ui->actionStart_Game, SIGNAL(triggered()), connection_dialog, SLOT(show()));    
    connect(connection_dialog, SIGNAL(accepted()), this, SLOT(startConnection()));

    config_dialog = new ConfigDialog(this);
    connect(ui->actionConfigure, SIGNAL(triggered()), config_dialog, SLOT(show()));
    connect(config_dialog, SIGNAL(bg_changed()), this, SLOT(changeBackground()));

    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    StartScene *start_scene = new StartScene;    

    QList<QAction*> actions;
    actions << ui->actionStart_Game
            << ui->actionConfigure
            << ui->actionStart_Server
            << ui->actionGeneral_Overview
            << ui->actionCard_Overview
            << ui->actionAbout << ui->actionExit;

    foreach(QAction *action, actions){
        start_scene->addButton(action);
    }

    scene = start_scene;
    FitView *view = new FitView(scene);

    setCentralWidget(view);

//    if(Config.TitleMusic)
//        Config.TitleMusic->play();    

    restoreFromConfig();
}

void MainWindow::restoreFromConfig(){
    resize(Config.value("WindowSize", QSize(1042, 719)).toSize());
    move(Config.value("WindowPosition", QPoint(20,20)).toPoint());

    ui->actionEnable_Hotkey->setChecked(Config.EnableHotKey);
    ui->actionNever_Nullify_My_Trick->setChecked(Config.NeverNullifyMyTrick);
}

void MainWindow::closeEvent(QCloseEvent *){
    Config.setValue("WindowSize", size());
    Config.setValue("WindowPosition", pos());
}

MainWindow::~MainWindow()
{    
    delete ui;
}

void MainWindow::gotoScene(QGraphicsScene *scene){
    QGraphicsView *view = qobject_cast<QGraphicsView *>(centralWidget());
    view->setScene(scene);
    delete this->scene;
    this->scene = scene;
}

void MainWindow::on_actionExit_triggered()
{
    QMessageBox::StandardButton result;
    result = QMessageBox::question(this,
                                   tr("Sanguosha"),
                                   tr("Are you sure to exit?"),
                                   QMessageBox::Ok | QMessageBox::Cancel);
    if(result == QMessageBox::Ok)
        close();
}

void MainWindow::on_actionStart_Server_triggered()
{
    Server *server = new Server(this);
    if(!server->isListening())
        return;    

    ui->actionStart_Game->disconnect();
    connect(ui->actionStart_Game, SIGNAL(triggered()), this, SLOT(startGameInAnotherInstance()));
    ui->actionStart_Server->setEnabled(false);

    StartScene *start_scene = qobject_cast<StartScene *>(scene);
    if(start_scene){
        start_scene->switchToServer(server);
    }
}

void MainWindow::startConnection(){
    Client *client = new Client(this);

    connect(client, SIGNAL(error_message(QString)), SLOT(networkError(QString)));
    connect(client, SIGNAL(server_connected()), SLOT(enterRoom()));
}

void MainWindow::restartConnection(){
    ClientInstance->disconnectFromHost();
    ClientInstance = NULL;

    delete Self;
    Self = NULL;

    startConnection();
}

void MainWindow::networkError(const QString &error_msg){
    QMessageBox::warning(this, tr("Network error"), error_msg);
}

void MainWindow::enterRoom(){
    ui->actionStart_Game->setEnabled(false);
    ui->actionStart_Server->setEnabled(false);

    RoomScene *room_scene = new RoomScene(Config.PlayerCount, this);
    ui->actionView_Discarded->setEnabled(true);
    connect(ui->actionView_Discarded, SIGNAL(triggered()), room_scene, SLOT(viewDiscards()));
    connect(ui->actionView_distance, SIGNAL(triggered()), room_scene, SLOT(viewDistance()));
    connect(room_scene, SIGNAL(restart()), this, SLOT(restartConnection()));

    gotoScene(room_scene);
}

void MainWindow::startGameInAnotherInstance(){
    QProcess::startDetached(QApplication::applicationFilePath(), QStringList());
}

void MainWindow::on_actionGeneral_Overview_triggered()
{
    GeneralOverview *overview = new GeneralOverview(this);
    overview->show();
}

void MainWindow::on_actionCard_Overview_triggered()
{
    CardOverview *overview = new CardOverview(this);
    overview->loadFromAll();
    overview->show();
}

void MainWindow::on_actionEnable_Hotkey_toggled(bool checked)
{
    if(Config.EnableHotKey != checked){
        Config.EnableHotKey = checked;
        Config.setValue("EnableHotKey", checked);
    }
}

void MainWindow::on_actionAbout_triggered()
{
    QString content =  "<center><img src=':/shencc.png'> <br /> </center>";
    content.append(tr("This is the open source clone of the popular <b>Sanguosha</b> game,"
                      "totally written in C++ Qt GUI framework <br />"
                      "My Email: moligaloo@gmail.com"));

    // FIXME: add acknowledgement

    QMessageBox::about(this, tr("About QSanguosha"), content);
}

void MainWindow::on_actionNever_Nullify_My_Trick_toggled(bool checked)
{
    if(Config.NeverNullifyMyTrick != checked){
        Config.NeverNullifyMyTrick = checked;
        Config.setValue("NeverNullifyMyTrick", checked);
    }
}

void MainWindow::changeBackground(){
    if(scene)
        scene->setBackgroundBrush(Config.BackgroundBrush);
}
