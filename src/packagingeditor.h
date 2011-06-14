#ifndef PACKAGINGEDITOR_H
#define PACKAGINGEDITOR_H

#include <QDialog>
#include <QListWidget>
#include <QGroupBox>
#include <QSettings>

class MetaInfoWidget: public QGroupBox{
    Q_OBJECT

public:
    MetaInfoWidget(bool load_config);
    void saveToSettings(QSettings &settings);
    void showSettings(const QSettings *settings);
};

class PackagingEditor : public QDialog
{
    Q_OBJECT
public:
    explicit PackagingEditor(QWidget *parent = 0);

private:
    QListWidget *package_list;
    MetaInfoWidget *package_list_meta;

    QListWidget *file_list;
    MetaInfoWidget *file_list_meta;

    QWidget *createManagerTab();
    QWidget *createPackagingTab();
    void loadPackageList();

private slots:
    void installPackage();
    void uninstallPackage();
    void rescanPackage();
    void browseFiles();
    void makePackage();
    void done7zProcess(int exit_code);
    void updateMetaInfo(QListWidgetItem *item);
};

#endif // PACKAGINGEDITOR_H
