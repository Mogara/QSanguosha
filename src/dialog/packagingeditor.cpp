#include "packagingeditor.h"
#include "mainwindow.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QTabWidget>
#include <QProcess>
#include <QMessageBox>
#include <QFormLayout>
#include <QSettings>

#include "settings.h"

typedef const QSettings *SettingsStar;

Q_DECLARE_METATYPE(SettingsStar);

MetaInfoWidget::MetaInfoWidget(bool load_config){
    setTitle(tr("Meta information"));

    QFormLayout *layout = new QFormLayout;

    name_edit = new QLineEdit;
    designer_edit = new QLineEdit;
    programmer_edit = new QLineEdit;
    version_edit = new QLineEdit;
    description_edit = new QTextEdit;

    name_edit->setObjectName("Name");
    designer_edit->setObjectName("Designer");
    programmer_edit->setObjectName("Programmer");
    version_edit->setObjectName("Version");
    description_edit->setObjectName("Description");

    if(load_config){
        Config.beginGroup("PackageManager");
        name_edit->setText(Config.value("Name", "My DIY").toString());
        designer_edit->setText(Config.value("Designer", tr("Designer")).toString());
        programmer_edit->setText(Config.value("Programmer", "Moligaloo").toString());
        version_edit->setText(Config.value("Version", "1.0").toString());
        description_edit->setText(Config.value("Description").toString());
        Config.endGroup();
    }

    layout->addRow(tr("Name"), name_edit);
    layout->addRow(tr("Designer"), designer_edit);
    layout->addRow(tr("Programmer"), programmer_edit);
    layout->addRow(tr("Version"), version_edit);
    layout->addRow(tr("Description"), description_edit);

    setLayout(layout);
}

void MetaInfoWidget::saveToSettings(QSettings &settings){
    QList<const QLineEdit *> edits = findChildren<const QLineEdit *>();

    foreach(const QLineEdit *edit, edits){
        settings.setValue(edit->objectName(), edit->text());
    }

    settings.setValue("Description", description_edit->toPlainText());
}

void MetaInfoWidget::setName(const QString &name){
    name_edit->setText(name);
}

void MetaInfoWidget::setDesigner(const QString &designer){
    designer_edit->setText(designer);
}

void MetaInfoWidget::setCoder(const QString &coder){
    programmer_edit->setText(coder);
}

void MetaInfoWidget::showSettings(const QSettings *settings){
    QList<QLineEdit *> edits = findChildren<QLineEdit *>();

    foreach(QLineEdit *edit, edits){
        edit->setText(settings->value(edit->objectName()).toString());
    }

    description_edit->setText(settings->value("Description").toString());
}

PackagingEditor::PackagingEditor(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("DIY package manager"));

    QString url = "http://www.7-zip.org";
    QLabel *label = new QLabel(tr("Package format is 7z, see its offcial site :<a href='%1' style = \"color:#0072c1; \">%1</a>").arg(url));

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
    foreach(QFileInfo info, dir.entryInfoList(QStringList() << "*.ini")){
        const QSettings *settings = new QSettings(info.filePath(), QSettings::IniFormat, package_list);

        QString name = settings->value("Name").toString();
        if(name.isEmpty())
            name = info.baseName();

        QListWidgetItem *item = new QListWidgetItem(icon, name, package_list);
        QVariant data = QVariant::fromValue(settings);
        item->setData(Qt::UserRole, data);
    }
}

