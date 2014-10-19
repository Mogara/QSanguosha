/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#include "mainwindow.h"
#include "startscene.h"
#include "roomscene.h"
#include "server.h"
#include "client.h"
#include "generaloverview.h"
#include "cardoverview.h"
#if defined(Q_OS_WIN) || defined(Q_OS_ANDROID)
#include "ui_mainwindow.h"
#else
#include "ui_mainwindow_nonwin.h"
#endif
#include "rule-summary.h"
#include "pixmapanimation.h"
#include "record-analysis.h"
#include "aboutus.h"
#include "updatechecker.h"
#include "recorder.h"
#include "audio.h"
#include "stylehelper.h"
#include "uiutils.h"
#include "serverdialog.h"
#include "banipdialog.h"
#include "cardeditor.h"
#include "flatdialog.h"
#include "connectiondialog.h"
#include "configdialog.h"
#include "window.h"

#include <lua.hpp>
#include <QGraphicsView>
#include <QMessageBox>
#include <QProcess>
#include <QFileDialog>
#include <QDesktopServices>
#include <QSystemTrayIcon>
#include <QLabel>
#include <QBitmap>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#if !defined(QT_NO_OPENGL) && defined(USING_OPENGL)
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
#include <QtOpenGL/QOpenGLWidget>
#else
#include <QtOpenGL/QGLWidget>
#endif
#endif

class FitView : public QGraphicsView {
public:
    FitView(QGraphicsScene *scene) : QGraphicsView(scene) {
        setSceneRect(Config.Rect);
        setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

#if !defined(QT_NO_OPENGL) && defined(USING_OPENGL)
        if (QGLFormat::hasOpenGL()) {
            QGLWidget *widget = new QGLWidget(QGLFormat(QGL::SampleBuffers));
            widget->makeCurrent();
            setViewport(widget);
            setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
        }
#endif
    }

#ifdef Q_OS_WIN
    virtual void mousePressEvent(QMouseEvent *event) {
        MainWindow *parent = qobject_cast<MainWindow *>(parentWidget());
        if (parent)
            parent->mousePressEvent(event);
        QGraphicsView::mousePressEvent(event);
    }

    virtual void mouseMoveEvent(QMouseEvent *event) {
        MainWindow *parent = qobject_cast<MainWindow *>(parentWidget());
        if (parent)
            parent->mouseMoveEvent(event);
        QGraphicsView::mouseMoveEvent(event);
    }

