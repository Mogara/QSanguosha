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
    QString filename = QFileDialog::getOpenFileName(this,
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

void ConfigDialog::saveConfig()
{
    int count_down = ui->nullificationSpinBox->value();
    Config.NullificationCountDown = count_down;
    Config.setValue("NullificationCountDown", count_down);
}

void ConfigDialog::on_browseBgMusicButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Select a background music"),
                                                    ".",
                                                    tr("Audio files (*.wav *.mp3)"));
    if(!filename.isEmpty()){
        ui->bgMusicPathLineEdit->setText(filename);
        Config.setValue("BackgroundMusic", filename);
    }
}

void ConfigDialog::on_resetBgMusicButton_clicked()
{
    ui->bgMusicPathLineEdit->clear();
    Config.remove("BackgroundMusic");
}