QWidget *PackagingEditor::createManagerTab(){
    QWidget *widget = new QWidget;

    package_list = new QListWidget;
    package_list->setViewMode(QListView::IconMode);
    package_list->setIconSize(QSize(64, 64));
    package_list->setMovement(QListView::Static);
    package_list->setWordWrap(true);
    package_list->setResizeMode(QListView::Adjust);

    loadPackageList();

    QVBoxLayout *vlayout = new QVBoxLayout;

    QCommandLinkButton *install_button = new QCommandLinkButton(tr("Install"));
    install_button->setDescription(tr("Install a DIY package"));

    QCommandLinkButton *modify_button = new QCommandLinkButton(tr("Modify"));
    modify_button->setDescription(tr("Modify a DIY package"));

    QCommandLinkButton *uninstall_button = new QCommandLinkButton(tr("Uninstall"));
    uninstall_button->setDescription(tr("Uninstall a DIY package"));

    QCommandLinkButton *rescan_button = new QCommandLinkButton(tr("Rescan"));
    rescan_button->setDescription(tr("Rescan existing packages"));

    vlayout->addWidget(install_button);
    vlayout->addWidget(modify_button);
    vlayout->addWidget(uninstall_button);
    vlayout->addWidget(rescan_button);
    vlayout->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addLayout(vlayout);
    layout->addWidget(package_list);
    layout->addWidget(package_list_meta = new MetaInfoWidget(false));
    widget->setLayout(layout);

    connect(install_button, SIGNAL(clicked()), this, SLOT(installPackage()));
    connect(modify_button, SIGNAL(clicked()), this, SLOT(modifyPackage()));
    connect(uninstall_button, SIGNAL(clicked()), this, SLOT(uninstallPackage()));
    connect(rescan_button, SIGNAL(clicked()), this, SLOT(rescanPackage()));
    connect(package_list, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(updateMetaInfo(QListWidgetItem*)));
    connect(package_list, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(updateMetaInfo(QListWidgetItem*)));
    connect(package_list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(modifyPackage()));

    return widget;
}

void PackagingEditor::updateMetaInfo(QListWidgetItem *item){
    if(item == NULL)
        return;
    SettingsStar settings = item->data(Qt::UserRole).value<SettingsStar>();
    if(settings){
        package_list_meta->showSettings(settings);
    }
}

void PackagingEditor::rescanPackage(){
    QList<QSettings *> settings_list = findChildren<QSettings *>();
    foreach(QSettings *settings, settings_list)
        settings->deleteLater();

    package_list->clear();

    loadPackageList();
}

QWidget *PackagingEditor::createPackagingTab(){
    QWidget *widget = new QWidget;

    file_list = new QListWidget;

    QVBoxLayout *vlayout = new QVBoxLayout;

    QCommandLinkButton *browse_button = new QCommandLinkButton(tr("Add files"));
    browse_button->setDescription(tr("Select files to package"));

    QCommandLinkButton *remove_button = new QCommandLinkButton(tr("Remove files"));
    remove_button->setDescription(tr("Remove files to package"));

    QCommandLinkButton *package_button = new QCommandLinkButton(tr("Make package"));
    package_button->setDescription(tr("Export files to a single package"));

    vlayout->addWidget(browse_button);
    vlayout->addWidget(remove_button);
    vlayout->addWidget(package_button);
    vlayout->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addLayout(vlayout);
    layout->addWidget(file_list);
    layout->addWidget(file_list_meta = new MetaInfoWidget(true));

    widget->setLayout(layout);

    connect(browse_button, SIGNAL(clicked()), this, SLOT(browseFiles()));
    connect(remove_button, SIGNAL(clicked()), this, SLOT(removeFile()));
    connect(package_button, SIGNAL(clicked()), this, SLOT(makePackage()));
    connect(file_list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(removeFile(QListWidgetItem*)));

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

        connect(process, SIGNAL(finished(int)), this, SLOT(done7zProcess(int)));
        QMessageBox::information(this, tr("Notice"), tr("DIY package is loaded, please reset the game."));
    }
}

void PackagingEditor::modifyPackage(){
    QListWidgetItem *item = package_list->currentItem();
    if(!item)
        return;
    SettingsStar settings = item->data(Qt::UserRole).value<SettingsStar>();
    if(settings == NULL)
        return;

    tab_widget->setCurrentIndex(1);

    file_list->clear();
    QStringList filelist = settings->value("FileList").toStringList();
    foreach(QString file, filelist){
        if(!QFile::exists(file))
            QMessageBox::warning(this, tr("Warning"), tr("File %1 not found.").arg(file));
        else
            new QListWidgetItem(file, file_list);
    }

    file_list_meta->showSettings(settings);
}