    virtual void mouseReleaseEvent(QMouseEvent *event) {
        MainWindow *parent = qobject_cast<MainWindow *>(parentWidget());
        if (parent)
            parent->mouseReleaseEvent(event);
        QGraphicsView::mouseReleaseEvent(event);
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    virtual void mouseDoubleClickEvent(QMouseEvent *event) {
        MainWindow *parent = qobject_cast<MainWindow *>(parentWidget());
        if (parent)
            parent->mouseDoubleClickEvent(event);
        QGraphicsView::mouseDoubleClickEvent(event);
    }
#endif
#endif

    virtual void resizeEvent(QResizeEvent *event) {
        QGraphicsView::resizeEvent(event);
        QGraphicsScene *scene = this->scene();
        if (scene) {
            QRectF newSceneRect(0, 0, event->size().width(), event->size().height());
            scene->setSceneRect(newSceneRect);
            if (scene->sceneRect().size() != event->size()) {
                QSizeF from(scene->sceneRect().size());
                QSizeF to(event->size());
                QTransform transform;
                transform.scale(to.width() / from.width(), to.height() / from.height());
                setTransform(transform);
            } else {
                resetTransform();
            }
            setSceneRect(scene->sceneRect());
        }

        MainWindow *main_window = qobject_cast<MainWindow *>(parentWidget());
        if (main_window)
            main_window->fitBackgroundBrush();
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isLeftPressDown(false),
      scene(NULL), ui(new Ui::MainWindow), server(NULL), about_window(NULL),
      minButton(NULL), maxButton(NULL), normalButton(NULL), closeButton(NULL),
      versionInfomationReply(NULL), changeLogReply(NULL)
{
    ui->setupUi(this);
    setWindowTitle(tr("QSanguosha-Hegemony") + " " + Sanguosha->getVersion());
#ifdef Q_OS_WIN
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
#endif
    setAttribute(Qt::WA_TranslucentBackground);

    setMouseTracking(true);
    setMinimumWidth(800);
    setMinimumHeight(580);

    fetchUpdateInformation();

    connection_dialog = new ConnectionDialog(this);
    connect(ui->actionStart_Game, SIGNAL(triggered()), connection_dialog, SLOT(exec()));
    connect(connection_dialog, SIGNAL(accepted()), this, SLOT(startConnection()));

    config_dialog = new ConfigDialog(this);
    connect(ui->actionConfigure, SIGNAL(triggered()), config_dialog, SLOT(show()));
    connect(config_dialog, SIGNAL(bg_changed()), this, SLOT(changeBackground()));
    connect(config_dialog, SIGNAL(tableBg_changed()), this, SLOT(changeBackground()));

    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionAcknowledgement_2, SIGNAL(triggered()), this, SLOT(on_actionAcknowledgement_triggered()));

    StartScene *start_scene = new StartScene(this);

    QList<QAction *> actions;
    actions << ui->actionStart_Game
        << ui->actionStart_Server
        << ui->actionPC_Console_Start
        << ui->actionReplay
        << ui->actionConfigure
        << ui->actionGeneral_Overview
        << ui->actionCard_Overview
        << ui->actionAbout;

    foreach(QAction *action, actions)
        start_scene->addButton(action);

#if defined(Q_OS_WIN) || defined(Q_OS_ANDROID)
    ui->menuSumMenu->setAttribute(Qt::WA_TranslucentBackground);
    ui->menuGame->setAttribute(Qt::WA_TranslucentBackground);
    ui->menuView->setAttribute(Qt::WA_TranslucentBackground);
    ui->menuOptions->setAttribute(Qt::WA_TranslucentBackground);
    ui->menuDIY->setAttribute(Qt::WA_TranslucentBackground);
    ui->menuCheat->setAttribute(Qt::WA_TranslucentBackground);
    ui->menuHelp->setAttribute(Qt::WA_TranslucentBackground);
#endif

    view = new FitView(scene);

    setCentralWidget(view);
    restoreFromConfig();

    roundCorners();

    BackLoader::preload();
    gotoScene(start_scene);

    addAction(ui->actionFullscreen);

#if defined(Q_OS_WIN) || defined(Q_OS_ANDROID)
    menu = new QPushButton(this);
    menu->setMenu(ui->menuSumMenu);
    menu->setProperty("control", true);
    StyleHelper::getInstance()->setIcon(menu, QChar(0xf0c9), 15);
    menu->setToolTip(tr("<font color=%1>Config</font>").arg(Config.SkillDescriptionInToolTipColor.name()));
#endif

#if defined(Q_OS_WIN)
    minButton = new QPushButton(this);
    minButton->setProperty("control", true);

    maxButton = new QPushButton(this);
    maxButton->setProperty("bold", true);
    maxButton->setProperty("control", true);

    normalButton = new QPushButton(this);
    normalButton->setProperty("bold", true);
    normalButton->setProperty("control", true);

    closeButton= new QPushButton(this);
    closeButton->setObjectName("closeButton");
    closeButton->setProperty("control", true);

    StyleHelper::getInstance()->setIcon(minButton, QChar(0xf068), 15);
    StyleHelper::getInstance()->setIcon(maxButton, QChar(0xf106), 15);
    StyleHelper::getInstance()->setIcon(normalButton, QChar(0xf107), 15);
    StyleHelper::getInstance()->setIcon(closeButton, QChar(0xf00d), 15);

    minButton->setToolTip(tr("<font color=%1>Minimize</font>").arg(Config.SkillDescriptionInToolTipColor.name()));
    connect(minButton, SIGNAL(clicked()), this, SLOT(showMinimized()));
    maxButton->setToolTip(tr("<font color=%1>Maximize</font>").arg(Config.SkillDescriptionInToolTipColor.name()));
    connect(maxButton, SIGNAL(clicked()), this, SLOT(showMaximized()));
    normalButton->setToolTip(tr("<font color=%1>Restore downward</font>").arg(Config.SkillDescriptionInToolTipColor.name()));
    connect(normalButton, SIGNAL(clicked()), this, SLOT(showNormal()));
    closeButton->setToolTip(tr("<font color=%1>Close</font>").arg(Config.SkillDescriptionInToolTipColor.name()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    menuBar()->hide();
#elif defined(Q_OS_ANDROID)
    ui->menuSumMenu->removeAction(ui->menuView->menuAction());
#endif
    repaintButtons();

#ifndef Q_OS_ANDROID
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(1000);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->setEasingCurve(QEasingCurve::OutCurve);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
#endif

    start_scene->showOrganization();

    systray = NULL;
}

#ifdef Q_OS_WIN
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (windowState() & (Qt::WindowMaximized | Qt::WindowFullScreen))
        return;
    if (event->button() == Qt::LeftButton) {
        if (isZoomReady) {
            isLeftPressDown = true;
            if (direction != None) {
                releaseMouse();
                setCursor(QCursor(Qt::ArrowCursor));
            }
        } else {
            bool can_move = true;
            if (view && view->scene()) {
                QPointF pos = view->mapToScene(event->pos());
                if (scene->itemAt(pos, QTransform()))
                    can_move = false;
            }
            if (can_move) {
                isLeftPressDown = true;
                movePosition = event->globalPos() - frameGeometry().topLeft();
                event->accept();
            }
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (windowState() & (Qt::WindowMaximized | Qt::WindowFullScreen))
        return;
    QPoint globalPoint = event->globalPos();
    if (isZoomReady && isLeftPressDown) {
        QRect rect = this->rect();
        QPoint topLeft = mapToGlobal(rect.topLeft());
        QPoint bottomRight = mapToGlobal(rect.bottomRight());

        QRect rMove(topLeft, bottomRight);

        switch (direction) {
        case Left:
            if (bottomRight.x() - globalPoint.x() <= minimumWidth())
                rMove.setX(topLeft.x());
            else
                rMove.setX(globalPoint.x());
            break;
        case Right:
            rMove.setWidth(globalPoint.x() - topLeft.x());
            break;
        case Up:
            if (bottomRight.y() - globalPoint.y() <= minimumHeight())
                rMove.setY(topLeft.y());
            else
                rMove.setY(globalPoint.y());
            break;
        case Down:
            rMove.setHeight(globalPoint.y() - topLeft.y());
            break;
        case LeftTop:
            if (bottomRight.x() - globalPoint.x() <= minimumWidth())
                rMove.setX(topLeft.x());
            else
                rMove.setX(globalPoint.x());
            if (bottomRight.y() - globalPoint.y() <= minimumHeight())
                rMove.setY(topLeft.y());
            else
                rMove.setY(globalPoint.y());
            break;
        case RightTop:
            rMove.setWidth(globalPoint.x() - topLeft.x());
            if (bottomRight.y() - globalPoint.y() <= minimumHeight())
                rMove.setY(topLeft.y());
            else
                rMove.setY(globalPoint.y());
            break;
        case LeftBottom:
            if (bottomRight.x() - globalPoint.x() <= minimumWidth())
                rMove.setX(topLeft.x());
            else
                rMove.setX(globalPoint.x());
            rMove.setHeight(globalPoint.y() - topLeft.y());
            break;
        case RightBottom:
            rMove.setWidth(globalPoint.x() - topLeft.x());
            rMove.setHeight(globalPoint.y() - topLeft.y());
            break;
        default:
            break;
        }
        setGeometry(rMove);
    } else if (isLeftPressDown && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPos() - movePosition);
        event->accept();
    } else if (!isLeftPressDown) {
        region(globalPoint);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *)
{
    isLeftPressDown = false;
    if (direction != None) {
        releaseMouse();
        setCursor(QCursor(Qt::ArrowCursor));
    }
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    bool can_change = true;
    if (view && view->scene()) {
        QPointF pos = view->mapToScene(event->pos());
        if (scene->itemAt(pos, QTransform()))
            can_change = false;
    }
    if (can_change) {
        if (windowState() & Qt::WindowMaximized)
            showNormal();
        else
            showMaximized();
    }
}
#endif

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        repaintButtons();
        roundCorners();
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        if (view && view->viewport())
            view->viewport()->repaint();
#endif
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    repaintButtons();
    roundCorners();
    QMainWindow::resizeEvent(event);
}

void MainWindow::restoreFromConfig() {
    resize(Config.value("WindowSize", QSize(1366, 706)).toSize());
    move(Config.value("WindowPosition", QPoint(-8, -8)).toPoint());
    setWindowState((Qt::WindowStates) Config.value("WindowState", 0).toInt());

    QFont font;
    if (Config.UIFont != font)
        QApplication::setFont(Config.UIFont, "QTextEdit");

    ui->actionEnable_Hotkey->setChecked(Config.EnableHotKey);
    ui->actionNever_nullify_my_trick->setChecked(Config.NeverNullifyMyTrick);
    ui->actionNever_nullify_my_trick->setEnabled(false);
}

void MainWindow::region(const QPoint &cursorGlobalPoint)
{
    QRect rect = this->rect();
    QPoint topLeft = mapToGlobal(rect.topLeft());
    QPoint bottomRight = mapToGlobal(rect.bottomRight());

    int x = cursorGlobalPoint.x();
    int y = cursorGlobalPoint.y();

    if (topLeft.x() + S_PADDING >= x && topLeft.x() <= x && topLeft.y() + S_PADDING >= y && topLeft.y() <= y) {
        direction = LeftTop;
        setCursor(QCursor(Qt::SizeFDiagCursor));
    } else if (x >= bottomRight.x() - S_PADDING && x <= bottomRight.x() && y >= bottomRight.y() - S_PADDING && y <= bottomRight.y()) {
        direction = RightBottom;
        setCursor(QCursor(Qt::SizeFDiagCursor));
    } else if (x <= topLeft.x() + S_PADDING && x >= topLeft.x() && y >= bottomRight.y() - S_PADDING && y <= bottomRight.y()) {
        direction = LeftBottom;
        setCursor(QCursor(Qt::SizeBDiagCursor));
    } else if (x <= bottomRight.x() && x >= bottomRight.x() - S_PADDING && y >= topLeft.y() && y <= topLeft.y() + S_PADDING) {
        direction = RightTop;
        setCursor(QCursor(Qt::SizeBDiagCursor));
    } else if (x <= topLeft.x() + S_PADDING && x >= topLeft.x()) {
        direction = Left;
        setCursor(QCursor(Qt::SizeHorCursor));
    } else if (x <= bottomRight.x() && x >= bottomRight.x() - S_PADDING) {
        direction = Right;
        setCursor(QCursor(Qt::SizeHorCursor));
    } else if (y >= topLeft.y() && y <= topLeft.y() + S_PADDING) {
        direction = Up;
        setCursor(QCursor(Qt::SizeVerCursor));
    } else if (y <= bottomRight.y() && y >= bottomRight.y() - S_PADDING) {
        direction = Down;
        setCursor(QCursor(Qt::SizeVerCursor));
    } else {
        direction = None;
        setCursor(QCursor(Qt::ArrowCursor));
    }
    if (direction != None)
        isZoomReady = true;
    else
        isZoomReady = false;
}

void MainWindow::fetchUpdateInformation()
{
    static QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
#ifdef QT_DEBUG
    QString URL1 = "http://ver.qsanguosha.org/test/UpdateInfo";
    QString URL2 = "http://ver.qsanguosha.org/test/whatsnew.html";
#else
    QString URL1 = "http://ver.qsanguosha.org/UpdateInfo";
    QString URL2 = "http://ver.qsanguosha.org/whatsnew.html";
#endif

    versionInfomationReply = mgr->get(QNetworkRequest(QUrl(URL1)));
    changeLogReply = mgr->get(QNetworkRequest(QUrl(URL2)));

    connect(versionInfomationReply, SIGNAL(finished()), SLOT(onVersionInfomationGotten()));
    connect(changeLogReply, SIGNAL(finished()), SLOT(onChangeLogGotten()));
}

void MainWindow::roundCorners()
{
#ifndef Q_OS_ANDROID
    QBitmap mask(size());
    if (windowState() & (Qt::WindowMaximized | Qt::WindowFullScreen)) {
        mask.fill(Qt::black);
    } else {
        mask.fill();
        QPainter painter(&mask);
        QPainterPath path;
        QRect windowRect = mask.rect();
        QRect maskRect(windowRect.x(), windowRect.y(), windowRect.width(), windowRect.height());
        path.addRoundedRect(maskRect, S_CORNER_SIZE, S_CORNER_SIZE);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.fillPath(path, Qt::black);
    }
    setMask(mask);
#endif
}

void MainWindow::repaintButtons()
{
#if defined(Q_OS_WIN)
    if (!minButton || !maxButton || !normalButton || !closeButton || !menu)
        return;
    int width = this->width();
    minButton->setGeometry(width - 130, 0, 40, 33);
    maxButton->setGeometry(width - 90, 0, 40, 33);
    normalButton->setGeometry(width - 90, 0, 40, 33);
    closeButton->setGeometry(width - 50, 0, 40, 33);

    Qt::WindowStates state = windowState();
    if (state & Qt::WindowMaximized) {
        maxButton->setVisible(false);
        normalButton->setVisible(true);
        minButton->setVisible(true);
        menu->setGeometry(width - 170, 0, 40, 33);
    } else if (state & Qt::WindowFullScreen) {
        maxButton->setVisible(false);
        normalButton->setVisible(false);
        minButton->setVisible(false);
        menu->setGeometry(width - 90, 0, 40, 33);
    } else {
        maxButton->setVisible(true);
        normalButton->setVisible(false);
        minButton->setVisible(true);
        menu->setGeometry(width - 170, 0, 40, 33);
    }
#elif defined(Q_OS_ANDROID)
    if (menu)
        menu->setGeometry(width() - 50, 0, 40, 33);
#endif
}

void MainWindow::closeEvent(QCloseEvent *) {
    if (!isFullScreen() && !isMaximized()) {
        Config.setValue("WindowSize", size());
        Config.setValue("WindowPosition", pos());
    }
    Config.setValue("WindowState", (int)windowState());
}

MainWindow::~MainWindow() {
    delete ui;
    view->deleteLater();
    QSanSkinFactory::destroyInstance();
    QSanUiUtils::QSanFreeTypeFont::quit();
}

void MainWindow::gotoScene(QGraphicsScene *scene) {
    if (this->scene) {
        this->scene->deleteLater();
        if (about_window) {
            about_window->deleteLater();
            about_window = NULL;
        }
    }

    this->scene = scene;
    view->setScene(scene);
    QResizeEvent e(QSize(view->size().width(), view->size().height()), view->size());
    view->resizeEvent(&e);
}

void MainWindow::on_actionExit_triggered() {
    QMessageBox::StandardButton result;
    result = QMessageBox::question(this,
        tr("Sanguosha"),
        tr("Are you sure to exit?"),
        QMessageBox::Ok | QMessageBox::Cancel);
    if (result == QMessageBox::Ok) {
        delete systray;
        systray = NULL;
        close();
    }
}

void MainWindow::on_actionStart_Server_triggered() {
    ServerDialog *dialog = new ServerDialog(this);
    if (!dialog->config())
        return;

    server = new Server(this);
    if (!server->listen()) {
        QMessageBox::warning(this, tr("Warning"), tr("Can not start server!"));
        return;
    }

    server->daemonize();

    ui->actionStart_Game->disconnect();
#ifdef QT_NO_PROCESS
    ui->actionStart_Game->setEnabled(false);
#else
    connect(ui->actionStart_Game, SIGNAL(triggered()), this, SLOT(startGameInAnotherInstance()));
#endif
    StartScene *start_scene = qobject_cast<StartScene *>(scene);
    if (start_scene) {
        start_scene->switchToServer(server);
        if (Config.value("EnableMinimizeDialog", false).toBool())
            this->on_actionMinimize_to_system_tray_triggered();
    }
}

void MainWindow::checkVersion(const QString &server_version_str, const QString &server_mod) {
    QString client_mod = Sanguosha->getMODName();
    if (client_mod != server_mod) {
        QMessageBox::warning(this, tr("Warning"), tr("Client MOD name is not same as the server!"));
        return;
    }

    Client *client = qobject_cast<Client *>(sender());
    const QSanVersionNumber &client_version = Sanguosha->getVersionNumber();
    QSanVersionNumber server_version(server_version_str);

    if (server_version == client_version) {
        client->signup();
        connect(client, SIGNAL(server_connected()), SLOT(enterRoom()));
        return;
    }

    client->disconnectFromHost();

    static QString link = "http://pan.baidu.com/share/home?uk=3173324412";
    QString text = tr("Server version is %1, client version is %2 <br/>").arg(server_version).arg(client_version);
    if (server_version > client_version)
        text.append(tr("Your client version is older than the server's, please update it <br/>"));
    else
        text.append(tr("The server version is older than your client version, please ask the server to update<br/>"));

    text.append(tr("Download link : <a href='%1'>%1</a> <br/>").arg(link));
    QMessageBox::warning(this, tr("Warning"), text);
}

void MainWindow::startConnection() {
    Client *client = new Client(this);

    connect(client, SIGNAL(version_checked(QString, QString)), SLOT(checkVersion(QString, QString)));
    connect(client, SIGNAL(error_message(QString)), SLOT(networkError(QString)));
}

void MainWindow::on_actionReplay_triggered() {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QString location = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#else
    QString location = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#endif
    QString last_dir = Config.value("LastReplayDir").toString();
    if (!last_dir.isEmpty())
        location = last_dir;

    QString filename = QFileDialog::getOpenFileName(this,
        tr("Select a reply file"),
        location,
        tr("QSanguosha Replay File(*.qsgs);; Image replay file (*.png)"));

    if (filename.isEmpty())
        return;

    QFileInfo file_info(filename);
    last_dir = file_info.absoluteDir().path();
    Config.setValue("LastReplayDir", last_dir);

    Client *client = new Client(this, filename);
    connect(client, SIGNAL(server_connected()), SLOT(enterRoom()));
    client->signup();
}

void MainWindow::networkError(const QString &error_msg) {
    if (isVisible())
        QMessageBox::warning(this, tr("Network error"), error_msg);

    if (NULL != RoomSceneInstance) {
        RoomSceneInstance->stopHeroSkinChangingAnimations();
    }
}

void BackLoader::preload() {
    QStringList emotions = G_ROOM_SKIN.getAnimationFileNames();

    foreach(QString emotion, emotions) {
        int n = PixmapAnimation::GetFrameCount(emotion);
        for (int i = 0; i < n; i++) {
            QString filename = QString("image/system/emotion/%1/%2.png").arg(emotion).arg(QString::number(i));
            G_ROOM_SKIN.getPixmapFromFileName(filename);
        }
    }
}

void MainWindow::enterRoom() {
    // add current ip to history
    if (!Config.HistoryIPs.contains(Config.HostAddress)) {
        Config.HistoryIPs << Config.HostAddress;
        Config.HistoryIPs.sort();
        Config.setValue("HistoryIPs", Config.HistoryIPs);
    }

    ui->actionStart_Game->setEnabled(false);
    ui->actionStart_Server->setEnabled(false);

    RoomScene *room_scene = new RoomScene(this);
    ui->actionView_Discarded->setEnabled(true);
    ui->actionView_distance->setEnabled(true);
    ui->actionServerInformation->setEnabled(true);
    ui->actionSurrender->setEnabled(true);
    ui->actionNever_nullify_my_trick->setEnabled(true);
    ui->actionSaveRecord->setEnabled(true);

    connect(ClientInstance, SIGNAL(surrender_enabled(bool)), ui->actionSurrender, SLOT(setEnabled(bool)));

    connect(ui->actionView_Discarded, SIGNAL(triggered()), room_scene, SLOT(toggleDiscards()));
    connect(ui->actionView_distance, SIGNAL(triggered()), room_scene, SLOT(viewDistance()));
    connect(ui->actionServerInformation, SIGNAL(triggered()), room_scene, SLOT(showServerInformation()));
    connect(ui->actionSurrender, SIGNAL(triggered()), room_scene, SLOT(surrender()));
    connect(ui->actionSaveRecord, SIGNAL(triggered()), room_scene, SLOT(saveReplayRecord()));

    if (ServerInfo.EnableCheat) {
        ui->menuCheat->setEnabled(true);

        connect(ui->actionDeath_note, SIGNAL(triggered()), room_scene, SLOT(makeKilling()));
        connect(ui->actionDamage_maker, SIGNAL(triggered()), room_scene, SLOT(makeDamage()));
        connect(ui->actionRevive_wand, SIGNAL(triggered()), room_scene, SLOT(makeReviving()));
        connect(ui->actionExecute_script_at_server_side, SIGNAL(triggered()), room_scene, SLOT(doScript()));
    }
    else {
        ui->menuCheat->setEnabled(false);
        ui->actionDeath_note->disconnect();
        ui->actionDamage_maker->disconnect();
        ui->actionRevive_wand->disconnect();
        ui->actionSend_lowlevel_command->disconnect();
        ui->actionExecute_script_at_server_side->disconnect();
    }

    connect(room_scene, SIGNAL(restart()), this, SLOT(startConnection()));
    connect(room_scene, SIGNAL(return_to_start()), this, SLOT(gotoStartScene()));

    gotoScene(room_scene);
}

void MainWindow::gotoStartScene() {
    if (server != NULL){
        server->deleteLater();
        server = NULL;
    }

    if (Self) {
        delete Self;
        Self = NULL;
    }

    StartScene *start_scene = new StartScene(this);

    QList<QAction *> actions;
    actions << ui->actionStart_Game
        << ui->actionStart_Server
        << ui->actionPC_Console_Start
        << ui->actionReplay
        << ui->actionConfigure
        << ui->actionGeneral_Overview
        << ui->actionCard_Overview
        << ui->actionAbout;

    foreach(QAction *action, actions)
        start_scene->addButton(action);

    setCentralWidget(view);

    ui->menuCheat->setEnabled(false);
    ui->actionDeath_note->disconnect();
    ui->actionDamage_maker->disconnect();
    ui->actionRevive_wand->disconnect();
    ui->actionSend_lowlevel_command->disconnect();
    ui->actionExecute_script_at_server_side->disconnect();
    gotoScene(start_scene);

    addAction(ui->actionShow_Hide_Menu);
    addAction(ui->actionFullscreen);

    delete systray;
    systray = NULL;
    if (ClientInstance)
        delete ClientInstance;
}

void MainWindow::startGameInAnotherInstance() {
#ifndef QT_NO_PROCESS
    QProcess::startDetached(QApplication::applicationFilePath(), QStringList());
#endif
}

void MainWindow::on_actionGeneral_Overview_triggered() {
    GeneralOverview *overview = GeneralOverview::getInstance(this);
    overview->fillGenerals(Sanguosha->getGeneralList());
    overview->show();
}

void MainWindow::on_actionCard_Overview_triggered() {
    CardOverview *overview = CardOverview::getInstance(this);
    overview->loadFromAll();
    overview->show();
}

void MainWindow::on_actionEnable_Hotkey_toggled(bool checked) {
    if (Config.EnableHotKey != checked) {
        Config.EnableHotKey = checked;
        Config.setValue("EnableHotKey", checked);
    }
}

void MainWindow::on_actionNever_nullify_my_trick_toggled(bool checked) {
    if (Config.NeverNullifyMyTrick != checked) {
        Config.NeverNullifyMyTrick = checked;
        Config.setValue("NeverNullifyMyTrick", checked);
    }
}

void MainWindow::on_actionAbout_triggered() {
    if (scene == NULL)
        return;

    if (about_window == NULL) {
        // Cao Cao's pixmap
        QString content = "<center><img src='image/system/shencc.png'> <br /> </center>";

        // Cao Cao' poem
        QString poem = tr("Disciples dressed in blue, my heart worries for you. You are the cause, of this song without pause");
        content.append(QString("<p align='right'><i>%1</i></p>").arg(poem));

        // Cao Cao's signature
        QString signature = tr("\"A Short Song\" by Cao Cao");
        content.append(QString("<p align='right'><i>%1</i></p>").arg(signature));

        QString email = "moligaloo@gmail.com";
        content.append(tr("This is the open source clone of the popular <b>Sanguosha</b> game,"
                          "totally written in C++ Qt GUI framework <br />"
                          "My Email: <a href='mailto:%1' style = \"color:#0072c1; \">%1</a> <br/>"
                          "My QQ: 365840793 <br/>"
                          "My Weibo: http://weibo.com/moligaloo <br/>").arg(email));

        QString config;

#ifdef QT_NO_DEBUG
        config = "release";
#else
        config = "debug";
#endif

        content.append(tr("Current version: %1 %2 (%3)<br/>")
                       .arg(Sanguosha->getVersion())
                       .arg(config)
                       .arg(Sanguosha->getVersionName()));

        const char *date = __DATE__;
        const char *time = __TIME__;
        content.append(tr("Compilation time: %1 %2 <br/>").arg(date).arg(time));

        QString project_url = "https://github.com/QSanguosha-Rara/QSanguosha-For-Hegemony";
        content.append(tr("Source code: <a href='%1' style = \"color:#0072c1; \">%1</a> <br/>").arg(project_url));

        QString forum_url = "http://qsanguosha.org";
        content.append(tr("Forum: <a href='%1' style = \"color:#0072c1; \">%1</a> <br/>").arg(forum_url));

        about_window = new Window(tr("About QSanguosha"), QSize(420, 465));
        scene->addItem(about_window);
        about_window->setZValue(32766);

        about_window->addContent(content);
        about_window->addCloseButton(tr("OK"));
        about_window->keepWhenDisappear();
    }

    about_window->shift(scene->sceneRect().center());
    about_window->appear();
}

void MainWindow::on_actionAbout_Us_triggered() {
    AboutUsDialog *dialog = new AboutUsDialog(this);
    dialog->show();
}

void MainWindow::fitBackgroundBrush() {
    if (scene) {
        QBrush brush(scene->backgroundBrush());
        QPixmap pixmap(brush.texture());

        QRectF rect(scene->sceneRect());
        QTransform transform;
        transform.translate(-rect.left(), -rect.top());
        transform.scale(rect.width() / pixmap.width(), rect.height() / pixmap.height());
        brush.setTransform(transform);
        scene->setBackgroundBrush(brush);
    }
}

void MainWindow::changeBackground() {
    scene->setBackgroundBrush(QBrush(QPixmap(scene->inherits("RoomScene") ? Config.TableBgImage : Config.BackgroundImage)));
    fitBackgroundBrush();

    if (scene->inherits("StartScene")) {
        StartScene *start_scene = qobject_cast<StartScene *>(scene);
        start_scene->setServerLogBackground();
    }
}

void MainWindow::on_actionFullscreen_triggered()
{
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

void MainWindow::on_actionMinimize_to_system_tray_triggered()
{
    if (systray == NULL) {
        QIcon icon("image/system/magatamas/3.png");
        systray = new QSystemTrayIcon(icon, this);

        QAction *appear = new QAction(tr("Show main window"), this);
        connect(appear, SIGNAL(triggered()), this, SLOT(show()));

        QMenu *menu = new QMenu;
        menu->addAction(appear);
        menu->addMenu(ui->menuGame);

        menu->addMenu(ui->menuView);
        menu->addMenu(ui->menuOptions);
        menu->addMenu(ui->menuHelp);

        systray->setContextMenu(menu);

        systray->show();
        systray->showMessage(windowTitle(), tr("Game is minimized"));

        hide();
    }
}

void MainWindow::on_actionRule_Summary_triggered()
{
    RuleSummary *dialog = new RuleSummary(this);
    dialog->show();
}

BroadcastBox::BroadcastBox(Server *server, QWidget *parent)
    : QDialog(parent), server(server)
{
    setWindowTitle(tr("Broadcast"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel(tr("Please input the message to broadcast")));

    text_edit = new QTextEdit;
    layout->addWidget(text_edit);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    QPushButton *ok_button = new QPushButton(tr("OK"));
    hlayout->addWidget(ok_button);

    layout->addLayout(hlayout);

    setLayout(layout);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));
}

void BroadcastBox::accept() {
    QDialog::accept();
    server->broadcastSystemMessage(text_edit->toPlainText());
}

void MainWindow::on_actionBroadcast_triggered() {
    Server *server = findChild<Server *>();
    if (server == NULL) {
        QMessageBox::warning(this, tr("Warning"), tr("Server is not started yet!"));
        return;
    }

    BroadcastBox *dialog = new BroadcastBox(server, this);
    dialog->exec();
}

void MainWindow::on_actionAcknowledgement_triggered() {
    if (scene == NULL)
        return;

    Window *window = new Window(QString(), QSize(1000, 677), "image/system/acknowledgement.png");
    scene->addItem(window);

    Button *button = window->addCloseButton(tr("OK"));
    button->moveBy(-85, -35);
    window->setZValue(32766);
    window->shift(scene->sceneRect().center());

    window->appear();
}

void MainWindow::on_actionPC_Console_Start_triggered() {
    ServerDialog *dialog = new ServerDialog(this);
    if (!dialog->config())
        return;

    server = new Server(this);
    if (!server->listen()) {
        QMessageBox::warning(this, tr("Warning"), tr("Can not start server!"));
        return;
    }

    server->daemonize();
    server->createNewRoom();

    Config.HostAddress = "127.0.0.1";
    startConnection();
}

void MainWindow::on_actionReplay_file_convert_triggered() {
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Please select a replay file"),
        Config.value("LastReplayDir").toString(),
        tr("QSanguosha Replay File(*.qsgs);; Image replay file (*.png)"));

    if (filename.isEmpty())
        return;

    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        QFileInfo info(filename);
        QString tosave = info.absoluteDir().absoluteFilePath(info.baseName());

        if (filename.endsWith(".qsgs")) {
            tosave.append(".png");

            // txt to png
            Recorder::TXT2PNG(file.readAll()).save(tosave);

        }
        else if (filename.endsWith(".png")) {
            tosave.append(".qsgs");

            // png to txt
            QByteArray data = Replayer::PNG2TXT(filename);

            QFile tosave_file(tosave);
            if (tosave_file.open(QIODevice::WriteOnly))
                tosave_file.write(data);
        }
    }
}

void MainWindow::on_actionRecord_analysis_triggered() {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QString location = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#else
    QString location = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#endif
    QString last_dir = Config.value("LastReplayDir").toString();
    if (!last_dir.isEmpty())
        location = last_dir;

    QString filename = QFileDialog::getOpenFileName(this,
        tr("Load replay record"),
        location,
        tr("QSanguosha Replay File(*.qsgs);; Image replay file (*.png)"));

    if (filename.isEmpty()) return;

    QDialog *rec_dialog = new QDialog(this);
    rec_dialog->setWindowTitle(tr("Record Analysis"));
    rec_dialog->resize(800, 500);
    QTableWidget *table = new QTableWidget;

    RecAnalysis record(filename);
    QMap<QString, PlayerRecordStruct *> record_map = record.getRecordMap();
    table->setColumnCount(11);
    table->setRowCount(record_map.keys().length());
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    static QStringList labels;
    if (labels.isEmpty()) {
        labels << tr("ScreenName") << tr("General") << tr("Role") << tr("Living") << tr("WinOrLose") << tr("TurnCount")
            << tr("Recover") << tr("Damage") << tr("Damaged") << tr("Kill") << tr("Designation");
    }
    table->setHorizontalHeaderLabels(labels);
    table->setSelectionBehavior(QTableWidget::SelectRows);

    int i = 0;
    foreach (PlayerRecordStruct *rec, record_map) {
        QTableWidgetItem *item = new QTableWidgetItem;
        QString screen_name = Sanguosha->translate(rec->m_screenName);
        if (rec->m_statue == "robot")
            screen_name += "(" + Sanguosha->translate("robot") + ")";

        item->setText(screen_name);
        table->setItem(i, 0, item);

        item = new QTableWidgetItem;
        QString generals = Sanguosha->translate(rec->m_generalName);
        if (!rec->m_general2Name.isEmpty())
            generals += "/" + Sanguosha->translate(rec->m_general2Name);
        item->setText(generals);
        table->setItem(i, 1, item);

        item = new QTableWidgetItem;
        item->setText(Sanguosha->translate(rec->m_role));
        table->setItem(i, 2, item);

        item = new QTableWidgetItem;
        item->setText(rec->m_isAlive ? tr("Alive") : tr("Dead"));
        table->setItem(i, 3, item);

        item = new QTableWidgetItem;
        bool is_win = record.getRecordWinners().contains(rec->m_role)
            || record.getRecordWinners().contains(record_map.key(rec));
        item->setText(is_win ? tr("Win") : tr("Lose"));
        table->setItem(i, 4, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(rec->m_turnCount));
        table->setItem(i, 5, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(rec->m_recover));
        table->setItem(i, 6, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(rec->m_damage));
        table->setItem(i, 7, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(rec->m_damaged));
        table->setItem(i, 8, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(rec->m_kill));
        table->setItem(i, 9, item);

        item = new QTableWidgetItem;
        item->setText(rec->m_designation.join(", "));
        table->setItem(i, 10, item);
        i++;
    }

    table->resizeColumnsToContents();

    QLabel *label = new QLabel;
    label->setText(tr("Packages:") + record.getRecordPackages().join(","));

    QLabel *label_game_mode = new QLabel;
    label_game_mode->setText(tr("GameMode:") + Sanguosha->getModeName(record.getRecordGameMode()));

    QLabel *label_options = new QLabel;
    label_options->setText(tr("ServerOptions:") + record.getRecordServerOptions().join(","));

    QTextEdit *chat_info = new QTextEdit;
    chat_info->setReadOnly(chat_info);
    chat_info->setText(record.getRecordChat());

    QLabel *table_chat_title = new QLabel;
    table_chat_title->setText(tr("Chat Information:"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(label_game_mode);
    layout->addWidget(label_options);
    layout->addWidget(table);
    layout->addSpacing(15);
    layout->addWidget(table_chat_title);
    layout->addWidget(chat_info);
    rec_dialog->setLayout(layout);

    rec_dialog->exec();
}

void MainWindow::on_actionAbout_fmod_triggered() {
    if (scene == NULL)
        return;

    QString content = tr("FMOD is a proprietary audio library made by Firelight Technologies");
    content.append("<p align='center'> <img src='image/logo/fmod.png' /> </p> <br/>");

    QString address = "http://www.fmod.org";
    content.append(tr("Official site: <a href='%1' style = \"color:#0072c1; \">%1</a> <br/>").arg(address));

#ifdef AUDIO_SUPPORT
    content.append(tr("Current versionn %1 <br/>").arg(Audio::getVersion()));
#endif

    Window *window = new Window(tr("About fmod"), QSize(500, 260));
    scene->addItem(window);

    window->addContent(content);
    window->addCloseButton(tr("OK"));
    window->setZValue(32766);
    window->shift(scene->sceneRect().center());

    window->appear();
}

void MainWindow::on_actionAbout_Lua_triggered() {
    if (scene == NULL)
        return;

    QString content = tr("Lua is a powerful, fast, lightweight, embeddable scripting language.");
    content.append("<p align='center'> <img src='image/logo/lua.png' /> </p> <br/>");

    QString address = "http://www.lua.org";
    content.append(tr("Official site: <a href='%1' style = \"color:#0072c1; \">%1</a> <br/>").arg(address));

    content.append(tr("Current versionn %1 <br/>").arg(LUA_RELEASE));
    content.append(LUA_COPYRIGHT);

    Window *window = new Window(tr("About Lua"), QSize(500, 585));
    scene->addItem(window);

    window->addContent(content);
    window->addCloseButton(tr("OK"));
    window->setZValue(32766);
    window->shift(scene->sceneRect().center());

    window->appear();
}

void MainWindow::on_actionAbout_GPLv3_triggered() {
    if (scene == NULL)
        return;

    QString content = tr("The GNU General Public License is the most widely used free software license, which guarantees end users the freedoms to use, study, share, and modify the software.");
    content.append("<p align='center'> <img src='image/logo/gplv3.png' /> </p> <br/>");

    QString address = "http://gplv3.fsf.org";
    content.append(tr("Official site: <a href='%1' style = \"color:#0072c1; \">%1</a> <br/>").arg(address));

    Window *window = new Window(tr("About GPLv3"), QSize(500, 225));
    scene->addItem(window);

    window->addContent(content);
    window->addCloseButton(tr("OK"));
    window->setZValue(32766);
    window->shift(scene->sceneRect().center());

    window->appear();
}

void MainWindow::on_actionManage_Ban_IP_triggered(){
    BanIpDialog *dlg = new BanIpDialog(this, server);
    if (server) {
        connect(server, SIGNAL(newPlayer(ServerPlayer*)), dlg, SLOT(addPlayer(ServerPlayer*)));
    }

    dlg->show();
}

void MainWindow::onVersionInfomationGotten()
{
    while (!versionInfomationReply->atEnd()) {
        QString line = versionInfomationReply->readLine();
        line.remove('\n');

        QStringList texts = line.split('|', QString::SkipEmptyParts);

        if(texts.size() != 2)
            return;

        QString key = texts.at(0);
        QString value = texts.at(1);
        if ("VersionNumber" == key) {
            QString v = value;
            if (value.contains("Patch")) {
                updateInfomation.is_patch = true;
                v.chop(6);
            } else {
                updateInfomation.is_patch = false;
            }

            QSanVersionNumber latest_version = Sanguosha->getVersionNumber();
            if (!v.isNull() && latest_version.tryParse(v))
                v = latest_version;

            updateInfomation.version_number = v;
            if (Sanguosha->getVersionNumber() < latest_version)
                setWindowTitle(tr("New Version Available") + "  " + windowTitle());
        } else if ("Address" == key) {
            updateInfomation.address = value;
        } else if ("StrategicAdvantageKey" == key) {
            Config.setValue(key, value);
        }
        if (!updateInfomation.address.isNull()
                && !updateInfomation.version_number.isNull())
            ui->actionCheckUpdate->setEnabled(true);
    }
    versionInfomationReply->deleteLater();
}

void MainWindow::onChangeLogGotten()
{
    QString fileName = "info.html";
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        qDebug() << "Cannot open the file: " << fileName;
        return;
    }
    QByteArray codeContent = changeLogReply->readAll();
    file.write(codeContent);
    file.close();
    changeLogReply->deleteLater();
}

void MainWindow::on_actionCheckUpdate_triggered()
{
    FlatDialog *dialog = new FlatDialog(this);
    dialog->setWindowTitle(tr("Check Update"));

    UpdateChecker *widget = new UpdateChecker;
    widget->fill(updateInfomation);
    dialog->mainLayout()->addWidget(widget);

    dialog->addCloseButton();

    dialog->show();
}


void MainWindow::on_actionCard_editor_triggered()
{
    static CardEditor *editor;
    if (editor == NULL)
        editor = new CardEditor(this);

    editor->show();
}
