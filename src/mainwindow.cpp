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
    int i;
    for(i=0; i<actions.size(); i++){
        start_scene->addButton(actions[i], i);
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
    server->start();
}