void PackagingEditor::uninstallPackage(){
    QListWidgetItem *item = package_list->currentItem();
    if(item == NULL)
        return;

    SettingsStar settings = item->data(Qt::UserRole).value<SettingsStar>();
    if(settings == NULL)
        return;

    QMessageBox::StandardButton button = QMessageBox::question(this,
                                                               tr("Uninstall package"),
                                                               tr("Are you sure to remove %1 ?").arg(item->text()),
                                                               QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes);
    if(button != QMessageBox::Yes)
        return;

    QStringList filelist = settings->value("FileList").toStringList();
    foreach(QString file, filelist)
        QFile::remove(file);

    QFile::remove(settings->fileName());
    delete item;
    QMessageBox::information(this, tr("Notice"), tr("DIY package is deleted."));
}

void PackagingEditor::browseFiles(){
    QStringList files = QFileDialog::getOpenFileNames(this,
                                                      tr("Select one or more files to package"),
                                                      "extensions",
                                                      tr("Any files (*.*)"));

    QDir dir;
    foreach(QString file, files)
        new QListWidgetItem(dir.relativeFilePath(file), file_list);
}

void PackagingEditor::removeFile(QListWidgetItem* item, bool mute){
    if(mute == true){
        delete(item);
        return;
    }
    QString file_name = item->text();

    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Remove file"), tr("Are you sure to physical delete this file?"),
                                                              QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel), QMessageBox::Cancel);
    if(button == QMessageBox::Cancel)
        return;
    if(button == QMessageBox::Yes)
        QFile::remove(file_name);
    delete(item);
}

void PackagingEditor::removeFile(){
    QListWidgetItem *item = file_list->currentItem();
    if(item)
        removeFile(item, false);
    else{
        QMessageBox::StandardButton button = QMessageBox::question(this, tr("Clear file list"), tr("Are you sure to clear the file list?"),
                                                                   QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes);
        if(button == QMessageBox::Yes)
            file_list->clear();
    }
}

void PackagingEditor::makePackage(){
    if(file_list->count() == 0)
        return;

    QList<const QLineEdit *> edits = file_list_meta->findChildren<const QLineEdit *>();
    foreach(const QLineEdit *edit, edits){
        if(edit->text().isEmpty()){
            QMessageBox::warning(this, tr("Warning"), tr("Please fill the meta information before making package"));
            return;
        }
    }

    Config.beginGroup("PackageManager");
    file_list_meta->saveToSettings(Config);
    Config.endGroup();

    QString filepath = file_list->item(0)->text();
    QStringList split = filepath.split("/");
    if(split.count() == 2 && filepath.endsWith(".lua"))
        filepath = split.last().replace(QString(".lua"), QString(""));

    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Select a package name"),
                                                    filepath,
                                                    tr("7z format (*.7z)"));

    if(!filename.isEmpty()){
        QFileInfo info(filename);
        QString spec_name = QString("extensions/%1.ini").arg(info.baseName());
        QSettings settings(spec_name, QSettings::IniFormat);
        file_list_meta->saveToSettings(settings);
        QStringList filelist;
        for(int i=0; i<file_list->count(); i++)
            filelist << file_list->item(i)->text();
        settings.setValue("FileList", filelist);

        QProcess *process = new QProcess(this);
        QStringList args;
        args << "a" << filename << spec_name << filelist;
        process->start("7zr", args);

        connect(process, SIGNAL(finished(int)), this, SLOT(done7zProcess(int)));
        QMessageBox::information(this, tr("Notice"), tr("DIY package is finished."));

        tab_widget->setCurrentIndex(0);
    }
}

void PackagingEditor::done7zProcess(int exit_code){
    if(exit_code != 0)
        QMessageBox::warning(this, tr("Warning"), tr("Package compress/decompress error!"));
    else
        rescanPackage();
}

void MainWindow::on_actionPackaging_triggered()
{
    PackagingEditor *editor = new PackagingEditor(this);
    editor->show();
}
