#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startscene.h"
#include "roomscene.h"
#include "server.h"

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QVariant>
#include <QMessageBox>
#include <QGLWidget>
#include <QTime>

class FitView : public QGraphicsView
{
public:
    FitView(QGraphicsScene *scene) : QGraphicsView(scene) {
        setSceneRect(Config.Rect);
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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    Config.init();

    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    ui->setupUi(this);

    StartScene *start_scene = new StartScene;
    QList<QAction*> actions;
    actions << ui->actionStart_Game << ui->actionConfigure << ui->actionStart_Server
            << ui->actionGeneral_Preview << ui->actionAcknowledgement << ui->actionExit;

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

void MainWindow::on_actionStart_Game_triggered()
{
    gotoScene(new RoomScene);
}

void MainWindow::on_actionStart_Server_triggered()
{
    Server *server = new Server(this);
    if(!server->start()){
        QMessageBox::warning(this, "Warning", tr("Can not start server!"));
        return;
    }

    ui->actionStart_Game->setEnabled(false);
    ui->actionStart_Server->setEnabled(false);

    StartScene *start_scene = qobject_cast<StartScene *>(scene);
    if(start_scene){
        start_scene->leave();
    }

    QString server_message;
    server_message = tr("Server Address: %1 Port: %2").arg(server->serverAddress().toString()).arg(server->serverPort());
    QGraphicsSimpleTextItem *server_message_item = scene->addSimpleText(server_message, Config.SmallFont);
    server_message_item->setBrush(Qt::white);
    server_message_item->setPos(-180, -250);
}
