#include "packagingeditor.h"
#include "mainwindow.h"

#include <QCommandLinkButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QTabWidget>
#include <QProcess>
#include <QMessageBox>

PackagingEditor::PackagingEditor(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("DIY package manager"));

    QString url = "http://www.7-zip.org";
    QLabel *label = new QLabel(tr("Package format is 7z, see its offcial site :<a href='%1'>%1</a>").arg(url));

    QTabWidget *tab_widget = new QTabWidget;
    tab_widget->addTab(createManagerTab(), tr("Package management"));
    tab_widget->addTab(createPackagingTab(), tr("Make package"));

    QVBoxLayout *layout = new QVBoxLayout;

    layout->addWidget(label);
    layout->addWidget(tab_widget);

    setLayout(layout);
}

void PackagingEditor::loadPackageList(){
    QDir dir("extensions");
    QIcon icon("image/system/ark.png");
    foreach(QFileInfo info, dir.entryInfoList(QStringList() << "*.spec")){
        new QListWidgetItem(icon, info.baseName(), package_list);
    }
}

QWidget *PackagingEditor::createManagerTab(){
    QWidget *widget = new QWidget;

    package_list = new QListWidget;
    package_list->setViewMode(QListView::IconMode);
    package_list->setIconSize(QSize(64, 64));
    package_list->setMovement(QListView::Static);
    package_list->setWordWrap(true);

    loadPackageList();

    QVBoxLayout *vlayout = new QVBoxLayout;

    QCommandLinkButton *install_button = new QCommandLinkButton(tr("Install"));
    install_button->setDescription(tr("Install a DIY package"));

    QCommandLinkButton *uninstall_button = new QCommandLinkButton(tr("Uninstall"));
    uninstall_button->setDescription(tr("Uninstall a DIY package"));

    QCommandLinkButton *rescan_button = new QCommandLinkButton(tr("Rescan"));
    rescan_button->setDescription(tr("Rescan existing packages"));

    vlayout->addWidget(install_button);
    vlayout->addWidget(uninstall_button);
    vlayout->addWidget(rescan_button);
    vlayout->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addLayout(vlayout);
    layout->addWidget(package_list);

    widget->setLayout(layout);

    connect(install_button, SIGNAL(clicked()), this, SLOT(installPackage()));
    connect(uninstall_button, SIGNAL(clicked()), this, SLOT(uninstallPackage()));
    connect(rescan_button, SIGNAL(clicked()), this, SLOT(rescanPackage()));

    return widget;
}

void PackagingEditor::rescanPackage(){
    package_list->clear();

    loadPackageList();
}

QWidget *PackagingEditor::createPackagingTab(){
    QWidget *widget = new QWidget;

    file_list = new QListWidget;

    QVBoxLayout *vlayout = new QVBoxLayout;

    QCommandLinkButton *browse_button = new QCommandLinkButton(tr("Browse files"));
    browse_button->setDescription(tr("Select files to package"));

    QCommandLinkButton *package_button = new QCommandLinkButton(tr("Make package"));
    package_button->setDescription(tr("Export files to a single package"));

    vlayout->addWidget(browse_button);
    vlayout->addWidget(package_button);
    vlayout->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addLayout(vlayout);
    layout->addWidget(file_list);

    widget->setLayout(layout);

    connect(browse_button, SIGNAL(clicked()), this, SLOT(browseFiles()));
    connect(package_button, SIGNAL(clicked()), this, SLOT(makePackage()));

    return widget;
}

void PackagingEditor::installPackage(){
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Select a package to install"),
                                                    QString(),
                                                    tr("7z format (*.7z)")
                                                    );

    if(!filename.isEmpty()){
        QProcess *process = new QProcess(this);
        QStringList args;
        args << "x" << filename;
        process->start("7zr", args);
        process->waitForFinished();

        rescanPackage();
    }
}

void PackagingEditor::uninstallPackage(){
    QListWidgetItem *item = package_list->currentItem();
    if(item == NULL)
        return;

    QMessageBox::StandardButton button = QMessageBox::question(this,
                                                               tr("Are you sure"),
                                                               tr("Are you sure to remove ?"));
    if(button != QMessageBox::Ok)
        return;

    QFile file(QString("extensions/%1.spec").arg(item->text()));
    if(file.open(QIODevice::ReadOnly)){
        QTextStream stream(&file);
        while(!stream.atEnd()){
            QFile::remove(stream.readLine());
        }
    }

    file.remove();

    rescanPackage();
}

void PackagingEditor::browseFiles(){
    QStringList files = QFileDialog::getOpenFileNames(this,
                                                      tr("Select one or more files to package"),
                                                      ".",
                                                      tr("Any files (*.*)"));

    QDir dir;
    foreach(QString file, files){
        new QListWidgetItem(dir.relativeFilePath(file), file_list);
    }
}

void PackagingEditor::makePackage(){
    if(file_list->count() == 0)
        return;

    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Select a package name"),
                                                    ".",
                                                    tr("7z format (*.7z)"));

    if(!filename.isEmpty()){
        QFileInfo info(filename);
        QString spec_name = QString("extensions/%1.spec").arg(info.baseName());
        QFile file(spec_name);
        if(file.open(QIODevice::WriteOnly)){
            QTextStream stream(&file);

            int i;
            for(i=0; i<file_list->count(); i++){
                stream << file_list->item(i)->text() << "\n";
            }

            file.close();
        }

        QProcess *process = new QProcess(this);
        QStringList args;
        args << "a" << filename << spec_name << ("@" + spec_name);
        process->start("7zr", args);
        process->waitForFinished();

        rescanPackage();
    }
}

void MainWindow::on_actionPackaging_triggered()
{
    PackagingEditor *editor = new PackagingEditor(this);
    editor->show();
}
