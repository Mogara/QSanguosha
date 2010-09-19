#include "configdialog.h"
#include "ui_configdialog.h"
#include "settings.h"

#include <QFileDialog>

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    QString bg_path = Config.value("BackgroundBrush").toString();
    if(!bg_path.startsWith(":"))
        ui->bgPathLineEdit->setText(bg_path);
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::on_browseBgButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(NULL,
                                                    tr("Select a background image"),
                                                    ".",
                                                   tr("Images (*.png *.bmp *.jpg)"));

    if(!filename.isEmpty()){
        ui->bgPathLineEdit->setText(filename);
        Config.changeBackground(filename);

        emit bg_changed();
    }
}

void ConfigDialog::on_resetBgButton_clicked()
{
    ui->bgPathLineEdit->clear();
    Config.changeBackground(":/background.png");
    emit bg_changed();
}
