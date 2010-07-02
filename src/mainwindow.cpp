#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startscene.h"
#include "roomscene.h"
#include "server.h"
#include "client.h"
#include "generaloverview.h"

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QVariant>
#include <QMessageBox>
#include <QGLWidget>
#include <QTime>
#include <QProcess>

class FitView : public QGraphicsView
{
public:
    FitView(QGraphicsScene *scene) : QGraphicsView(scene) {
        setSceneRect(Config.Rect);

        if(Config.UseOpenGL)
            setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
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
    QApplication::setApplicationName(tr("Open Source Sanguosha"));
    Sanguosha = new Engine(this);

    Config.init();
    connection_dialog = new ConnectionDialog(this);
    connect(ui->actionStart_Game, SIGNAL(triggered()), connection_dialog, SLOT(show()));    
    connect(connection_dialog, SIGNAL(accepted()), this, SLOT(startConnection()));

    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    // initialize random seed for later use
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    StartScene *start_scene = new StartScene;
    QList<QAction*> actions;
    actions << ui->actionStart_Game << ui->actionConfigure << ui->actionStart_Server
            << ui->actionGeneral_Overview << ui->actionCard_Overview
            << ui->actionAcknowledgement << ui->actionExit;

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
    resize(Config.value("WindowSize").toSize());
    move(Config.value("WindowPosition").toPoint());
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
    if(!server->isListening()){
        QMessageBox::warning(this, "Warning", tr("Can not start server!"));
        return;
    }

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

    connect(client, SIGNAL(error_message(QString)), SLOT(connectionError(QString)));
    connect(client, SIGNAL(connected()), SLOT(enterRoom()));
}

void MainWindow::connectionError(const QString &error_msg){
    QMessageBox::warning(this, tr("Connection failed"), error_msg);
}

void MainWindow::enterRoom(){
    ui->actionStart_Game->setEnabled(false);
    ui->actionStart_Server->setEnabled(false);

    Client *client = qobject_cast<Client*>(sender());
    gotoScene(new RoomScene(client));
}

void MainWindow::startGameInAnotherInstance(){
    QProcess::startDetached(QApplication::applicationFilePath());
}

void MainWindow::on_actionGeneral_Overview_triggered()
{
    GeneralOverview *overview = new GeneralOverview(this);
    overview->show();
}
