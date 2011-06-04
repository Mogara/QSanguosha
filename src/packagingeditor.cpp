#include "packagingeditor.h"
#include "mainwindow.h"

#include <QCommandLinkButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QTabWidget>

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

QWidget *PackagingEditor::createManagerTab(){
    QWidget *widget = new QWidget;

    package_list = new QListWidget;

    QVBoxLayout *vlayout = new QVBoxLayout;

    QCommandLinkButton *install_button = new QCommandLinkButton(tr("Install"));
    install_button->setDescription(tr("Install a DIY package"));

    QCommandLinkButton *uninstall_button = new QCommandLinkButton(tr("Uninstall"));
    uninstall_button->setDescription(tr("Uninstall a DIY package"));

    vlayout->addWidget(install_button);
    vlayout->addWidget(uninstall_button);
    vlayout->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addLayout(vlayout);
    layout->addWidget(package_list);

    widget->setLayout(layout);

    connect(install_button, SIGNAL(clicked()), this, SLOT(installPackage()));
    connect(uninstall_button, SIGNAL(clicked()), this, SLOT(uninstallPackage()));

    return widget;
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

}

void PackagingEditor::uninstallPackage(){

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

}

void MainWindow::on_actionPackaging_triggered()
{
    PackagingEditor *editor = new PackagingEditor(this);
    editor->show();
}
