#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startscene.h"
#include "roomscene.h"
#include "server.h"
#include "client.h"
#include "generaloverview.h"
#include "cardoverview.h"

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QVariant>
#include <QMessageBox>
#include <QGLWidget>
#include <QTime>
#include <QProcess>
#include <QCheckBox>

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
    :QMainWindow(parent), ui(new Ui::MainWindow), role_combobox(NULL)
{
    ui->setupUi(this);

    Sanguosha = new Engine(this);
    Config.init();

    // initialize random seed for later use
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    connection_dialog = new ConnectionDialog(this);
    connect(ui->actionStart_Game, SIGNAL(triggered()), connection_dialog, SLOT(show()));    
    connect(connection_dialog, SIGNAL(accepted()), this, SLOT(startConnection()));

    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    ui->actionEnable_Hotkey->setChecked(Config.EnableHotKey);

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

void MainWindow::createSkillButtons(const Player *player){
    QStatusBar *status_bar = statusBar();
    const General *general = player->getAvatarGeneral();

    QObjectList skills = general->getSkills();
    foreach(QObject *skill_obj, skills){
        Skill *skill = qobject_cast<Skill*>(skill_obj);
        QPushButton *button = new QPushButton(Sanguosha->translate(skill->objectName()));
        if(skill->isCompulsory()){
            button->setText(button->text() + tr("[Compulsory]"));
            button->setDisabled(true);
        }

        if(skill->isLordSkill()){
            button->setText(button->text() + tr("[Lord Skill]"));
        }

        status_bar->addPermanentWidget(button);
        if(skill->isFrequent()){
            QCheckBox *checkbox = new QCheckBox(tr("Auto use"));
            checkbox->setChecked(true);
            status_bar->addPermanentWidget(checkbox);
        }

        if(skill->isToggleable())
            button->setCheckable(true);
    }
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

    connect(client, SIGNAL(error_message(QString)), SLOT(networkError(QString)));
    connect(client, SIGNAL(connected()), SLOT(enterRoom()));
}

void MainWindow::networkError(const QString &error_msg){
    QMessageBox::warning(this, tr("Network error"), error_msg);
}

void MainWindow::enterRoom(){
    ui->actionStart_Game->setEnabled(false);
    ui->actionStart_Server->setEnabled(false);

    Client *client = qobject_cast<Client*>(sender());
    const Player *player = client->getPlayer();

    // add skill buttons
    createSkillButtons(player);

    // add role combobox
    role_combobox = new QComboBox;
    role_combobox->addItem(tr("Your role"));
    role_combobox->addItem(tr("Unknown"));
    statusBar()->addPermanentWidget(role_combobox);
    connect(player, SIGNAL(role_changed(QString)), this, SLOT(updateRoleCombobox(QString)));

    RoomScene *room_scene = new RoomScene(client, 2);
    ui->actionView_Discarded->setEnabled(true);
    connect(ui->actionView_Discarded, SIGNAL(triggered()), room_scene, SLOT(viewDiscards()));

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

void MainWindow::updateRoleCombobox(const QString &new_role){
    role_combobox->setItemText(1, Sanguosha->translate(new_role));    
    role_combobox->setItemIcon(1, QIcon(QString(":/images/roles/%1.png").arg(new_role)));
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
    QMessageBox::about(this, tr("About QSanguosha"),
                       tr("This is the open source clone of the popular <b>Sanguosha</b> game, totally written in C++ Qt GUI framework"));
}
